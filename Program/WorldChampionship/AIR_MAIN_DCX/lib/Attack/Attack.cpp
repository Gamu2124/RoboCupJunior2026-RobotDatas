#include <Arduino.h>
#include "Attack.h"

void Attack::begin(){
  A = 5;
  B = 999;
  static_state = -1;
  for(int i = 0; i < 3; i++){
    static_state_arr[i] = -1;
  }
  moveave_far.setup(10);
}

void Attack::attach(BNOPID& bno, MOTOR& mot, KICK& k, BALL& b, LINE_TEST& lt, SIMPLIFY& s, Cam& cam)
{
  bnopid = &bno;
  motor  = &mot;
  kick   = &k;
  ball = &b;
  line_test = &lt;
  simplify = &s;
  camera = &cam;
}

void Attack::push_front3(int value) {
  static_state_arr[2] = static_state_arr[1];
  static_state_arr[1] = static_state_arr[0];
  static_state_arr[0] = value;
}

void Attack::run(){
  if(timer_sideGetBall.read_ms() > 1500 && side_getBall_flag != 0){
    side_getBall_flag = 0;
  }
  for(int i = 0; i < 2; i++){
    Line_vec[i] = 0;
    Line_side_vec[i] = 0;
  }
  vchange_on = 1;
  motor_flag = 0;
  isHold = 0;
  now_ = 0;
  isCatch_Ball_pid = 0;

  Ball_angle = ball->getAngle();
  get_Ball_far = moveave_far.add(ball->getFar());
  isCam_read = camera->getFrontread();
  cam_angle = camera->getFrontAngle();
  byte_height = camera->getFrontheight();
  isBackcam_read = camera->getCamFlag();
  nm_goal_flag = camera->getReturnFlag();
  Back_cam_angle = camera->getBackAngle();
  bool isLineNew = line_test->isRead_Line();
  OMNICAM_center = camera->getOMNICAMcenter();
  OMNICAM_flont = camera->getFrontOMNICAMAngle();
  OMNICAM_opposite = camera->getOppositeOMNICAMAngle();
  float OMNICAM_flont_wDIR = simplify->goPM(OMNICAM_flont) + bnopid->getDIR();
  cam_height = camera->getFrontheight();
  NOW_DIR = bnopid->getDIR();
  camm_width = camera->getFrontwidth();
  cam_dot = camera->getFrontdot();
  int cam_ball_read     = camera->getBallRead();     // 1: 見えている, 0: ロスト
  float cam_ball_angle  = camera->getBallAngle();    // 正面を0度とした左右の角度誤差
  float cam_ball_dist   = camera->getBallDistance(); // カメラからの生距離（0〜250）

  float default_Ball_angle = Ball_angle;
  if(cam_ball_read == 1){
    Ball_angle = simplify->FixLimit(cam_ball_angle);
  }
  else{
    if(Ball_angle == 999){
      Ball_angle = 999;
    }
  }
  ball->updateCatch(Ball_angle);
  isHold = ball->isCaught();
  Serial.println(Ball_angle);

  if(isHold == 1) Ball_angle = 0;
  if(isLineNew){
    Line_angle = line_test->angle(Line_vec, Line_side_vec); //sincos
    default_Line_angle = Line_angle;
    if(Line_angle != 999){
      isLine_read = 1;
    }
    else{
      isLine_read = 0;
    }
    Line_depth = line_test->getDepth();
  }
  if(A == 5){ //ボールなし処理
    B = A;

    if(Ball_angle != 999) A = 10;
    if(Line_angle != 999) A = 20;

    if(B == A){
      go_angle = 0;
      motor_speed = 0;
      motor_flag = 1;
      locked_pid = 0;
      pid_angle = bnopid->get_pd(0);
    }
  }

  if(A == 7){
    B = A;

    if(Ball_angle != 999) A = 10;
    if(Line_angle != 999) A = 20;

    if(timer_goBack.read_ms() < 800){
      if(B == A){
        go_angle = 180;
        pid_angle = bnopid->get_pd(0);
        locked_pid = 0;
        motor_speed = 150;
        motor_flag = 1;
      }
    }
    else{
      timer_goCenter.reset();
      A = 5;
    }
  }

  if(A == 8){
    B = A;

    if(Ball_angle != 999) A = 10;
    if(Line_angle != 999) A = 20;

    if(timer_goBack.read_ms() < 600){
      if(B == A){
        go_angle = 180;
        locked_pid = 0;
        pid_angle = bnopid->get_pd(0);
        motor_speed = 150;
        motor_flag = 1;
      }
    }
    else{
      timer_goCenter.reset();
      A = 5;
    }
  }

  if(A == 10){ //通常処理
    B = A;

    if(isCam_read == 1 && isHold == 1){
      A = 30;
      timer_catch.reset();
      kick_step = 2;
    }
    else{
      if(isCam_read == 1) kick_step = 1;
      else kick_step = 0;
    }

    if(Ball_angle == 999){
      if(isCam_read == 0){
        A = 7;
        timer_goBack.reset();
      }
      else{
        A = 5;
      }
    }

    if(Line_angle != 999) A = 20;

    if(B == A){
      std::pair<float, int> result = ball->around(int(Ball_angle), (float)get_Ball_far, 0, 50, 230);
      go_angle = result.first;
      int mySpeed = result.second;

      motor_flag = 1;
      float angle_ = Ball_angle;
      if(angle_ > 180){
        angle_ -= 360;
      }
      float use_angle = Ball_angle;
      if(use_angle > 180){
        use_angle -= 360;
      }

      motor_speed = 175;
      float ball_pm = abs(simplify->goPM(Ball_angle));
      if(get_Ball_far > 40){
        if(10 <= ball_pm && ball_pm < 15){
          motor_speed -= 10;
        }
        else if(ball_pm < 10){
          motor_speed -= 15;
        }
      }

      bool isthis = 0;
      float use_cam = cam_angle + NOW_DIR;
      float use_ball = simplify->goPM(Ball_angle) + NOW_DIR;
      if((cam_angle >= 0 && use_ball >= 0) || (cam_angle < 0 && use_ball < 0)) isthis = 1;

      float smoothAng = (float)Ball_angle;
      float currentGoPM = simplify->goPM(smoothAng);

      if (!hasLastGoPM) {
        lastGoPM = currentGoPM;
        hasLastGoPM = true;
      }
      else {
        float diff = currentGoPM - lastGoPM;
        if (diff < -180.0f) diff += 360.0f;
        if (diff >= 180.0f) diff -= 360.0f;

        if (fabsf(diff) > 120.0f) {
          currentGoPM = lastGoPM;
        } else {
          lastGoPM = currentGoPM;
        }
      }

      float rawAttitude = bnopid->getDIR() + currentGoPM;

      float attitudeDeg = rawAttitude;
      if (attitudeDeg > 180.0f)  attitudeDeg -= 360.0f;
      if (attitudeDeg < -180.0f) attitudeDeg += 360.0f;

      if (!attitudeInit) {
        filteredAttitude = attitudeDeg;
        attitudeInit = true;
      } else {
        float absDeg = fabsf(attitudeDeg);
        float alpha = 0.20f;

        if (absDeg < 2.0f) {
          attitudeDeg = 0.0f;
          alpha = 0.10f;
        }
        else if (absDeg < 8.0f) {
          alpha = 0.05f;
        }
        else if (absDeg > 25.0f) {
          alpha = 0.40f;
        }

        float diff = attitudeDeg - filteredAttitude;
        if (diff > 180.0f)  diff -= 360.0f;
        if (diff < -180.0f) diff += 360.0f;

        filteredAttitude += alpha * diff;

        if (filteredAttitude > 180.0f)  filteredAttitude -= 360.0f;
        if (filteredAttitude < -180.0f) filteredAttitude += 360.0f;
      }
      float setAttitude = filteredAttitude;

      float useAttitude = 0.0f;
      if(fabsf(currentGoPM) < 25.0f){ // ボールが正面付近(25度未満)ならその方向へ姿勢制御
        useAttitude = setAttitude;
      }

        pid_angle = bnopid->get_pd(0);
        locked_pid = 0;

      if(abs(abs(use_ball)-abs(cam_angle)) < 35 && get_Ball_far > 30 && abs(use_cam) < 30){
        if(isCam_read == 1 && isthis == 1){
          pid_angle = bnopid->get_pd(use_cam);
          locked_pid = use_cam;
          isCatch_Ball_pid = 0;
        }
      }
      motor_speed = mySpeed;
    }
  }

  if(A == 20){ // ライン処理
    B = A;

    if(Line_angle == 999){
      A = 10;

      float dissLINE = abs(Last_Line_angle - Bangle_LEGEND);
      if(dissLINE < 70 || dissLINE > 290){
        read_LINE = 1;
      }
    }
    else{
      if(read_LINE){
        read_LINE = 0;
        Bangle_LEGEND = 0;
      }
      Last_Line_angle = Line_angle;
    }

    if(B == A){
      go_angle = simplify->FixLimit(Line_angle + 180);
      motor_speed = 210;
      pid_angle = bnopid->get_pd(0);
      motor_flag = 1;
      vchange_on = 0;
      Bangle_LEGEND = Ball_angle;
    }

    isCatch_Ball_pid = 0;
  }

  float camm = cam_angle;
  if(A == 30){ //持ってる時処理
    KICK_DISS = 5.0f;
    if(Ball_angle == 999){
      if(isCam_read == 0){
        A = 7;
        timer_goBack.reset();
      }
      else{
        A = 5;
      }
    }
    if(isHold == 0 || isCam_read == 0) A = 10;
    if(Line_angle != 999) A = 20;

    int kick_threshold = 0;
    if(abs(cam_angle) < KICK_DISS) kick_threshold = 1;

    pid_angle = bnopid->get_pd(cam_angle + bnopid->getDIR());

    if(B == A){
      float thisthis = 0;
      go_angle = 0;

      isCatch_Ball_pid = 1;
      vchange_on = 0;
      motor_flag = 1;
    }

    motor_speed = 200;
  }
  else{
    uart_catch = 0;
  }

  if(A == 40) //押し込み
  {
    B = A;
    if(Line_depth < -0.8){
      A = 48;
    }
    else if(abs(simplify->goPM(Ball_angle)) > 30){
      A = 48;
    }
    else{
      go_angle = Ball_angle;
      motor_speed = 70;
    }

    if(B == A){
      if(isHold == 1) active_Kick = 1;
      pid_angle = bnopid->get_pd(0);
      motor->calc(float(go_angle), pid_angle, int(motor_speed), int(isCatch_Ball_pid), cam_angle);
    }
  }

  if(A == 48) //押し込みから復帰
  {
    if(B != A){
      B = A;
      timer_return_court.reset();
    }
    if(timer_return_court.read_ms() > 300 && Line_angle == 999){
      A = 10;
      sideLine_trase_flag = 0;
      startLine_timer = 0;
    }
    else{
      go_angle = 180;
      motor_speed = 200;
    }

    if(B == A){
      pid_angle = bnopid->get_pd(0);
      motor->calc(float(go_angle), pid_angle, int(motor_speed), int(isCatch_Ball_pid), cam_angle);
    }
  }

  if(A == 200){ //ホールド待機
    B = A;
    if(isHold != 1) A = 10;
    if(timer_calcBLDC.read_ms() > 800){
      A = 250;
      timer_calcBLDC.reset();
    }
    read_LINE = 0;

    if(B == A){
      motor->calc(180, bnopid->get_pd(0), 0, int(isCatch_Ball_pid), cam_angle);
    }
  }

  if(A == 250){ //引き戻し
    B = A;
    if(Line_angle != 999) A = 20;
    if(isCam_read == 1 || isHold != 1) A = 10;
    motor_speed = int(timer_calcBLDC.read_ms() / 20); //スピードを徐々に上げる感じ
    if(motor_speed > 200){
      motor_speed = 200;
    }
    if(B == A){
      motor->calc(180, bnopid->get_pd(0), motor_speed, int(isCatch_Ball_pid), cam_angle);
    }
  }

  if(A == 300){ //
    B = A;
    if(Line_angle != 999) A = 20;

    if(B == A){
      motor->calc(0, bnopid->get_pd(simplify->goPM(Ball_angle)), 100, int(isCatch_Ball_pid), cam_angle);
    }
  }

  if(isHold == 1 && A == 30){
    NOW_DIR = bnopid->getDIR();
    if(NOW_DIR >= 0) go_angle = 15;
    else if(NOW_DIR < 0) go_angle = 345;
    motor_speed = 230;
    KICK_DISS = 5;
    if(abs(cam_angle) < KICK_DISS) active_Kick = 1;
    pid_angle = bnopid->get_pd(cam_angle + bnopid->getDIR());
    locked_pid = cam_angle + bnopid->getDIR();
    motor_flag = 1;
  }

  if(motor_flag == 1){
    if(nm_goal_flag == 1) {
      float ball_pm = simplify->goPM(Ball_angle); // -180 〜 180度
      if(abs(ball_pm) > 90.0f) {
        motor_speed = 0;
        go_angle = 0;
      }
    }
    if(B != 5){
      get_Ball_far = constrain(get_Ball_far, 0, 50);

      int danger_ang = 45;
      for(int i = 0; i < 4; i++){
        if((danger_ang + i * 90 - 10) <= go_angle && go_angle < (danger_ang + i * 90 + 10)){
          motor_speed -= 20;
          break;
        }
      }
    }

    int use_far = 40;
    if(read_LINE == 1 && side_getBall_flag == 0){ //ライン読んでた　過去
      float vx, vy, dissBall;
      dissBall = simplify->goPM(abs(Ball_angle));

      if((abs(dissBall) < 15) && (abs(simplify->goPM(Last_Line_angle) < 30)) && (isCam_read == 0)){ //LEGEND再現
        vx = cos(radians(simplify->RoboToMath(Ball_angle)));
        vy = 0; //y成分を消す

        motor_speed = 150 * abs(vx);
        go_angle = simplify->MathToRobo(degrees(atan2(vy, vx)));
        read_LINE = 1;
        pid_angle = bnopid->get_pd(simplify->goPM(Ball_angle));
        locked_pid = Ball_angle;
      }
      else if((((60 < Last_Line_angle && Last_Line_angle < 120) || (240 < Last_Line_angle && Last_Line_angle < 300)) && abs(simplify->goPM(Ball_angle)) < 90) && isCam_read == 1){ //LEGEND再現
        if(simplify->goPM(Last_Line_angle) > 0) vx = 0.2;
        else vx = -0.2;
        vy = sin(radians(simplify->RoboToMath(Ball_angle)));

        go_angle = simplify->MathToRobo(degrees(atan2(vy, vx)));
        motor_speed = 300 * abs(vy);
        if(motor_speed > 180) motor_speed = 180;
        read_LINE = 1;
        pid_angle = bnopid->get_pd(simplify->goPM(Ball_angle));
        if(isHold == 1) active_Kick = 1;
      }
      else{
        read_LINE = 0;
        Bangle_LEGEND = 0;
      }
    }
    go_angle = simplify->FixLimit(go_angle);
    if(isHold == 0){
      motor_speed -= 40;
    }
    else{
      motor_speed -= 50;
    }
    if(motor_speed < 0) motor_speed = 0;
    motor->calc(float(go_angle), pid_angle, motor_speed, int(isCatch_Ball_pid), cam_angle);
  }

  if(active_Kick == 1){
    active_Kick = kick->shoot();
  }

  Last_pid_angle = pid_angle;

  if(static_state != B){
    static_state = B;
    push_front3(static_state);

    if(static_state_arr[0] != 20 && static_state_arr[1] == 20 && static_state_arr[2] != 20){ //違う、ライン、違う
      if(startLine_timer != 1){
        timer_Linecheck.reset();
        startLine_timer = 1;
      }
      else{
        if(timer_Linecheck.read_ms() > 2000){
          startLine_timer = 0;
          readLine_count = 0;
        }
      }
    }
    else if(startLine_timer == 1 && static_state_arr[0] == 20 && static_state_arr[1] != 20 && static_state_arr[2] == 20){ //ライン、違う、ライン、連続を検知
      if(readLine_count < 3) readLine_count++; //3回検知するまで入らない
      else{
        sideLine_trase_flag = 1;
        startLine_timer = 0;   // 再検出防止
        readLine_count = 0;
      }
    }
    else{
      startLine_timer = 0;
      readLine_count = 0;
      sideLine_trase_flag = 0;
    }
  }
  Last_Ball_angle = Ball_angle;
}


