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
  for(int i = 0; i < 2; i++){
    Line_vec[i] = 0;
    Line_side_vec[i] = 0;
  }
  vchange_on = 1;
  motor_flag = 0;
  isHold = 0;
  now_ = 0;
  isCatch_Ball_pid = 0;

  //センサーの値を更新
  Ball_angle = ball->getAngle();
  get_Ball_far = moveave_far.add(ball->getFar());
  isCam_read = camera->getFrontread();
  cam_angle = camera->getFrontAngle();
  isHold = ball->isCaught();
  byte_height = camera->getFrontheight();
  // Line_angle = line_test->angle(Line_vec, Line_side_vec); //sincos
  bool isLineNew = line_test->isRead_Line();
  // OMNICAM_center = camera->getOMNICAMcenter();
  // OMNICAM_flont = camera->getFrontOMNICAMAngle();
  OMNICAM_opposite = camera->getOppositeOMNICAMAngle();
  float OMNICAM_flont_wDIR = simplify->goPM(OMNICAM_flont) + bnopid->getDIR();
  cam_height = camera->getFrontheight();
  NOW_DIR = bnopid->getDIR();
  camm_width = camera->getFrontwidth();
  cam_dot = camera->getFrontdot();
  // Serial.println(isHold)
  if(analogRead(A8) < 400){
    active_Kick = 1;
    // isHold = 1;
  }

  if(isLineNew){
    Line_angle = line_test->angle(Line_vec, Line_side_vec); //sincos
    default_Line_angle = Line_angle;
    if(Line_angle != 999){
      isLine_read = 1;
    //   if(line_test->isRead_Angel() != 0){
    //     Line_angle = simplify->FixLimit(simplify->goPM(Line_angle) + NOW_DIR);
    //   }
    //   else{
    //     if(Line_angle == 0){
    //       Line_angle = 0;
    //     }
    //     else{
    //       Line_angle = simplify->FixLimit(simplify->goPM(Line_angle) + NOW_DIR);
    //     }
    //   }
    }
    else{
      isLine_read = 0;
    }
    Line_depth = line_test->getDepth();
    // if(Line_depth < 0.5) Line_angle = simplify->FixLimit(OMNICAM_center+180);
  }
  // Line_angle = 999;
  // Serial.println(Line_angle);
  // Serial.println(OMNICAM_center);

  if(A == 5){ //ボールなし処理
    B = A;

    if(Ball_angle != 999) A = 10;
    if(Line_angle != 999) A = 20;

    if(B == A){
      // if(timer_goCenter.read_ms() > 1500){
      //   go_angle = simplify->goMP(OMNICAM_center); 
      //   motor_speed = 70;
      // }
      // else{
        go_angle = 0;
        motor_speed = 0;
      // }
      pid_angle = bnopid->get_pd(0);
      motor_flag = 1;
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
      //タイマー起動 これは補足せんさ安定時
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

      // --- ここから中立制御 ＆ 姿勢制御マージ ---
      bool isthis = 0;
      float use_cam = cam_angle + NOW_DIR;
      float use_ball = simplify->goPM(Ball_angle) + NOW_DIR;
      if((cam_angle >= 0 && use_ball >= 0) || (cam_angle < 0 && use_ball < 0)) isthis = 1; 

      // 1. ボール角度のスムージング処理
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

      // 2. ターゲット姿勢の計算 (Attackクラス用に bnopid->getDIR() を使用)
      float rawAttitude = bnopid->getDIR() + currentGoPM;
      
      // 3. アダプティブフィルタ関数のインライン展開 (smoothAttitude の中身)
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

      // 4. 正面指定角度（目標10度以内など）の判定と反映
      float useAttitude = 0.0f;
      if(fabsf(currentGoPM) < 25.0f){ // ボールが正面付近(25度未満)ならその方向へ姿勢制御
        useAttitude = setAttitude;
      }

      // PID操作量の確定
      // if(NOW_DIR > -30 && NOW_DIR < 30){
        // pid_angle = bnopid->get_pd(useAttitude);
      // }
      // else{
        pid_angle = bnopid->get_pd(0);
      // }


      // カメラがゴールを捉えている場合の優先上書き
      if(abs(abs(use_ball)-abs(cam_angle)) < 35 && get_Ball_far > 30 && abs(use_cam) < 30){
        if(isCam_read == 1 && isthis == 1){
          pid_angle = bnopid->get_pd(use_cam);
          isCatch_Ball_pid = 0;
        }
      }
      motor_speed = 200;
      // if(motor_speed != 0) motor_speed = constrain(motor_speed, MIN_SPEED, MAX_SPEED);
    }
  }
  
  // float use_ball = simplify->goPM(Ball_angle) + NOW_DIR;
  // Serial.print(Ball_angle);
  // Serial.print(" ");
  // Serial.print(NOW_DIR);
  // Serial.print(" ");
  // Serial.print(use_ball);
  // Serial.println(" ");

  bool lineIsSide =
  (45  <= Line_angle && Line_angle < 135) ||
  (225 <= Line_angle && Line_angle < 315);
      // --- 前後移動方向 ---
  if(A == 20){ //ライン処理
    if(B != A){
      B = A;
      kotei_pid = Last_pid_angle;
    }

    float go_arr[2] = {0, 0};
    float ballPM = simplify->goPM(Ball_angle);
    float linePM = simplify->goPM(Line_angle);

    bool isSame_side_ball =
        ((ballPM < 0 && linePM < 0) || (ballPM >= 0 && linePM >= 0));
    bool isSame_side_cam =
        ((OMNICAM_flont_wDIR <= 0 && linePM >= 0) || (OMNICAM_flont_wDIR > 0 && linePM < 0));

    // sideLine_trase_flag = 0;
    if(sideLine_trase_flag == 1){
      push_flag = 0;
 
      if(abs(ballPM) < 70){
        go_arr[1] = 1;
      } 
      else{
        go_arr[1] = -1;
      }

      lineIsSide = 0;
      // --- ライン追従判定 ---
      if(lineIsSide == 1){
        float lineCos = cos(radians(simplify->RoboToMath(Line_angle))) / (-1.5*Line_depth + 2.5);
        if(Line_depth < 0.6){
          if(Line_depth < 0.2) sideLine_trase_flag = 0;
          go_arr[0] = -lineCos;
        }
        else{
          go_arr[0] = 0;
        }
      }
      else if((abs(simplify->goPM(Line_angle)) < 15) && isCam_read == 1 && abs(simplify->goPM(Ball_angle)) < 30){
        A = 40;
      }
      else{
        push_flag = 0;
        sideLine_trase_flag = 0;
      }
    }

    if(Line_angle == 999){
      A = 10;
      float dissLINE;
      dissLINE = abs(Last_Line_angle - Bangle_LEGEND); //前回ラインと前回ボールの差分
      if(70 > dissLINE || dissLINE > 290){
        read_LINE = 1;
      }
    }
    else{ //ライン読んでる
      if(read_LINE == 1){
        read_LINE = 0;
        Bangle_LEGEND = 0;
      }
      Last_Line_angle = Line_angle;
    }

    if(B == A){
      if(sideLine_trase_flag == 1){
        go_angle = simplify->MathToRobo(degrees(atan2(go_arr[1], go_arr[0])));
        if(get_Ball_far < 30){
          motor_speed = 140;
        }
        else{
          motor_speed = 140;
        }
        if((30 < abs(simplify->goPM(Ball_angle)) && abs(simplify->goPM(Ball_angle)) < 90) && isSame_side_ball == 0){
          go_angle = simplify->goMP(OMNICAM_center); 
          motor_speed = 240;
        }
        // if(isHold > 0.1){
        //   active_Kick = 1;
        // }
      }
      else{
        go_angle = simplify->FixLimit(Line_angle+180);
        motor_speed = 210;
      }
      pid_angle = bnopid->get_pd(0);
      motor_flag = 1;
      vchange_on = 0;
      Bangle_LEGEND = Ball_angle;
    }
    isCatch_Ball_pid = 0;
  }

  // if(cam_height < 18){
  //   kick_step = 2; //遠い時はハーフラインまで引っ張る
  // }
  // else if(cam_height >= 18){
  //   kick_step = 1;
  // }
  // else if(isCam_read == 0){
  //   kick_step = 0;
  // }

  if(A == 30){ //持ってる時処理
    KICK_DISS = 10;
    // Serial.println(timer_catch.read_ms());
    if(B != A){ //enterstate
      B = A;
      if(abs(cam_angle) < KICK_DISS) isdiss_in = 1;
      else isdiss_in = 0;
      timer_catch.reset();
    }
    uart_catch = 1;
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
    if(abs(simplify->goPM(Ball_angle)) > 20) A = 10; 
    if(Line_angle != 999) A = 20;

    if(B == A) kick_step = 2;
    else{
      kick_delay_flag = 0;
      isdiss_in = 0;
    }

    int kick_timer_threshold = 0;
    if(isdiss_in == 1 && timer_catch.read_ms() > 90) kick_timer_threshold = 1;
    else if(isdiss_in == 0 && timer_catch.read_ms() > 90) kick_timer_threshold = 1;

    int kick_threshold = 0;
    if(abs(cam_angle) < KICK_DISS) kick_threshold = 1;
    else kick_threshold = 0;

    if(isdiss_in == 1) kick_step = 10; //異常

    if(isHold == 1 && kick_timer_threshold == 1 && kick_threshold == 1 && kick_delay_flag == 0){
      kick_delay_flag = 1;
      timer_kick_delay.reset();
      kick_step = 3;

      if(timer_catch.read_ms() < 100){
        kick_delay = 200;
        kick_step = 20;
      }
    }

    if(kick_delay_flag == 1 && (timer_kick_delay.read_ms() > kick_delay+100)){
      if(kick_threshold == 1){
        active_Kick = 1;
      }
      kick_step = 4;
    }
    
    if(B == A){      
      float thisthis = 0;
      if(timer_catch.read_ms() > 90){ //的避け
        if(NOW_DIR < 0) thisthis = cam_dot+camm_width/4;
        else thisthis = cam_dot-camm_width/4;
        cam_angle = -1 * (float)(thisthis - CENTER_VAL) * (HFOV / 250.0);
      }
      go_angle = 0;
      pid_angle = bnopid->get_pd(cam_angle + NOW_DIR);
      // if(cam_angle > 15) go_angle = 8;
      // else if(cam_angle < -15) go_angle = -8;

      isCatch_Ball_pid = 1;
      vchange_on = 0;
      motor_flag = 1;
    }   
    motor_speed = 200;
  }
  else{
    uart_catch = 0;
  }

  // Serial.print(cam_angle);
  // Serial.print(" ");
  // Serial.println(isCam_read);
  // if(A == 40) //押し込み判断
  // { 
  //   B = A;
  //   if(line_test->isRead_Angel() == 1){
  //     A = 45;
  //     timer_pushIn.reset();
  //   }
  //   else{
  //     go_angle = 0;
  //     motor_speed = 100;
  //   }

  //   if(B == A){
  //     pid_angle = bnopid->get_pd(0);
  //     motor->calc(float(go_angle), pid_angle, int(motor_speed), int(isCatch_Ball_pid), cam_angle);
  //   }
  // }
  // Serial.println(Line_depth);

  if(A == 40) //押し込み
  {
    B = A;
    if(Line_depth < -0.8){
      A = 48;
    }
    else{
      go_angle = Ball_angle;
      motor_speed = 70;
    } 

    // if(abs(simplify->goPM(Ball_angle)) > 20) A = 48; 

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

  if(A == 999){
  }

  // Serial.print(go_angle);
  // Serial.print(" ,");
  // Serial.print(pid_angle);
  // Serial.print(" ,");
  // Serial.print(motor_speed);
  // Serial.println();
  if(motor_flag == 1)
  {
    if(B != 5){
      get_Ball_far = constrain(get_Ball_far, 0, 50); 
      // if(B == 10){
      //   motor_speed = (get_Ball_far * 0.5) + 185;
      // }

      int danger_ang = 45;
      for(int i = 0; i < 4; i++){
        if((danger_ang + i * 90 - 10) <= go_angle && go_angle < (danger_ang + i * 90 + 10)){
          motor_speed -= 20;
          break;
        }
      }
      // if(motor_speed != 0) motor_speed = constrain(motor_speed, MIN_SPEED, MAX_SPEED);
    }

    // if(B == 30){
    //   motor_speed = 230;
    // }

    int use_far = 40;
    if(read_LINE == 1){ //ライン読んでた　過去
      float vx, vy, dissBall;
      dissBall = simplify->goPM(abs(Bangle_LEGEND - Ball_angle));
      
      if((abs(dissBall) < 30) && (330 < Last_Line_angle || Last_Line_angle < 30) && (get_Ball_far > use_far) && (isCam_read == 0)){ //LEGEND再現
        vx = cos(radians(simplify->RoboToMath(Ball_angle)));
        vy = 0; //y成分を消す

        motor_speed = 100 * abs(vx);
        go_angle = simplify->MathToRobo(degrees(atan2(vy, vx)));
        read_LINE = 1;
        pid_angle = bnopid->get_pd(0);
      }
      else if((abs(dissBall) < 30) && ((60 < Last_Line_angle && Last_Line_angle < 210) || (240 < Last_Line_angle && Last_Line_angle < 300)) && (isCam_read == 0) && (get_Ball_far > use_far)){
        vx = 0; //x成分を消す
        vy = sin(radians(simplify->RoboToMath(Ball_angle)));
        motor_speed = 100 * abs(vy);

        go_angle = simplify->MathToRobo(degrees(atan2(vy, vx)));

        read_LINE = 1;
        pid_angle = bnopid->get_pd(0);
      }
      else{
        read_LINE = 0;
        Bangle_LEGEND = 0;
        //ここでボールタッチしたい
      }
    }

    // Serial.print(go_angle);
    // Serial.print("  ");
    // if(isCam_read == 0){
    //   pid_angle = OMNICAM_angle_flont;
    // }
    // motor.calc(float(go_angle), pid_angle, ((getBatteryVoltage() > float(11.5)) ? int(getCompensatedPWM(motor_speed)) : int(motor_speed)), isCatch_Ball);
    if(abs(simplify->goPM(go_angle)) < 5) go_angle = 0;
    motor->calc(float(go_angle), pid_angle, int(motor_speed), int(isCatch_Ball_pid), cam_angle);
  }

  if(active_Kick == 1){
    active_Kick = kick->shoot();
  }

  Last_pid_angle = pid_angle;

  if(static_state != B){
    static_state = B;
    push_front3(static_state);
    /* ===== タイマー開始：値,20,値 ===== */
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
  // Serial.print(static_state);
  // Serial.print("  ");
  // Serial.print(readLine_count);
  // Serial.print("  ");
  // Serial.print(timer_Linecheck.read_ms()); 
  // Serial.println();
}

// void Attack::runSimple(int speed) {
//   // 単純に前に進むだけの実験用メソッド
//   // go_angle = 0 で前進、pid_angle = 0 で方位維持なし
//   motor->calc(0.0, 0.0, speed, 0, 0.0);
// }

