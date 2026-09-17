#include "Cam.h"

void Cam::begin(unsigned long baud){
  Serial6.begin(baud);
  Serial7.begin(baud);
  moveave_cam.setup(3);
  moveave_ball.setup(3);
}

void Cam::attach(BALL& b, SIMPLIFY& s){
  ball = &b;
  simplify = &s;
}

void Cam::update_Front(){
  // if(Serial7.available() < 8){
  //   return;
  // }

  // if(Serial7.peek() != 255){
  //   Serial7.read();   // 1バイトだけ捨てる
  //   return;
  // }

  // Serial7.read(); // header = 255

  // ball_dot        = Serial7.read(); // BallX (0〜250)
  // ball_distance   = Serial7.read(); // BallY (0〜250)
  // blue_dot        = Serial7.read(); // BlueX
  // blue_width      = Serial7.read(); // BlueW
  // yellow_dot      = Serial7.read(); // YellowX
  // yellow_width    = Serial7.read(); // YellowW
  // Serial7.read(); // footer

  // if(selected_color == 1){
  //   byte_y_1 = yellow_dot;
  //   cam_width = (int)yellow_width;
  // }
  // else if(selected_color == 2){
  //   byte_y_1 = blue_dot;
  //   cam_width = (int)blue_width;
  // }

  // if(byte_y_1 < 250){
  //   last_byte_y_1 = byte_y_1;
  //   isCam_read = 1;
  // }
  // else{
  //   isCam_read = 0;
  // }

  // const float CENTER_VAL = 115;
  // const float HFOV = 60;
  // cam_angle = 1 * (float)(byte_y_1 - CENTER_VAL) * (HFOV / 250.0);
  // cam_angle = moveave_cam.add(cam_angle);

  // if(ball_dot < 250){
  //   isBall_read = 1;
  //   // ボール角度計算（ゴールと同じロジック）
  //   ball_angle = 1 * (float)(ball_dot - CENTER_VAL) * (HFOV / 250.0);
  //   ball_angle = moveave_ball.add(ball_angle);
  // }
  // else{
  //   isBall_read = 0;
  // }

  // // ===== 送信===== 
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

  if(Serial6.available() < 6){
    return;
  }

  trash_1 = Serial6.read();   
  byte_blue_y = Serial6.read();  
  byte_blue_x = Serial6.read();  
  byte_yellow_y = Serial6.read();   
  byte_yellow_x = Serial6.read();   
  trash_2 = Serial6.read();  

  // フレームチェック
  if(trash_1 != 255 || trash_2 != 255){
    byte_y_2 = last_byte_y_2;
    byte_x_2 = last_byte_x_2;
    Serial6.read();
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

  // cam_flag
  if(byte_y_2 < 250){
    cam_flag = 1;
    last_byte_y_2 = byte_y_2;
    last_byte_x_2 = byte_x_2;
  }
  else{
    cam_flag = 0;
  }

  int far = 150;
  // return_flag
  if(byte_x_2 < far){
    if(far < last_byte_x_2){
      timer_return.reset();
    }
    if(1000 <= timer_return.read_ms()){
      return_flag = 1;
    }
  }
  if(byte_x_2 >= (far + 10)){
    return_flag = 0;
  }

  if(cam_flag == 0){
    Back_cam_angle = Last_Back_cam_angle;
  }
  else{
    Back_cam_angle = (((byte_y_2 - 122.5) /2.5 +180) - 10);
  }
  // Serial.print("cam_ : ");
  // Serial.print(cam_flag);
  // Serial.print(" | x : ");
  // Serial.print(byte_x_2);
  // Serial.print(" | R : ");
  // Serial.println(return_flag);
  // Front_cam_angle = (((byte_y_2 - 122.5) / 2.5) * (-1));
  Last_Back_cam_angle = Back_cam_angle; 

  last_byte_x_2 = byte_x_2;
  last_byte_y_2 = byte_y_2;
}

void Cam::update_OMNICAM(){
  while(Serial7.available() >= 6){

    if(Serial7.peek() != 255){
      Serial7.read();
      continue;
    }

    Serial7.read(); // header

    OMNICAM_byte_blue_c   = Serial7.read();
    OMNICAM_byte_blue_s   = Serial7.read();
    OMNICAM_byte_yellow_c = Serial7.read();
    OMNICAM_byte_yellow_s = Serial7.read();
    byte footer         = Serial7.read();

    if(footer != 0){   // ← 今のログ的にfooterは0
      continue;
    }

    OMNICAM_angle_blue_c  = OMNICAM_byte_blue_c * 2;
    OMNICAM_angle_blue_s  = OMNICAM_byte_blue_s * 2;
    OMNICAM_angle_yellow_c = OMNICAM_byte_yellow_c * 2;
    OMNICAM_angle_yellow_s = OMNICAM_byte_yellow_s * 2;

    if(selected_color == 1){
      OMNICAM_angle_flont = -simplify->goPM((float)OMNICAM_angle_yellow_s);
      OMNICAM_opposite    = -simplify->goPM((float)OMNICAM_angle_blue_c);
    }
    else if(selected_color == 2){
      OMNICAM_angle_flont = -simplify->goPM((float)OMNICAM_angle_blue_s);
      OMNICAM_opposite    = -simplify->goPM((float)OMNICAM_angle_yellow_c);
    }

    if(OMNICAM_angle_flont == -150) OMNICAM_angle_flont = 999;
    if(OMNICAM_opposite == -150)    OMNICAM_opposite = 999;
  }
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
  return byte_height;
}

float Cam::getFrontOMNICAMAngle() const{
  return OMNICAM_angle_flont;
}

float Cam::getOppositeOMNICAMAngle() const{
  return OMNICAM_opposite;
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