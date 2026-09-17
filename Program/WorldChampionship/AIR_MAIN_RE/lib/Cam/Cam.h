#pragma once
#include <Arduino.h>
#include <BALL.h>
#include <MA.h>
#include <TIMER.h>
#include <simplify.h>

class Cam{
private:

  BALL* ball = nullptr;
  SIMPLIFY* simplify = nullptr;
  MA moveave_cam;
  MA moveave_ball;

  // ===== Front Cam (Serial6) =====
  byte byte_x_1 = 255;
  byte byte_y_1 = 255;
  byte last_byte_x_1 = 255;
  byte last_byte_y_1 = 255;
  byte blue_angle;
  byte blue_dot;
  byte blue_height;
  byte blue_width;
  byte yellow_angle;
  byte yellow_height;
  byte yellow_dot;
  byte yellow_width;
  byte byte_height;
  byte Last_byte_height;
  byte ball_dot;
  byte ball_distance;
  int selected_color = 0;
  int isCam_read = 0;
  float cam_angle = 0;
  int uart_catch = 0;
  int cam_height = 0;
  float cam_width = 0;
  float ball_angle = 0.0;
  int isBall_read = 0;

  // ===== フラグ =====
  int cam_flag = 0;
  int return_flag = 0;

  // ===== Back Cam (Serial7) =====
  byte byte_x_2 = 255;
  byte byte_y_2 = 255;
  byte last_byte_x_2 = 255;
  byte last_byte_y_2 = 255;
  byte byte_blue_y = 255;  
  byte byte_blue_x = 255;  
  byte byte_yellow_y = 255;   
  byte byte_yellow_x = 255;  

  // ===== OMNICAM Cam (Serial7) =====
  byte OMNICAM_byte_blue_c = 0;
  byte OMNICAM_byte_blue_s = 0;
  byte OMNICAM_byte_yellow_c = 0;
  byte OMNICAM_byte_yellow_s = 0;
  int OMNICAM_angle_blue_c = 0;
  int OMNICAM_angle_blue_s = 0;
  int OMNICAM_angle_yellow_c = 0;
  int OMNICAM_angle_yellow_s = 0;
  float OMNICAM_angle_flont = 0;
  float OMNICAM_opposite = 0;

  // ===== 角度 =====
  float Back_cam_angle = 0;
  float Last_Back_cam_angle = 0;


  TIMER timer_return;

public:
  void begin(unsigned long baud);
  void setSelectedColor(int color){
    selected_color = color;
  }
  void attach(BALL& b, SIMPLIFY& s);
  void update_Front();          // ← serialEvent6
  void update_Back();          // ← serialEvent7
  void update_OMNICAM();
  int getCamFlag() const;
  int getReturnFlag() const;
  float getBackAngle() const; 
  int getFrontread() const;  
  float getFrontAngle() const;
  float getFrontheight() const;
  float getFrontOMNICAMAngle() const;
  float getOppositeOMNICAMAngle() const;
  float getFrontwidth() const;
  int getFrontdot() const;
  int getBallRead() const;
  float getBallAngle() const;
  float getBallDistance() const;
};
