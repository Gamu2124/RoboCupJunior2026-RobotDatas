#include "UI.h"

void UI::begin(unsigned long baud){
  Serial4.begin(baud);
  moveave_far.setup(10);
}

void UI::attach(BNOPID& bno, LINE_TEST& lt, Cam& c, BALL& b, SIMPLIFY& s, Attack& a)
{
  bnopid    = &bno;
  line_test = &lt;
  cam       = &c;
  ball      = &b;
  attack    = &a;
  simplify = &s;
}


void UI::update(){
  for(int i = 0; i < 2; i++){
    Line_vec[i] = 0;
    Line_side_vec[i] = 0;
  }

  Ball_angle = ball->getAngle();
  get_Ball_far = moveave_far.add(ball->getFar());
  Line_angle = line_test->angle(Line_vec, Line_side_vec); //sincos
  if(Line_angle != 999){
    Line_depth = sqrt(Line_vec[0]*Line_vec[0] + Line_vec[1]*Line_vec[1]);
  }
  else{
    Line_depth = 999;
  }
  catch_val = ball->catch_sent();
  KICK_VAL = ball->Kick_val_sent();
}

void UI::commu(uint32_t lastreset){
  int array_length = 4;

  if(Serial4.available() >= 3){
    if(Serial4.read() == 255){
      byte cmd = Serial4.read();
      Serial4.read();  // dummy

      if(cmd == 1 || cmd == 2) selected_color = int(cmd);
      if(cmd == 3 || cmd == 4) selected_role = int(cmd);

      if(cam){
        cam->setSelectedColor(selected_color);
      }
      // if(cmd == 15) array_length = 3;

      byte array_toSend[array_length];
      // memset(array_toSend, 0, sizeof(array_toSend)); // ★ゼロ初期化
      array_toSend[0] = 255;

      byte send_angle;
      byte send_depth;

      if(Line_angle == 999) send_angle = 255;
      else send_angle = (byte)(Line_angle / 2);

      if(Line_depth == 999) send_depth = 255;
      else send_depth = (byte)(Line_depth * 100);

      float now = bnopid->getDIR();
      if(now < 0) now += 360;

      switch(cmd){
        case 5:
          array_toSend[1] = int(Ball_angle/2);
          array_toSend[2] = get_Ball_far;
          break;

        case 10:
          array_toSend[1] = (byte)send_angle;
          array_toSend[2] = (byte)send_depth;
          break;

        case 15:
          array_toSend[1] = (byte)(now/2);
          array_toSend[2] = 128;
          isToggle_ON = 0;
          break;

        case 20:
          array_toSend[1] = KICK_VAL;
          array_toSend[2] = catch_val/2;
          break;
        case 25:
          set_dir_request = 1;
          array_toSend[1] = 0;
          array_toSend[2] = 0;
          break;

        case 30:
          kick_request = 1;
          break;

        case 40:
          array_toSend[1] = lastreset;
          array_toSend[2] = 128;
          break;

        case 100:
          isToggle_ON = 0;
          break;
        
        case 150:
          isToggle_ON = 1;
          array_toSend[1] = attack->kick_step;
          array_toSend[2] = 128;
          break;
      }
      Serial4.write(array_toSend, array_length);
    }
    else{
      Serial4.read();
    }
  }
}

int UI::getRole(){
  return selected_role;
}

int UI::KickRequest(){
  if(kick_request == 1){
    kick_request = 0;
    return 1;
  }
  return 0;
}

int UI::SetDirRequest(){
  if(set_dir_request == 1){
    set_dir_request = 0;
    return 1;
  }
  return 0;
}

int UI::getRobotStartRequest(){
  return isToggle_ON;
}
