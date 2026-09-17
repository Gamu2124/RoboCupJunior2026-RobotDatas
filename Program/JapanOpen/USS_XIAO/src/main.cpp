#include <Arduino.h>

// ピン定義 (ESP32-S3)
const int TRIG_1 = 5;
const int ECHO_1 = 6;
const int TRIG_2 = 7;
const int ECHO_2 = 8;

// Event Groupのビット定義
#define BIT_SENSOR_1_DONE (1 << 0)
#define BIT_SENSOR_2_DONE (1 << 1)

// フィルタリング設定
const float MIN_DIST = 10.0;      
const float MAX_DIST = 100.0;    
const float DIFF_LIMIT = 20.0;   // 20cm以上の急変は一旦無視
const int   RETRY_MAX = 999;     
const int   CONSISTENT_COUNT = 30; // 急変しても、15回連続で同じ値なら新環境として採用
const int   WAIT_DURATION = 3;  
const int   MAX_CALCWIDTH = 7000;

// ウォームアップ設定
const int   WARMUP_COUNT = 50;   

EventGroupHandle_t xSensorEventGroup;
TaskHandle_t xTaskAHandle;

volatile float distance1 = 20.0;
volatile float distance2 = 20.0;
int errorCount1 = 0;
int errorCount2 = 0;

// 新環境への追従用カウンタ
int stableCount1 = 0;
int stableCount2 = 0;
float pendingDist1 = 0;
float pendingDist2 = 0;

int warmUp1 = 0;
int warmUp2 = 0;

// --- 強化版：差分フィルタ付き 距離測定関数 ---
float readDistanceWithDiffFilter(int trig, int echo, float lastValue, int &errorCount, int &warmUp, int &stableCount, float &pendingValue) {
  uint32_t waitStart = micros();
  while (digitalRead(echo) == HIGH) {
    if (micros() - waitStart > 2000) break;
  }
  
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long duration = pulseIn(echo, HIGH, MAX_CALCWIDTH);
  if (duration == 0) return lastValue; 

  float currentDist = duration * 0.034 / 2;

  // 1. 起動直後のウォームアップ
  if (warmUp < WARMUP_COUNT) {
    warmUp++;
    if (currentDist >= MIN_DIST && currentDist <= MAX_DIST) {
      return currentDist; 
    }
    return lastValue;
  }

  // 2. 範囲外チェック
  bool isInvalid = (currentDist < MIN_DIST || currentDist > MAX_DIST);
  float diff = abs(currentDist - lastValue);

  // 3. フィルタリングロジック
  if (isInvalid || diff > DIFF_LIMIT) {
    // 【追加】「急変した値そのもの」が安定しているかチェック
    // 1つ前の「保留値」と今回の値が近ければ、カウンタを増やす
    if (abs(currentDist - pendingValue) < 5.0) { 
      stableCount++;
    } else {
      stableCount = 0;
    }
    pendingValue = currentDist; // 今回の値を次の比較のためにキープ
    errorCount++;

    // 規定回数(15回)連続で安定した値が入ってきたら、それを「新しい壁」とみなす
    if (errorCount < RETRY_MAX && stableCount < CONSISTENT_COUNT) {
      return lastValue; // まだノイズ（敵ロボ）の可能性が高いので前回の値を維持
    } 
  }

  // 値が確定した場合のリセット
  errorCount = 0;
  stableCount = 0;
  return currentDist;
}

// --- Task A: センサー1担当 ---
void vTaskA(void *pvParameters) {
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(WAIT_DURATION)); 
    distance1 = readDistanceWithDiffFilter(TRIG_1, ECHO_1, distance1, errorCount1, warmUp1, stableCount1, pendingDist1);
    
    xEventGroupSetBits(xSensorEventGroup, BIT_SENSOR_1_DONE);
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  }
}

// --- Task B: センサー2担当 + UART管理 ---
void vTaskB(void *pvParameters) {
  for (;;) {
    distance2 = readDistanceWithDiffFilter(TRIG_2, ECHO_2, distance2, errorCount2, warmUp2, stableCount2, pendingDist2);
    
    xEventGroupSetBits(xSensorEventGroup, BIT_SENSOR_2_DONE);
    
    xEventGroupWaitBits(
        xSensorEventGroup,
        BIT_SENSOR_1_DONE | BIT_SENSOR_2_DONE,
        pdTRUE,
        pdTRUE,
        portMAX_DELAY);

    uint8_t sendData[4];
    sendData[0] = 255;                                   
    sendData[1] = (uint8_t)constrain(distance1, 0, 254); 
    sendData[2] = (uint8_t)constrain(distance2, 0, 254); 
    sendData[3] = 0;                                     

    Serial0.write(sendData, 4); 
    // デバッグ用
    Serial.printf("S1: %.1f, S2: %.1f\n", distance1, distance2);

    vTaskDelay(pdMS_TO_TICKS(WAIT_DURATION));
    xTaskNotifyGive(xTaskAHandle); 
  }
}

void setup() {
  Serial.begin(115200);
  Serial0.begin(115200);
  delay(1000);

  pinMode(TRIG_1, OUTPUT);
  pinMode(ECHO_1, INPUT);
  pinMode(TRIG_2, OUTPUT);
  pinMode(ECHO_2, INPUT);

  xSensorEventGroup = xEventGroupCreate();

  xTaskCreatePinnedToCore(vTaskA, "S1_Task", 4096, NULL, 1, &xTaskAHandle, 0);
  xTaskCreatePinnedToCore(vTaskB, "S2_Task", 4096, NULL, 1, NULL, 1);
}

void loop() {
  vTaskDelete(NULL);
}