#include <Arduino.h>
#include <math.h>

/* =========================================================
   RP2350 高速IRボールセンサ（ご提示の距離構造ベース版）
   ========================================================= */

const uint8_t SENSOR_COUNT = 24;

const uint8_t pin[SENSOR_COUNT] = {
    29, 2, 3, 6,
    7, 8, 9, 10,
    11, 12, 13, 14,
    15, 16, 17, 18,
    19, 20, 21, 22,
    23, 26, 27, 28
};

#define NOISE_THRESHOLD        0
#define SENSOR_HIST_COUNT      12  // ご提示の 12履歴
#define ANGLE_EMA_ALPHA        0.35f
#define DIST_UPDATE_MS         5
#define LOST_RESET_COUNT       10

uint32_t pinMasks[SENSOR_COUNT];
uint32_t ballCount[SENSOR_COUNT] = {0};

uint32_t sensorHistory[SENSOR_COUNT][SENSOR_HIST_COUNT];
int historyIndex = 0;
bool historyFull = false;

uint32_t smoothedBallCount[SENSOR_COUNT] = {0};

float Ball_angle_deg = 0.0f;
uint8_t Ball_best_num = 0;
float Ball_distance = 0.0f;
uint32_t Ball_sum = 0;
int Ball_num[7];

float emaX = 1.0f;
float emaY = 0.0f;
bool emaInit = false;

float cosTable[SENSOR_COUNT];
float sinTable[SENSOR_COUNT];

byte sendBuf_byte[4]; 

uint32_t lastDistUpdate = 0;
uint8_t lostCount = 0;

// 関数宣言
void sampling100();
bool updateHistoryAndCheckReaction();
int findMaxSensorIndex();
uint32_t calculateRawDistance(int maxIndex);
void calculateAngle();
void resetDistanceSystem();
void debugAllSensors(); // 追加した全データデバッグ表示関数

void setup()
{
    Serial.begin(115200);
    // メイン通信用
    Serial2.setTX(4);
    Serial2.setRX(5);
    Serial2.begin(921600);

    for(int i = 0; i < SENSOR_COUNT; i++){
        float rad = (i * 15.0f) * DEG_TO_RAD;
        cosTable[i] = cosf(rad);
        sinTable[i] = sinf(rad);
    }

    for (int i = 0; i < SENSOR_COUNT; i++) {
        pinMasks[i] = 1ul << pin[i];
        pinMode(pin[i], INPUT_PULLUP); 
    }

    memset(sensorHistory, 0, sizeof(sensorHistory));
    pinMode(25, OUTPUT); 
    pinMode(24, OUTPUT); 
    digitalWrite(25, HIGH); 
    delay(1000); 
    digitalWrite(25, LOW); 
}

void loop()
{
    // 1. センサーのサンプリング
    sampling100();
    
    // 角度計算
    calculateAngle();

    // 2. 距離計算システム（5ms周期で同期）
    if(millis() - lastDistUpdate >= DIST_UPDATE_MS)
    {
        lastDistUpdate = millis();
        
        bool ballPresent = updateHistoryAndCheckReaction();
        uint32_t rawDistance = 0;

        if (ballPresent) {
            lostCount = 0;
            int maxIndex = findMaxSensorIndex();
            rawDistance = calculateRawDistance(maxIndex); 
        } else {
            lostCount++;
            if(lostCount >= LOST_RESET_COUNT){ 
                resetDistanceSystem(); 
            } 
        }

        // 3. 距離のフィルタリング処理
        float currentFar = ballPresent ? (float)rawDistance : 0.0f;

        Ball_distance = currentFar;
        Ball_sum = (uint32_t)Ball_distance;

        if(Ball_sum == 0 && currentFar > 0){
            Ball_sum = 1;
        }
    }

    /* =====================================================
       通信処理 (4バイト送信)
    ===================================================== */
    sendBuf_byte[0] = 0xAA; 

    if(Ball_sum == 0){
        sendBuf_byte[1] = 0x80; 
        sendBuf_byte[2] = 0x00;
        sendBuf_byte[3] = 0x00;
    } else {
        int sendAngle = (int)Ball_angle_deg; 
        if(sendAngle < 0) sendAngle = 0;
        if(sendAngle > 359) sendAngle = 359;

        sendBuf_byte[1] = (uint8_t)((sendAngle >> 8) & 0xFF); 
        sendBuf_byte[2] = (uint8_t)(sendAngle & 0xFF);

        if (Ball_distance > 255.0f) Ball_distance = 255.0f;
        sendBuf_byte[3] = (uint8_t)Ball_distance;   
    }

    Serial2.write(sendBuf_byte, 4);

    // 全センサーのデバッグデータ表示（100ms周期）
    debugAllSensors();
}

void sampling100() {
    memset(ballCount, 0, sizeof(ballCount));
    for (int j = 0; j < 500; j++) {
        uint32_t gpio_state = sio_hw->gpio_in;
        for (int i = 0; i < SENSOR_COUNT; i++) {
            if (!(gpio_state & pinMasks[i])) {
                ballCount[i]++; 
            }
        }
    }
}

bool updateHistoryAndCheckReaction() {
    for (int i = 0; i < SENSOR_COUNT; i++) {
        sensorHistory[i][historyIndex] = ballCount[i];
    }

    int loops = historyFull ? SENSOR_HIST_COUNT : (historyIndex + 1);

    historyIndex++;
    if (historyIndex >= SENSOR_HIST_COUNT) {
        historyIndex = 0;
        historyFull = true;
    }
    
    bool anyReaction = false;
    for (int i = 0; i < SENSOR_COUNT; i++) {
        uint32_t sum = 0;
        for (int h = 0; h < loops; h++) {
            sum += sensorHistory[i][h];
        }
        smoothedBallCount[i] = sum / loops;
        
        if (smoothedBallCount[i] > NOISE_THRESHOLD) { 
            anyReaction = true;
        }
    }
    return anyReaction;
}

int findMaxSensorIndex() {
    int maxIdx = 0;
    uint32_t maxVal = 0;
    for (int i = 0; i < SENSOR_COUNT; i++) {
        if (smoothedBallCount[i] > maxVal) {
            maxVal = smoothedBallCount[i];
            maxIdx = i;
        }
    }
    return maxIdx;
}

uint32_t calculateRawDistance(int maxIndex) {
    uint32_t sumTop12 = 0;
    for (int d = -5; d <= 6; d++) {
        int idx = maxIndex + d;
        if (idx < 0) idx += SENSOR_COUNT;
        else if (idx >= SENSOR_COUNT) idx -= SENSOR_COUNT;
        sumTop12 += smoothedBallCount[idx];
    }

    return sumTop12 /= 12;
}

void calculateAngle() {
    int maxIndex = findMaxSensorIndex();
    uint16_t maxVal = ballCount[maxIndex];
    if(maxVal == 0) return;

    Ball_num[0] = (maxIndex + 21) % SENSOR_COUNT;
    Ball_num[1] = (maxIndex + 22) % SENSOR_COUNT;
    Ball_num[2] = (maxIndex + 23) % SENSOR_COUNT;
    Ball_num[3] = maxIndex;                      
    Ball_num[4] = (maxIndex + 1) % SENSOR_COUNT; 
    Ball_num[5] = (maxIndex + 2) % SENSOR_COUNT; 
    Ball_num[6] = (maxIndex + 3) % SENSOR_COUNT; 

    float sumX = 0.0f; float sumY = 0.0f;
    
    for(int i = 0; i < 7; i++) {
        int idx = Ball_num[i];
        if(ballCount[idx] < maxVal * 0.25f) { continue; }
        float w = (float)(ballCount[idx] * ballCount[idx]);
        sumX += cosTable[idx] * w; sumY += sinTable[idx] * w;
    }
    if(sumX == 0.0f && sumY == 0.0f) { return; }
    if(!emaInit){ emaX = sumX; emaY = sumY; emaInit = true; }
    emaX = sumX * ANGLE_EMA_ALPHA + emaX * (1.0f - ANGLE_EMA_ALPHA);
    emaY = sumY * ANGLE_EMA_ALPHA + emaY * (1.0f - ANGLE_EMA_ALPHA);
    float rad = atan2f(emaY, emaX);
    if(rad < 0) rad += 2.0f * PI;
    Ball_angle_deg = rad * RAD_TO_DEG;
    Ball_best_num = ((int)((Ball_angle_deg + 7.5f) / 15.0f)) % SENSOR_COUNT;
}

void resetDistanceSystem() {
    Ball_distance = 0; 
    Ball_sum = 0; 
    historyIndex = 0; 
    historyFull = false;
    memset(sensorHistory, 0, sizeof(sensorHistory)); 
    memset(smoothedBallCount, 0, sizeof(smoothedBallCount));
}

/* =========================================================
   全24個のセンサー情報をまとめて出力するデバッグ関数
   ========================================================= */
void debugAllSensors() {
    static uint32_t lastDebug = 0;
    if (millis() - lastDebug < 100) return; // 100ms間隔で出力
    lastDebug = millis();

    int rawMaxIndex = 0;
    uint32_t rawMaxVal = 0;
    for (int i = 0; i < SENSOR_COUNT; i++) {
        if (ballCount[i] > rawMaxVal) {
            rawMaxVal = ballCount[i];
            rawMaxIndex = i;
        }
    }
    int smoothMaxIndex = findMaxSensorIndex();

    Serial.println("===============================================================");
    Serial.print("Angle: "); Serial.print(Ball_angle_deg, 1);
    Serial.print(" | BestNum: "); Serial.print(Ball_best_num);
    Serial.print(" | Dist: "); Serial.print(Ball_distance, 1);
    Serial.print(" | Sum: "); Serial.print(Ball_sum);
    Serial.print(" | RawMaxIdx: "); Serial.print(rawMaxIndex);
    Serial.print(" | SmoothMaxIdx: "); Serial.println(smoothMaxIndex);
    Serial.println("---------------------------------------------------------------");

    // 全24個のセンサー値を列挙して表示
    for (int i = 0; i < SENSOR_COUNT; i++) {
        Serial.print("[S");
        if (i < 10) Serial.print("0");
        Serial.print(i);
        Serial.print("]");

        // 最大値を持つセンサーにマークをつける
        if (i == smoothMaxIndex) Serial.print("*");
        else Serial.print(" ");

        Serial.print("R:");
        Serial.print(ballCount[i]);
        Serial.print("\tS:");
        Serial.print(smoothedBallCount[i]);

        // 改行処理（6個ごとに改行して4行で見やすく整理）
        if ((i + 1) % 6 == 0) {
            Serial.println();
        } else {
            Serial.print(" | ");
        }
    }
    Serial.println("===============================================================\n");
}