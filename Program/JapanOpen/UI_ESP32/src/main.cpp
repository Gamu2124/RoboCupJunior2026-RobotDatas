#include <Arduino.h>

#define FASTLED_NO_STD
#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <WiFi.h>
#include <esp_now.h>

// ===== ESP-NOW設定 =====
uint8_t dcxAddress[] = {0xDC, 0xB4, 0xD9, 0x3B, 0x41, 0x24}; // 新しいDCX機のMACアドレスに修正
uint8_t reAddress[] = {0x1C, 0x69, 0x20, 0xAA, 0x14, 0x80};  // RE機のMACアドレス

typedef struct comm_message {
  bool isAttack;
  uint32_t timestamp;
  bool isReply;
  int stateA;         // 自分の現在のAの値（画面ステート）
} comm_message;

comm_message myData;
comm_message recvData;
esp_now_peer_info_t peerInfo;
volatile bool needToSendPong = false;
comm_message pongDataQueue;
uint8_t pongTargetMac[6];
unsigned long lastSendTime = 0;
int sendCount = 0;
bool partner_isAttack = false;
uint32_t lastRTT = 0;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Serial.print("Last Packet Send Status: ");
  // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&recvData, incomingData, sizeof(recvData));
  
  if (recvData.isReply) {
    // Pongを受信：RTTを計算
    lastRTT = micros() - recvData.timestamp;
  } else {
    // Pingを受信：Pong送信用にデータを保持（loop内で送信）
    memcpy(&pongDataQueue, &recvData, sizeof(recvData));
    pongDataQueue.isReply = true;
    memcpy(pongTargetMac, mac, 6);
    needToSendPong = true;
    
    partner_isAttack = recvData.isAttack;
  }
  
  /*
  Serial.print("--- Data Received --- from MAC: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", mac[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
  Serial.print("Partner is Attack: ");
  Serial.println(partner_isAttack ? "Yes" : "No");
  if (recvData.isReply) {
    Serial.printf("RTT: %lu us (%.2f ms)\n", lastRTT, lastRTT / 1000.0);
  }
  Serial.println("---------------------");
  */
}

// タイマ系は最後
#include <TIMER.h>
#include "simplify.h"

#include <Preferences.h>
Preferences preferences; //eepromみたいなやつ


//Neopixels
#define PIN        19
#define NUMPIXELS  16
#define MAX_NUM    10   // 最大許容数 X

// ===== FastLED設定 =====
#define LED_TYPE   WS2812B
#define COLOR_ORDER GRB
#define BRIGHTNESS 45

CRGB leds[NUMPIXELS];

// ===== 変数 =====
int targets[MAX_NUM];
int target_count = 0;   // 今何個入っているか
int target;             // 点灯させたいLED番号（0〜15）

void rainbow(int);
int set_rightPIXNUM(int);
void lightOne(int,uint8_t,uint8_t,uint8_t);
void lightMultiple(const int *nums, int count, uint8_t r, uint8_t g, uint8_t b);
bool addTarget(int);
void clearNeoPixel();
void lightAll(uint8_t, uint8_t, uint8_t);

//displays
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

SIMPLIFY simplify;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int Ball_angle = 45;
int Ball_far = 35;

float Line_sin = 0;
float Line_cos = 0;
int Line_level = 830;
// line_test.vec(Line_vals,Line_vec); //sincos

float catch_level = 30;
float catch_same = 3;

const int tact_Switch_1 = 25;
const int tact_Switch_2 = 26;
const int tact_Switch_3 = 27;
const int toggle_Switch = 23;
int Last_toggle_flag = 0;

int selected_color_flag = 0;
int blink_flag = 0;
int start_flag = 0;

double Now_DIR = 0;

int A = 0;
int B = 0;
int Last_A = 0;

int change_role = 0;
int change_commRole = 0;
int menu_transis = 0;
const int menu_MAX = 11; //0~~11
int prev_tact;
int now_tact;

int sent_flag = 0;

int role_changed = 0;
bool last_roleStatus;
bool last_camStatus; 
bool last_commRole; // false: DCX, true: RE
int first_change = 0;

TIMER timer_Right;
TIMER timer_Left;
TIMER timer_Center;
TIMER timer_Blinking;
TIMER timer_communication;

int change_coordinate(int angle){
  angle -= 90;
  if (angle < 0){
    angle += 360;
  }
  return angle;
}

struct TactResult{
  int Left;
  int Right;
  int Center;
};

TactResult TactSwitch_Check();
void displayModeText(const char* msg, int textSize);
void displayMenuText(const char* mainText, int mainSize, const char* leftText, const char* rightText);
void displayBallCheck(int Ball_angle, int Ball_far);
void displayLineCheck(float Line_angle, float Line_far);
void displayLineSet(int Line_level);
void displayGetCheck(float catch_level, float catch_same);
void displayGetSet(float catch_level);
void displayKickCheck();
void displaySelectColor(int selected_color_flag);
void displayCommRoleSet(int change_commRole);
void displayPartnerCheck(bool partner_isAttack, uint32_t rtt);
void displaySetDir(int Now_DIR);
void displayBlack();

void setup(){
  Serial.begin(115200);
  Serial2.begin(115200);

  // preferencesから設定を読み込む
  preferences.begin("config", false);
  last_roleStatus = preferences.getBool("get_role", false);
  last_camStatus = preferences.getBool("get_camrole", false);
  last_commRole = preferences.getBool("comm_role", false);
  preferences.end();

  // ESP-NOW 初期化
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
  } else {
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);

    // ピア登録 (自分がREなら相手はDCX, 自分がDCXなら相手はRE)
    if (last_commRole) {
      memcpy(peerInfo.peer_addr, dcxAddress, 6);
    } else {
      memcpy(peerInfo.peer_addr, reAddress, 6);
    }
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
      Serial.println("Failed to add peer");
    }
  }
  Wire.begin(21, 22); // SDA=21, SCL=22
  Wire.setClock(800000); // 800kHzに設定。安定しない場合は400000
  while (!Serial2) {
  // wait for serial port to connect. Needed for native USB
  }
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306の初期化に失敗しました"));
    for (;;);
  }

  display.clearDisplay();
  display.display();

  pinMode(tact_Switch_1, INPUT_PULLUP);
  pinMode(tact_Switch_2, INPUT_PULLUP);
  pinMode(tact_Switch_3, INPUT_PULLUP);
  // pinMode(toggle_Switch, INPUT);
  FastLED.addLeds<LED_TYPE, PIN, COLOR_ORDER>(leds, NUMPIXELS);
  FastLED.setBrightness(BRIGHTNESS);

  // ★ OLEDと共存させるため重要
  #define FASTLED_ALLOW_INTERRUPTS 1

  // ★ 電源安定（必要に応じて）
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);

  FastLED.clear();
  FastLED.show();

  clearNeoPixel();
}

void loop(){
  if (needToSendPong) {
    needToSendPong = false;
    esp_now_send(pongTargetMac, (uint8_t *) &pongDataQueue, sizeof(pongDataQueue));
  }

  // ESP-NOW ロール送信 (200msに1回)
  if(timer_communication.read_ms() > 200){
    timer_communication.reset();
    myData.isAttack = (last_roleStatus == 0); // 0:Attack, 1:Defence
    myData.timestamp = micros();
    myData.isReply = false;
    myData.stateA = A; // 自分のAの状態を載せる

    uint8_t* targetAddr = last_commRole ? dcxAddress : reAddress;
    
    esp_err_t result = esp_now_send(targetAddr, (uint8_t *) &myData, sizeof(myData));
    if(result != ESP_OK){
      Serial.println("Error sending the data");
    }
  }

  // unsigned long startTime = micros(); // 開始（マイクロ秒）
  sent_flag = 0;
  int data;
  int data_length = 4;
  // if(menu_transis == 2 || menu_transis == 4 || menu_transis == 5) data_length = 4;

  int recieved_data[data_length] = {0};    // ← 最大サイズで固定！安全！
  if(Serial2.available() >= data_length){
    if(Serial2.read() == 255){
      recieved_data[0] = 255; 
      for(int i = 1; i < data_length; i++){
        recieved_data[i] = Serial2.read();
      }
      // Serial.print("read");
      // for(int i = 0; i < data_length; i++){
      //   Serial.print(" ");
      //   Serial.print(recieved_data[i]);
      // }
      // Serial.println();
    }
    else{
      Serial2.read();
    }
  }

  sent_flag = 0;
  TactResult tact = TactSwitch_Check();
  display.clearDisplay();

  // --- 画面ごとの描画 ---
  if(A == 0){
    if(B == A){
      if(first_change == 0){
        change_role = int(last_roleStatus);
        first_change = 1;
      }

      if(tact.Left == 1){
        change_role -= 1;
      } 
      if(tact.Right == 1){
        change_role += 1;
      } 
      if(tact.Center == 1){
        A = 10;
        role_changed = 1;

        if(change_role != int(last_roleStatus)){
          preferences.begin("config", false);
          preferences.putBool("get_role", bool(change_role)); // ★新しく決まった方を保存！

          preferences.end();
          last_roleStatus = bool(change_role);
        }
        first_change = 0;
      } 

      if(change_role > 1) change_role = 0;
      if(change_role < 0) change_role = 1;
      
      switch(change_role){
        case 0: displayModeText("Attack", 3); break;
        case 1: displayModeText("Defence", 3); break;
      }
    }
  }

  if(A == 10){
    clearNeoPixel();
    if(B == A){
      int tact_pressed = tact.Left || tact.Right || tact.Center;

      if(tact.Left == 1){
        menu_transis -= 1;
        first_change = 0;
      } 
      if(tact.Right == 1){
        menu_transis += 1;
        first_change = 0;
      } 
      if(tact.Center == 1){
        if(menu_transis == 0){
          A = 30;
        }
        else{
          A = 20; 
        }          
      }

      if(menu_transis > menu_MAX) menu_transis = 0;
      if(menu_transis < 0) menu_transis = menu_MAX;

      switch(menu_transis){
        case 0: displayMenuText("START", 4, "Last_RESET", "Set_Ball"); break;
        case 1: displayMenuText("Set_Ball", 2, "START", "Ball_Check"); break;
        case 2: displayMenuText("Ball_Check", 2, "Set_Ball", "Set_Line"); break;
        case 3: displayMenuText("Set_Line", 2, "Ball_Check", "Line_Check"); break;
        case 4: displayMenuText("Line_Check", 2, "Set_Line", "Get_Check"); break;
        case 5: displayMenuText("Get_Check", 2, "Line_Check", "Get_Set"); break;
        case 6: displayMenuText("Get_Set", 2, "Get_Check", "Kick_Check"); break;
        case 7: displayMenuText("Kick_Check", 2, "Get_Set", "Set Rainbow"); break;
        case 8: displayMenuText("Rainbow", 2, "Kick_Check", "Device_ID"); break;
        case 9: displayMenuText("Device_ID", 2, "Rainbow", "P_Check"); break;
        case 10: displayMenuText("P_Check", 2, "Device_ID", "Last_RESET"); break;
        case 11: displayMenuText("Last_RESET", 2, "P_Check", "START"); break;
      }
    }
  }

  int kick_now = 0;
  if(A == 20){
    if(B == A){
      if(tact.Center == 1){
        if(menu_transis == 0){
          A = 30;
        }
        else{
          A = 10;
          menu_transis = 0;
        }
      }
      int ball_num = 0;
      if(menu_transis != 8) clearNeoPixel();
      switch(menu_transis){
        case 1: displayBlack(); break;
        case 2: 
          displayBallCheck(simplify.FixLimit(recieved_data[1]*2), recieved_data[2]); 
          // ball_num = recieved_data[1]*2 / 22.5;
          // lightOne(set_rightPIXNUM(ball_num), 255, 0, 0);
          break;
        case 3:
          if(tact.Left == 1) Line_level--;
          if(tact.Right == 1) Line_level++;
          displayLineSet(Line_level); break;
        case 4: 
          if(recieved_data[1] != 255) recieved_data[1] *= 2; 
          if(recieved_data[2] != 255);
          displayLineCheck(recieved_data[1], recieved_data[2]); break;
        case 5: 
          displayGetCheck(recieved_data[1], recieved_data[2]*2); break;
        case 6: 
          if(tact.Left == 1) catch_level--;
          if(tact.Right == 1) catch_level++;
          displayGetSet(catch_level); break;
          break;
        case 7: 
          if(digitalRead(toggle_Switch) == 1) kick_now = 1; sent_flag = 1;
          displayKickCheck(); break;
        case 8: rainbow(10); break;
        case 9:
          if(first_change == 0){
            change_commRole = int(last_commRole);
            first_change = 1;
          }
          if(tact.Left == 1) change_commRole = 0; 
          if(tact.Right == 1) change_commRole = 1; 
          displayCommRoleSet(change_commRole);
          if(change_commRole != int(last_commRole)){
            preferences.begin("config", false);
            preferences.putBool("comm_role", bool(change_commRole));
            preferences.end();
            last_commRole = bool(change_commRole);
          }
          break;
        case 10:
          displayPartnerCheck(partner_isAttack, lastRTT);
          break;
        case 11:
          displayBlack(); break;
      }

      if(menu_transis == 2 || menu_transis == 4 || menu_transis == 5){ //ボールライン
        sent_flag = 1;
      }
    }
  }

  int select_now = 0;
  if(A == 30){
    if(B == A){
      if(first_change == 0){
        selected_color_flag = int(last_camStatus)+1;
        first_change = 1;
      }

      select_now = 1;
      if(tact.Left == 1) selected_color_flag--;
      if(tact.Right == 1) selected_color_flag++;
      if(tact.Center == 1){
        A = 40;

        if(selected_color_flag != int(last_camStatus)+1){
          preferences.begin("config", false);
          preferences.putBool("get_camrole", bool(selected_color_flag-1)); // ★新しく決まった方を保存！

          preferences.end();
          last_camStatus = bool(selected_color_flag-1);
          first_change = 0;
        }
      }
      
      if(selected_color_flag > 2) selected_color_flag = 2;
      if(selected_color_flag < 1) selected_color_flag = 1;
      sent_flag = 1;
      displaySelectColor(selected_color_flag-1);
    }
  }

  int get_now = 0;
  int isStart = 0;
  if(A == 40){ //トグルに一旦する
    if(B == A){
      sent_flag = 1;
      if(recieved_data[1] != 255 && recieved_data[2] == 128 && isStart != 1) Now_DIR = recieved_data[1] * 2; 

      if(digitalRead(toggle_Switch) == 1){
        isStart = 1;
        start_flag = 150;
        displayBlack();
        // lightAll(255, 0, 0); // 白で全点灯
        // if(recieved_data[1] != 255 && recieved_data[2] == 128){ //ライン
        //   if(recieved_data[1] == 1) lightAll(255,0,0);
        //   else lightAll(0,0,255);
        // } 
        // if(recieved_data[1] != 255 && recieved_data[2] == 128){ //kickstep
        //   if(recieved_data[1] == 0) lightAll(0,0,255); //持ってないblue
        //   else if(recieved_data[1] == 1) lightAll(0,255,0); //片方持ってる紫
        //   else if(recieved_data[1] == 2) lightAll(255,150,0);
        //   else if(recieved_data[1] == 3) lightAll(255,60,0); //持ってるred
        //   else if(recieved_data[1] == 4) lightAll(255,0,0); //両方とも持ってる白
        //   else if(recieved_data[1] == 10) lightAll(128,0,128); //ライン両方ともある水色
        //   else if(recieved_data[1] == 20) lightAll(255,255,255); //ライン片方ある黄色 //ライン片方ある黄色
        // }
      }
      else{
        clearNeoPixel();
        if(Last_toggle_flag == 150){
          isStart = 1;
          start_flag = 100;
        }
        else{
          displaySetDir(int(Now_DIR));
          get_now = 1;
          if(tact.Center == 1) get_now = 2;
        }
      }
    }
  }

  if(role_changed){
    byte array_toSent[3] = {255, 0, 0};
    array_toSent[1] = change_role + 3; // 3:Attack 4:Defence
    Serial2.write(array_toSent, 3);
    role_changed = 0;
  }
  if(sent_flag == 1){
    //1,2:色　　3,4:モードチェンジ 5:ボール 10:ライン 15:角度 20:補足 25:角度リセット 30:キックチェック
    byte array_toSent[3] = {255, 0, 0}; //送るフラグ
    if(A == 10 || A == 20){ //画面遷移
      switch(menu_transis){
        case 2: array_toSent[1] = 5; Serial.println("ball"); break; //ボール
        case 4: array_toSent[1] = 10; Serial.println("line"); break; //ライン
        case 5: array_toSent[1] = 20; Serial.println("catch"); break; //補足
        case 7: if(kick_now == 1) array_toSent[1] = 30; //キックチェック
      }
    }
    else{
      switch(A){ //画面遷移以外で送信するやつ、ステートで遷移
        case 0: array_toSent[1] = change_role + 3; break; //モードチェンジ
        case 30: if(selected_color_flag != 0 && select_now == 1) array_toSent[1] = selected_color_flag; break; //色選択
        case 40:
          if(isStart == 1){
            array_toSent[1] = start_flag; //起動
          }
          else{
            switch(get_now){
              case 1: array_toSent[1] = 15; break;//角度取得
              case 2: array_toSent[1] = 25; break;//リセット
            }
          }
        break;
      }
    }
    // for(int i = 0 ; i < 3; i++){
    //   Serial.print(array_toSent[i]);
    //   Serial.print("  ");
    // }
    // Serial.println();
    Serial2.write(array_toSent, 3);
  }

  display.display();
  B = A;

  Last_toggle_flag = start_flag;
  // unsigned long endTime = micros();   // 終了
  // unsigned long processTimeUs = endTime - startTime; // 差分(μs)

  // if(recieved_data[1] != 255 && recieved_data[2] == 128 && isStart == 1){
  //   Serial.print(recieved_data[1]);
  // }
  // Serial.print("   Process Time: ");
  // Serial.print(processTimeUs);
  // Serial.println(" us");
}


TactResult TactSwitch_Check(){
  bool tact1 = digitalRead(tact_Switch_1);
  bool tact2 = digitalRead(tact_Switch_2);
  bool tact3 = digitalRead(tact_Switch_3);

  int Left= 0;
  int Right = 0;
  int Center = 0;
  static int Left_A = 0;
  static int Right_A = 0;
  static int Center_A = 0;
  
  if(Left_A == 0){
    if(tact1 == LOW){
      Left_A = 1;
      timer_Left.reset();
    }
  }
  else if(Left_A == 1){
    if(100 < timer_Left.read_ms()){
      if(tact1 == HIGH){
        Left_A = 2;
      }
    }
    if(800 < timer_Left.read_ms()){
      if(tact1 == LOW){
        Left_A = 5;
      }
    }
  }
  else if(Left_A == 2){
    Left = 1;
    Left_A = 0;
  }
  else if(Left_A == 5){
    if(50 < timer_Left.read_ms()){
      if(tact1 == LOW){
        Left = 1;
        timer_Left.reset();
      }
      else{
        Left_A = 0;
      }
    }
  }

  if(Right_A == 0){
    if(tact3 == LOW){
      Right_A = 1;
      timer_Right.reset();
    }
  }
  else if(Right_A == 1){
    if(100 < timer_Right.read_ms()){
      if(tact3 == HIGH){
        Right_A = 2;
      }
    }
    if(800 < timer_Right.read_ms()){
      if(tact3 == LOW){
        Right_A = 5;
      }
    }
  }
  else if(Right_A == 2){
    Right = 1;
    Right_A = 0;
  }
  else if(Right_A == 5){
    if(50 < timer_Right.read_ms()){
      if(tact3 == LOW){
        Right = 1;
        timer_Right.reset();
      }
      else{
        Right_A = 0;
      }
    }
  }

  if(Center_A == 0){
    if(tact2 == LOW){
      Center_A = 1;
      timer_Center.reset();
    }
  }
  else if(Center_A == 1){
    if(100 < timer_Center.read_ms()){
      if(tact2 == HIGH){
        Center_A = 2;
      }
    }
    if(800 < timer_Center.read_ms()){
      if(tact2 == LOW){
        Center_A = 5;
      }
    }
  }
  else if(Center_A == 2){
    Center = 1;
    Center_A = 0;
  }
  else if(Center_A == 5){
    if(50 < timer_Center.read_ms()){
      if(tact2 == LOW){
        Center = 1;
        timer_Center.reset();
      }
      else{
        Center_A = 0;
      }
    }
  }

  if(Right == 1 && Left == 1){
    Center = 1;
  }

  // Serial.print(" Left : ");
  // Serial.print(Left);
  // Serial.print(" Center : ");
  // Serial.print(Center);
  // Serial.print(" Right : ");
  // Serial.println(Right);

  TactResult result = { Left, Right, Center };
  return result;
}

void displayModeText(const char* msg_, int textSize_){
  float textWidth;
  float textHeight;
  display.setTextSize(textSize_);
  display.setTextColor(SSD1306_WHITE);

  // 幅と高さを計算（floatで正確に）
  textWidth  = 6.0 * textSize_ * strlen(msg_); // 文字幅 6px × textSize × 文字数
  textHeight = 8.0 * textSize_;               // 文字高さ 8px × textSize

  // 中央座標を計算
  int16_t cursorX = (SCREEN_WIDTH - textWidth) / 2;
  int16_t cursorY = (SCREEN_HEIGHT - textHeight) / 2;

  // 描画
  display.setCursor(cursorX, cursorY);
  display.print(msg_);
}

void displayMenuText(const char* mainText_, int mainSize_, const char* leftText_, const char* rightText_){
  display.clearDisplay();

  // --- メインテキスト ---
  display.setTextSize(mainSize_);
  display.setTextColor(SSD1306_WHITE);

  int16_t textWidth  = 6 * mainSize_ * strlen(mainText_);
  int16_t textHeight = 8 * mainSize_;

  int16_t cursorX = (SCREEN_WIDTH - textWidth) / 2;
  int16_t cursorY = (SCREEN_HEIGHT - textHeight) / 2;

  display.setCursor(cursorX, cursorY);
  display.print(mainText_);

  // --- 左下の項目 ---
  display.setTextSize(1);
  // int16_t leftWidth = 6 * 1 * strlen(leftText);
  display.setCursor(0, SCREEN_HEIGHT - 8);
  display.print(leftText_);

  // --- 右下の項目 ---
  display.setTextSize(1);
  int16_t rightWidth = 6 * 1 * strlen(rightText_);
  display.setCursor(SCREEN_WIDTH - rightWidth, SCREEN_HEIGHT - 8);
  display.print(rightText_);

  display.display();
}

void displayBallCheck(int Ball_angle_, int Ball_far_){
  int x = 32;
  int y = 32;
  int r_1 = 30;
  int r_2 = 20;
  int r_3 = 10;
  int far_max = 50;
  float fixed_angle = 0;
  float vx = 0;
  float vy = 0;
  int right_Ball_far = 0;

  right_Ball_far = 50 - Ball_far_;

  // 距離を円の半径に変換
  float this_far = (right_Ball_far / (float)far_max) * 30.0;

  // 画面クリア
  display.clearDisplay();

  // 同心円と十字線を描画
  display.drawCircle(x, y, r_1, SSD1306_WHITE);
  display.drawCircle(x, y, r_2, SSD1306_WHITE);
  display.drawCircle(x, y, r_3, SSD1306_WHITE);
  display.drawLine(x - r_1, y, x + r_1, y, SSD1306_WHITE);
  display.drawLine(x, y - r_1, x, y + r_1, SSD1306_WHITE);

  // 角度を座標系に変換（change_coordinate関数を使用）
  fixed_angle = change_coordinate(Ball_angle_);
  vx = cos(radians(fixed_angle)) * this_far;
  vy = sin(radians(fixed_angle)) * this_far;

  // ボール位置を描画
  display.fillCircle(vx + x, vy + y, 4, SSD1306_WHITE);

  // テキスト表示
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(70, 10);
  display.print("Ball");

  display.setTextSize(1);
  display.setCursor(70, 30);
  display.print("Dir:");
  display.setCursor(100, 30);
  display.print(Ball_angle_, 1);

  display.setCursor(70, 40);
  display.print("Far:");
  display.setCursor(100, 40);
  display.print(Ball_far_, 1);

  // 描画を反映
  display.display();
}

void displayLineCheck(float Line_angle_, float Line_depth_){
  // --- 前提 ---
  int x = 32;           // 円の中心X
  int y = 32;           // 円の中心Y
  int r_1 = 30;         // 外円半径
  float r_3 = 17.5;     // 内円半径
  float len = 10;       // 法線の長さ
  float fixed_Line_vec_x = 0;
  float fixed_Line_vec_y = 0;
  float coordinate_x = 0; 
  float coordinate_y = 0;
  float m = 0;
  float m_normal = 0;
  float dx = 0; 
  float dy = 0;
  float Line_angle = 0;
  int Line_far = 0;

  
  // ロボカッパーベクトル
  // Line_angle_ -= 180;
  // Line_angle_ = simplify.FixLimit(Line_angle_);
  if(Line_depth_ == 255){
    displayModeText("No LINE", 3);
    return;
  }
  fixed_Line_vec_x = cos(radians(change_coordinate(Line_angle_))) * Line_depth_;  // x成分 (cos)
  fixed_Line_vec_y = -sin(radians(change_coordinate(Line_angle_))) * Line_depth_;  // y成分 (sin)

  // 実験
  // fixed_Line_vec_x = 0.48865; // x成分 (sin)
  // fixed_Line_vec_y = -0.48865;  // y成分 (cos)


  // --- 画面クリア ---
  display.clearDisplay();

  // --- 円の描画 ---
  display.drawCircle(x, y, r_1, SSD1306_WHITE);
  display.drawCircle(x, y, r_3, SSD1306_WHITE);

  // --- ロボカッパー座標系 → ディスプレイ座標系変換 ---
  coordinate_x = x + fixed_Line_vec_x * r_3;  // x方向そのまま
  coordinate_y = y - fixed_Line_vec_y * r_3;  // y方向反転

  // --- 元線の傾き ---
  bool vertical = false;

  if(coordinate_x != x){
    m = (coordinate_y - y) / (coordinate_x - x);
  } 
  else{
    vertical = true;
  }

  // --- 法線の傾き ---
  if(!vertical){
    m_normal = -1.0 / m;
  }

  // --- 法線の長さを計算 ---
  if(vertical){
    dx = len;
    dy = 0;
  } 
  else if(coordinate_x == x && coordinate_y == y){
    dx = len;
    dy = 0;
  } 
  else{
    dx = len / sqrt(1 + m_normal * m_normal);
    dy = m_normal * dx;
  }

  // --- 法線の始点・終点（元線先端を中心に） ---
  float x_start = coordinate_x - dx;
  float y_start = coordinate_y - dy;
  float x_end   = coordinate_x + dx;
  float y_end   = coordinate_y + dy;

  // --- 法線を描画 ---
  // display.drawLine(x_start, y_start, x_end, y_end, SSD1306_WHITE);

  // // --- 直線（元線）を描画 ---
  display.drawLine(x, y, coordinate_x, coordinate_y, SSD1306_WHITE);

  // --- テキスト表示 ---
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(70, 10);
  display.print("Line");

  display.setTextSize(1);
  display.setCursor(70, 30);
  display.print("Dir:");
  display.setCursor(100, 30);
  display.print(Line_angle_, 1);

  display.setCursor(70, 40);
  display.print("Far:");
  display.setCursor(100, 40);
  display.print(Line_depth_, 1);

  // --- 描画反映 ---
  display.display();

}

void displayLineSet(int Line_level_){
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // --- タイトル文字 ---
  const char* msg = "Line_level";
  float textSize = 1.5;
  int16_t charWidth = 6;
  int16_t charHeight = 8;

  int16_t textWidth = charWidth * textSize * strlen(msg);
  int16_t textHeight = charHeight * textSize;

  int16_t cursorX = (SCREEN_WIDTH - textWidth) / 2;
  int16_t cursorY = ((SCREEN_HEIGHT - textHeight) / 2) - 20;

  display.setTextSize(textSize);
  display.setCursor(cursorX, cursorY);
  display.print(msg);

  // --- 整数値を中央下に表示 ---
  display.setTextSize(3);
  String value = String((int)Line_level_);  // 整数に変換
  int16_t valueWidth = 6 * 3 * value.length();
  int16_t valueX = (SCREEN_WIDTH - valueWidth) / 2;
  int16_t valueY = 30;

  display.setCursor(valueX, valueY);
  display.print(value);

  display.display();
}

void displayGetCheck(float catch_level_, float catch_same_){
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // --- Get 表示 ---
  display.setCursor(0, 0);
  display.print("Get: ");
  if (catch_same_ < catch_level_){
    display.print("O");  // ◯
  } 
  else{
    display.print("X");  // ×
  }

  // --- catch_same 表示 ---
  display.setCursor(0, 16);
  display.print("catch_same: ");
  display.print(catch_same_);

  display.display();
}

void displayGetSet(float catch_level_){
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // --- タイトル文字 ---
  const char* msg = "catch_level";
  float textSize = 1.5;
  int16_t charWidth = 6;
  int16_t charHeight = 8;

  int16_t textWidth = charWidth * textSize * strlen(msg);
  int16_t textHeight = charHeight * textSize;

  int16_t cursorX = (SCREEN_WIDTH - textWidth) / 2;
  int16_t cursorY = ((SCREEN_HEIGHT - textHeight) / 2) - 20;

  display.setTextSize(textSize);
  display.setCursor(cursorX, cursorY);
  display.print(msg);

  // --- 整数値を中央下に表示 ---
  display.setTextSize(3);
  String value = String((int)catch_level_);  // 整数に変換
  int16_t valueWidth = 6 * 3 * value.length();
  int16_t valueX = (SCREEN_WIDTH - valueWidth) / 2;
  int16_t valueY = 30;

  display.setCursor(valueX, valueY);
  display.print(value);

  display.display();
}

void displayKickCheck(){
  display.setTextSize(4);
  display.setTextColor(SSD1306_WHITE);

  // 幅と高さを計算（floatで正確に）
  float textWidth  = 6.0 * 4 * strlen("Kick"); // 文字幅 6px × textSize × 文字数
  float textHeight = 8.0 * 4;               // 文字高さ 8px × textSize

  // 中央座標を計算
  int16_t cursorX = (SCREEN_WIDTH - textWidth) / 2;
  int16_t cursorY = (SCREEN_HEIGHT - textHeight) / 2;

  // 描画
  display.setCursor(cursorX, cursorY);
  display.print("Kick");
}

void displaySelectColor(int selected_color_flag_){
  int blinkInterval = 500;
  // ---- 点滅用の状態更新 ----
  if(timer_Blinking.read_ms()> blinkInterval){
    if(blink_flag == 0){
      blink_flag  = 1;
    }
    else{
      blink_flag = 0;
    }
    timer_Blinking.reset();
  }

  display.clearDisplay();

  // ==== 文字サイズを大きく ====
  display.setTextSize(2);

  // ==== 座標設定（高さ中央に揃える）====
  const int y_pos = 24; // 64px 高さなので、2倍文字で中央は 24～40 あたり

  // ===========================
  //        Yellow の描画（左）
  // ===========================
  if(selected_color_flag_ == 0){
    if(blink_flag == 1){
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // 反転
    } 
    else{
      display.setTextColor(SSD1306_WHITE);
    }
  } 
  else{
    display.setTextColor(SSD1306_WHITE);
  }
  display.setCursor(0, y_pos);
  display.print("Yellow");


  // ===========================
  //         Blue の描画（右寄せ）
  // ===========================
  int textWidth = 6 * 2 * 4; 
  // "Blue" は 4 文字
  // 1文字幅6px × TextSize2 = 12px → 12 × 4 = 48px

  int x_blue = 128 - textWidth - 2; // 少し右に余白 (-2)

  if(selected_color_flag_ == 1){
    if(blink_flag == 1){
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); 
    } 
    else{
      display.setTextColor(SSD1306_WHITE);
    }
  } 
  else{
    display.setTextColor(SSD1306_WHITE);
  }
  display.setCursor(x_blue, y_pos);
  display.print("Blue");

  display.display();
}

void displayCommRoleSet(int change_commRole_) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 10);
  display.print("Device ID");

  display.setTextSize(3);
  if (change_commRole_ == 0) {
    display.setCursor(40, 35);
    display.print("DCX");
  } else {
    display.setCursor(40, 35);
    display.print("RE");
  }
  display.display();
}

void displayPartnerCheck(bool partner_isAttack_, uint32_t rtt_) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 5);
  display.print("Partner:");

  display.setTextSize(3);
  display.setCursor(10, 30);
  if (partner_isAttack_) {
    display.print("Attack");
  } else {
    display.print("Defence");
  }

  // Latency (RTT) の表示
  display.setTextSize(1);
  display.setCursor(10, 55);
  display.print("Latency: ");
  display.print(rtt_ / 1000.0, 1);
  display.print("ms");

  display.display();
}

void displaySetDir(int Now_DIR_){
  display.clearDisplay();

  // --- 文字サイズを大きく ---
  int textSize = 3;  // 好きな大きさに変更可能
  display.setTextSize(textSize);
  display.setTextColor(SSD1306_WHITE);

  // --- Dir: を左端に固定 ---
  int cursorX = 0;  // 左端に固定
  int cursorY = (SCREEN_HEIGHT - 8 * textSize) / 2;  // 縦方向中央
  display.setCursor(cursorX, cursorY);
  display.print("Dir:");

  // --- 数値を Dir: の右に表示 ---
  int numberX = cursorX + 6 * textSize * 4; // "Dir:" は4文字
  display.setCursor(numberX, cursorY);
  display.print(Now_DIR_);

  display.display();
}

void displayBlack(){
  display.clearDisplay();  // バッファをクリア
  display.display();       // 実際の画面を更新（真っ黒にする）
}

void rainbow(int wait) {
  static uint16_t hue = 0;

  for (int i = 0; i < NUMPIXELS; i++) {
    leds[i] = CHSV((hue + i * 256 / NUMPIXELS) & 0xFF, 255, 255);
  }

  FastLED.show();
  hue += 2;
  delay(wait);
}

/* ===== LED番号補正 ===== */
int set_rightPIXNUM(int target_num) {
  int goNum = target_num - 7;
  if (goNum > 15) goNum -= 16;
  if (goNum < 0)  goNum += 16;
  return goNum;
}

/* ===== 1個点灯 ===== */
void lightOne(int num, uint8_t r, uint8_t g, uint8_t b) {
  FastLED.clear();
  leds[num] = CRGB(r, g, b);
  FastLED.show();
}

/* ===== 複数点灯 ===== */
void lightMultiple(const int *nums, int count, uint8_t r, uint8_t g, uint8_t b) {
  FastLED.clear();
  for (int i = 0; i < count; i++) {
    leds[nums[i]] = CRGB(r, g, b);
  }
  FastLED.show();
}

/* ===== 配列に追加 ===== */
bool addTarget(int value) {
  if (value > 15) return false;               // 範囲外
  if (target_count >= MAX_NUM) return false;  // 満杯

  targets[target_count] = value;
  target_count++;
  return true;
}

void clearNeoPixel() {
  FastLED.clear();
  FastLED.show();
}

/* ===== すべてのLEDを点灯 ===== */
void lightAll(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < NUMPIXELS; i++) {
    leds[i] = CRGB(r, g, b);
  }
  FastLED.show();
}