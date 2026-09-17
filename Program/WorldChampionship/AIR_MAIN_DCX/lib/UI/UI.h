#pragma once
#include <Arduino.h>
#include <BNOPID.h>
#include <LINE_test.h>
#include <Cam.h>
#include <MA.h>
#include <simplify.h>
#include <Attack.h>

class UI{
private:
    BNOPID* bnopid;
    LINE_TEST* line_test;
    Cam* cam;
    BALL* ball;
    SIMPLIFY* simplify;
    Attack* attack;
    MA moveave_far; 

    int selected_color = 0;
    int selected_role = 0;
    float Line_depth = 0;
    float Line_angle = 0;
    float Ball_angle = 0;
    int get_Ball_far = 0;
    int KICK_VAL = 0;
    int catch_val = 0;
    int Line_vals[24];
    int side_vals[4];
    float Line_vec[3];
    float Line_side_vec[3];
    int kick_request = 0;
    int isToggle_ON = 0;
    int set_dir_request = 1;

  

public:
  void begin(unsigned long baud);
  void update();
  void commu(uint32_t);          // ← serialEvent2
  void attach(BNOPID& bno, LINE_TEST& lt, Cam& c, BALL& b, SIMPLIFY& s, Attack& a);
  int kick_cmd = 0;
  int getRole();
  int KickRequest();
  int SetDirRequest();
  int getRobotStartRequest();
};
