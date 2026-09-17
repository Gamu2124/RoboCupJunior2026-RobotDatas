#include <Arduino.h>
#include <LINE_test.h>
#include <MOTOR.h>
#include <BNOPID.h>
#include <MA.h>
#include <BALL.h>
#include <KICK.h>
#include "UI.h"
#include "USS.h"
#include "Defence.h"
#include "Attack.h"
#include <ESCControl.h>

BALL ball;
BNOPID bnopid;
SIMPLIFY simplify;
MOTOR motor;
LINE_TEST line_test;
KICK kick;
Cam cam; 
UI ui;
USS uss;
Defence defence;
Attack attack;
const int escPin = 3;
const int comm = 37;
const int comm2 = 38;
const int comm3 = 39;
ESCControl esc(escPin);
//スイッチ
int isToggle_ON = 0;
int selected_role = 0;
int kick_request = 0;  
int robot_start = 0;
const int motor_pin = 2;
uint32_t lastResetCause = 0;

void checkResetCause() {
  // 起動直後のレジスタ値をコピー
  lastResetCause = SRC_SRSR;

  // レジスタをクリア（次のリセットに備える）
  SRC_SRSR = lastResetCause;

  // 原因に応じたログ出力や安全対策
  if (lastResetCause & (1 << 4)) {
    // 【重要】ウォッチドッグが原因なら、何か異常があった証拠
    // ここでモーター出力を強制停止するなどの安全処理を入れる
    Serial.println("ALERT: Recovered from Watchdog Reset!");
  } else if (lastResetCause & (1 << 0)) {
    Serial.println("Normal Power-on.");
  }
}

//First.

void setup()
{
  delay(500);
  Serial.begin(9600);
  if(CrashReport){
    Serial.println(CrashReport);
  }

  checkResetCause();
  line_test.begin(921600);
  ball.begin(921600);
  cam.begin(115200);
  cam.attach(ball, simplify);
  ui.begin(115200);
  ui.attach(bnopid, line_test, cam, ball, simplify, attack);
  uss.begin(115200);
  pinMode(motor_pin, OUTPUT);
  bnopid.setup();
  bnopid.set_target();
  attack.attach(bnopid, motor, kick, ball, line_test, simplify, cam);
  attack.begin();
  defence.attach(bnopid, motor, kick, ball, line_test, simplify, cam);
  defence.begin();
  pinMode(comm, INPUT);
  pinMode(comm2, INPUT);
  pinMode(comm3, INPUT);

  esc.begin();
    // while (!Serial) {
  //   // wait for USB serial port to connect (TelePlot / Serial Monitor)
  // }

  digitalWrite(motor_pin, LOW);
}


void loop()
{
  line_test.updateSensor();
  cam.update_Front();
  cam.update_Back();
  cam.update_OMNICAM();
  ball.updateBallSensor();
  ui.update();
  ui.commu(lastResetCause);
  float B_angle = ball.getAngle();
  uss.update();
  // Serial.println(B_angle);
  selected_role = ui.getRole();
  robot_start = ui.getRobotStartRequest();

  
  line_test.setRaw(selected_role);

  if(ui.SetDirRequest() == 1){
    bnopid.set_target();
  }
  if(ui.KickRequest() == 1){
    kick_request = 1;
  }

  // kick_request = 1;
  if(kick_request == 1){
    kick_request = kick.shoot(); 
  }
  // selected_role = 3;
  // robot_start = 1;
  // selected_role = 3;
  // robot_start = 1;
  if(selected_role == 3){          // Attack
    // attack.runSimple(150);
    esc.update();
    if(robot_start == 1){
      if (esc.isReady()) {
        if(300 < B_angle || B_angle < 60){
          esc.setPulseUs(1400);
        }
        else{
          esc.setPulseUs(1500);
        }
      }
    }
    else{
      esc.setPulseUs(1500);
    }
    attack.run();
  }
  else if(selected_role == 4){     // Defence 
    defence.run();
  }
  if((robot_start == 1 || ((digitalRead(comm) == 1) || digitalRead(comm2) == 1) || digitalRead(comm3) == 1) && (selected_role != 0)){
    digitalWrite(motor_pin, HIGH);
  }
  else{
    digitalWrite(motor_pin, LOW);
  }
  // static uint32_t lastBnoPrintMs = 0;
  // if (millis() - lastBnoPrintMs >= 100) {
  //   bnopid.print();
  //   lastBnoPrintMs = millis();     v 
  // }
  // Serial.println(analogRead(A8));
  // Serial.print("here");
}