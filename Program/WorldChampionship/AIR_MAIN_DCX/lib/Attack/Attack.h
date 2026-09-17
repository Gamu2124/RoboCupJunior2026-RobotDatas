#pragma once
#include <Arduino.h>
#include <LINE_test.h>
#include <MOTOR.h>
#include <BNOPID.h>
#include <MA.h>
#include <BALL.h>
#include <Cam.h>
#include <TIMER.h>
#include <KICK.h>
#include <simplify.h>

class Attack {
private:

  BALL* ball;
	BNOPID* bnopid;
  MOTOR* motor;
  KICK* kick;
 	LINE_TEST* line_test;
  Cam* camera = nullptr;
	MA moveave_far;
	MA moveave_ball_catch;
	SIMPLIFY* simplify = nullptr;
  
  // ===== 状態 =====
  int A = 0;
  int B = 0;
  int sideLine_trase_flag = 0;

	// ===== 情報 =====
  float Last_Line_angle = 0;
	float Ball_angle = 0;
  int get_Ball_far = 0;
	float Line_vec[3];
  float Bangle_LEGEND = 0;
  int read_LINE = 0;
  int vchange_on = 0;
  float Last_pid_angle = 0;
  int uart_catch = 0;
  float cam_angle = 0;
  int now_ = 0;
  float speed_val = 0;
  int isCam_read = 0;
  int Line_vals[24];
  int side_vals[4];
  float Line_side_vec[3];
  float KICK_DISS = 10.0; // ±5°
  int isHold = 0;
  byte byte_height;
  float default_Line_angle = 0;
  float OMNICAM_center = 0;
  float OMNICAM_flont = 0;
  float OMNICAM_opposite = 0;
  float cam_height = 0;
  int push_flag = 0;
  int isCatch_Ball_pid = 0;
  float NOW_DIR = 0;

  float cam_angle_wDIR = 0;
  int DEFAULT_FARVAL = 50;
  int NOCAM_FARVAL = 30;

  const int MIN_SPEED = 70;
  const int MAX_SPEED = 185;

  int isBackcam_read = 0;
  int nm_goal_flag = 0;
  int Last_nm_goal_flag = 0;
  float Back_cam_angle = 0;

	// ===== 出力 =====
  float go_angle = 0;
  float motor_speed = 0;
  int motor_flag = 0;
  float pid_angle = 0;
  float camm_width = 0;
  int cam_dot = 0;
  float Last_Ball_angle = 0;
  bool lostProcessed = false;
  // ===== タイマー =====
	TIMER timer_goBack;
  TIMER timer_hold;
  TIMER timer_catch;
  TIMER timer_Linecheck;
  TIMER timer_pushIn;
  TIMER timer_wait;
  TIMER timer_goCenter;
  TIMER timer_kick_delay;
  TIMER timer_return_court;
  TIMER timer_startDash;
  TIMER timer_calcBLDC;
  TIMER timer_sideStop;
  TIMER timer_300;
  TIMER timer_avoid_mygoal;
  bool returnTo300 = false;
  TIMER timer_sideGetBall;
  TIMER timer_push;
  const float CENTER_VAL = 115;
  const float HFOV = 60;

  float tekiyoke_cam_angle = 0;
  // ===== 内部フラグ =====
  int active_Kick = 0;
  int startLine_timer = 0;
  int static_state_arr[3] = {0, 0, 0};
  int static_state = 0;

  int kick_delay_flag = 0;
  int isdiss_in = 0;

  unsigned long kick_delay = 100;

  void push_front3(int value);

  unsigned long startTime;
  unsigned long finishTime;
  float Last_go_angle = 0;

  float kotei_pid = 0;

  int readLine_count = 0;
  int Last_isStart = 0;
  float startdash_ang = 0;
  int chatta = 0;
  float locked_pid = 0;
  float line_locked_pid = 0;
  int way = 0;
  int side_getBall_flag = 0;


  // 角度スムーズ用メンバ変数
  float filteredAttitude = 0.0f;
  bool attitudeInit = false;

  // チャタリング防止用メンバ変数
  float lastGoPM = 0.0f;
  bool hasLastGoPM = false;

public:
  void begin();
  void run();
  // void runSimple(int speed = 150);
	int kick_step = 0;
  int isLine_read = 0;
	float Line_angle = 0;
  float Line_depth = 0;
  void attach(BNOPID& bno, MOTOR& mot, KICK& k, BALL& b, LINE_TEST& lt, SIMPLIFY& s, Cam& cam);
};
