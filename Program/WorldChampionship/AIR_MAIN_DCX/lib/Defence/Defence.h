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

class Defence {
private:

  BALL* ball = nullptr;
	BNOPID* bnopid = nullptr;
  MOTOR* motor = nullptr;
  KICK* kick = nullptr;
 	LINE_TEST* line_test = nullptr;
  Cam* camera = nullptr;
	MA moveave_ball_far;
  MA moveave_dash;
  MA moveave_dash_near;
	SIMPLIFY* simplify = nullptr;
	

  // ===== 状態 =====
  int A = 0;
  int B = 0;
  int L = 0;

	// ===== 情報 =====
	float Line_angle = 0;
  float Last_Line_angle = 0;
	int get_Ball_angle = 0;
  int get_Ball_far = 0;
	float get_Ball_far_ave = 0;
	float Line_vec[3];
  float Back_cam_angle = 0;
  float Last_Back_cam_angle = 0;
  int Line_vals[24];
  int side_vals[4];
  float Line_side_vec[3];
  float flont_cam_angle = 0;
  float OMNICAM_angle_flont = 0;
  float OMNICAM_opposite = 0;
  float OMNICAM_angle_flont_L = 0;
  float OMNICAM_opposite_L = 0;
  float Ball_angle = 0;
  int isCam_read = 0;

	// ===== 出力 =====
  float move_angle = 0;
  float motor_speed = 0;
  int motor_flag = 0;
  float pid_angle = 0;
  float camm_width = 0;
  int cam_dot = 0;

  // ===== タイマー =====
	TIMER timer_dash;
  TIMER timer_dash_near;
  TIMER timer_dash_kill;
  TIMER timer_return;
  TIMER timer_no_cam;
  TIMER timer_hold;

  const float CENTER_VAL = 115;
  const float HFOV = 60;

  float tekiyoke_cam_angle = 0;
  // ===== 内部フラグ =====
  int active_Kick = 0;
	int dash_flag = 0;
	int Last_dash_flag = 0;
	int uart_catch = 0;
	int return_flag = 0;
	int cam_flag = 0;
  int Last_cam_flag = 0;
  int dash_judge = 0;
  float dash_judge_ave = 0;
  int dash_flag_near = 0;
  int Last_dash_flag_near = 0;
  int dash_judge_near = 0;
  float dash_judge_naer_ave = 0;
  int dash_flag_keep = 0;


public:
  void begin();
  void run();
  int defence_step = 0;
  void attach(BNOPID& bno, MOTOR& mot, KICK& k, BALL& b, LINE_TEST& lt, SIMPLIFY& s, Cam& cam);
};
