#include "Cam.h"

void Cam::begin(unsigned long baud){
  Serial6.begin(baud);
  Serial7.begin(baud);
  Serial2.begin(baud);
  moveave_cam.setup(3);
  moveave_ball.setup(3);
}

void Cam::attach(BALL& b, SIMPLIFY& s){
  ball = &b;
  simplify = &s;
}

void Cam::update_Front(){
  if(Serial6.available() < 8){
    return;
  }

  if(Serial6.peek() != 255){
    Serial6.read();   // 1バイトだけ捨てる
    return;
  }

  Serial6.read(); // header = 255

  ball_dot        = Serial6.read(); // BallX (0〜250)
  ball_distance   = Serial6.read(); // BallY (0〜250)
  blue_dot        = Serial6.read(); // BlueX
  blue_width      = Serial6.read(); // BlueW
  yellow_dot      = Serial6.read(); // YellowX
  yellow_width    = Serial6.read(); // YellowW
  Serial7.read(); // footer

  // selected_color = 2;
  if(selected_color == 1){
    byte_y_1 = yellow_dot;
    cam_width = (int)yellow_width;
  }
  else if(selected_color == 2){
    byte_y_1 = blue_dot;
    cam_width = (int)blue_width;
  }

  if(byte_y_1 < 250){
    last_byte_y_1 = byte_y_1;
    isCam_read = 1;
  }
  else{
    isCam_read = 0;
  }

  const float CENTER_VAL = 115;
  const float HFOV = 60;
  cam_angle = 1 * (float)(byte_y_1 - CENTER_VAL) * (HFOV / 250.0);
  cam_angle = moveave_cam.add(cam_angle);

  if(ball_dot < 250){
    isBall_read = 1;
    // ボール角度計算（ゴールと同じロジック）
    ball_angle = 1 * (float)(ball_dot - CENTER_VAL) * (HFOV / 250.0);
    ball_angle = moveave_ball.add(ball_angle);
  }
  else{
    isBall_read = 0;
  }

  // ===== 送信===== 
  // if(ball && ball->isCaught()){
  //   Serial7.write(255); 
  //   Serial7.write(1); 
  //   Serial7.write(255);
  // }
  // else{ 
  //   Serial7.write(255); 
  //   Serial7.write(255); 
  //   Serial7.write(255);
  // }
  
}


void Cam::update_Back(){
  byte trash_1;
  byte trash_2;

  if(Serial7.available() < 6){
    return;
  }

  trash_1 = Serial7.read();   
  byte_blue_y = Serial7.read();  
  byte_blue_x = Serial7.read();  
  byte_yellow_y = Serial7.read();   
  byte_yellow_x = Serial7.read();   
  trash_2 = Serial7.read();  

  // フレームチェック
  if(trash_1 != 255 || trash_2 != 255){
    byte_y_2 = last_byte_y_2;
    byte_x_2 = last_byte_x_2;
    Serial7.read();
    return;
  }

  // selected_color = 1;
  if(selected_color == 1){
    byte_y_2 = byte_blue_y;
    byte_x_2 = byte_blue_x;
  }
  else if(selected_color == 2){
    byte_y_2 = byte_yellow_y;
    byte_x_2 = byte_yellow_x;
  }

  // Serial.print("Back_cam : ");
  // Serial.println(byte_x_2);
  // cam_flag
  if(byte_y_2 < 250){
    cam_flag = 1;
    last_byte_y_2 = byte_y_2;
    last_byte_x_2 = byte_x_2;
  }
  else{
    cam_flag = 0;
  }

  int far_on = 150;
  int far_off = 135;
  // Serial.println((byte_x_2));
  // --- フラグ管理の修正部分 ---
  if (int(byte_x_2) >= far_on) {
    return_flag = 1;
  } 
  else return_flag = 0;
  // else if (byte_x_2 < far_off) {
  //   return_flag = 0;
  // }
  // ----------------------------

  
  Back_cam_angle = (((byte_y_2 - 122.5) / 2.5 + 180) - 10);
  
  // Serial.print("cam_ : ");
  // Serial.print(cam_flag);
  // Serial.print(" | x : ");
  // Serial.print(byte_x_2);
  // Serial.print(" | R : ");
  // Serial.println(return_flag);
  // // Front_cam_angle = (((byte_y_2 - 122.5) / 2.5) * (-1));
  Last_Back_cam_angle = Back_cam_angle; 

  last_byte_x_2 = byte_x_2;
  last_byte_y_2 = byte_y_2;
}

void Cam::update_OMNICAM(){
  if(Serial2.available() < 4){
    return;
  }
  if(Serial2.read() == 255){
    OMNICAM_byte_blue = Serial2.read();
    OMNICAM_byte_yellow = Serial2.read();
    OMNICAM_byte_green = Serial2.read();
    Serial2.read();
  }
  else{
    Serial2.read();
  }

  OMNICAM_angle_blue   = OMNICAM_byte_blue * 2;
  OMNICAM_angle_yellow = OMNICAM_byte_yellow * 2;
  OMNICAM_angle_green = int(OMNICAM_byte_green)*2;

  if(selected_color == 1){
    OMNICAM_angle_flont = -simplify->goPM((float)OMNICAM_angle_yellow);
    OMNICAM_opposite    = -simplify->goPM((float)OMNICAM_angle_blue);
  }
  else if(selected_color == 2){
    OMNICAM_angle_flont = -simplify->goPM((float)OMNICAM_angle_blue);
    OMNICAM_opposite    = -simplify->goPM((float)OMNICAM_angle_yellow);
  }
  OMNICAM_center = OMNICAM_angle_green;
  OMNICAM_center = -simplify->goPM((float)OMNICAM_center);

  if(OMNICAM_angle_flont == -150) OMNICAM_angle_flont = 999;
  if(OMNICAM_opposite == -150)    OMNICAM_opposite = 999;
  if(OMNICAM_center == -150)      OMNICAM_center = 999;
}


int Cam::getCamFlag() const {
  return cam_flag;
}

int Cam::getReturnFlag() const {
  return return_flag;
}

float Cam::getBackAngle() const {
  return Back_cam_angle;
}

int Cam::getFrontread() const{
  return isCam_read;
}

float Cam::getFrontAngle() const{
  return cam_angle;
}

float Cam::getFrontheight() const{
  return cam_height;
}

float Cam::getFrontOMNICAMAngle() const{
  return OMNICAM_angle_flont;
}

float Cam::getOppositeOMNICAMAngle() const{
  return OMNICAM_opposite;
}

float Cam::getOMNICAMcenter() const{
  return OMNICAM_center;
}

float Cam::getFrontwidth() const{
  return cam_width;
}

int Cam::getFrontdot() const{
  return int(byte_y_1);
}

// ボールが見えているか (1: 見えている, 0: ロスト)
int Cam::getBallRead() const {
  return isBall_read;
}

// ボールの角度を取得
float Cam::getBallAngle() const {
  return ball_angle;
}

// ボールの距離（生データ ball_distance: 0〜250）を取得
// 数値が小さいほど遠く、大きいほど近い（またはその逆、カメラの映り方次第）
float Cam::getBallDistance() const {
  return (float)ball_distance;
}