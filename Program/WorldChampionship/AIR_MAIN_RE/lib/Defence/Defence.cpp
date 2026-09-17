//世界大会総合優勝！！！！
#include <Arduino.h>
#include "Defence.h"

void Defence::begin(){
  A = 0;
  B = 999;
  L = 1;
  moveave_ball_far.setup(40);
  moveave_dash.setup(40);
  // moveave_dash_near.setup(40);
}

void Defence::attach(BNOPID& bno, MOTOR& mot, KICK& k, BALL& b, LINE_TEST& lt, SIMPLIFY& s, Cam& cam)
{
  bnopid = &bno;
  motor  = &mot;
  kick   = &k;
  ball = &b;
  line_test = &lt;
  simplify = &s;
  camera = &cam;
}

void Defence::run(){

  if(!ball || !camera || !line_test || !bnopid || !motor || !kick || !simplify){
    return; // 未接続なら何もしない
  }

  for(int i = 0; i < 2; i++){
    Line_vec[i] = 0;
    Line_side_vec[i] = 0;
  }

  get_Ball_angle = ball->getAngle();
  get_Ball_far_ave = moveave_ball_far.add(ball->getFar());
  cam_flag = camera->getCamFlag();
  return_flag = camera->getReturnFlag();
  Back_cam_angle = camera->getBackAngle(); 
  uart_catch = ball->isCaught();
  flont_cam_angle = camera->getFrontAngle();
  isCam_read = camera->getFrontread();
  OMNICAM_angle_flont = camera->getFrontOMNICAMAngle();
  OMNICAM_opposite = camera->getOppositeOMNICAMAngle();
  Line_angle = line_test->angle(Line_vec, Line_side_vec); //sincos
  camm_width = camera->getFrontwidth();
  cam_dot = camera->getFrontdot();

  if(OMNICAM_opposite == 999) OMNICAM_opposite = OMNICAM_opposite_L;
  else OMNICAM_opposite_L = OMNICAM_opposite;
  
  if(OMNICAM_angle_flont == 999) OMNICAM_angle_flont = OMNICAM_angle_flont_L;
  else  OMNICAM_angle_flont_L = OMNICAM_angle_flont;

  // int cam_ball_read     = camera->getBallRead();     // 1: 見えている, 0: ロスト
  // float cam_ball_angle  = camera->getBallAngle();    // 正面を0度とした左右の角度誤差
  // float cam_ball_dist   = camera->getBallDistance(); // カメラからの生距離（0〜250）

  // Serial.print("cam : ");
  // Serial.print(flont_cam_angle);
  // Serial.print("cam_ball_read : ");
  // Serial.print(cam_ball_read);
  // Serial.print(" | Angle : ");
  // Serial.print(cam_ball_angle);
  // Serial.print(" | Distance : ");
  // Serial.println(cam_ball_dist);


  int Lost_cam_flag = 0;


  if(Lost_cam_flag == 1){
    cam_flag = 1;
    isCam_read = 1;
    Back_cam_angle = 180;
    flont_cam_angle = 45;
    return_flag = 0;
  }

  Ball_angle = simplify->goPM(get_Ball_angle);
  float Back_cam_angle_2 = simplify->goPM(Back_cam_angle);
  // if(Back_cam_angle_2 < 0) flont_cam_angle = 45;
  // else flont_cam_angle = 315;
  
  // int Back_ = 0;
  // // int Left_ = 0;
  // // int Right_ = 0;
  // if(side_vals[2] ==1){
  //   Back_ = 1;
  // }
  // if(side_vals[1] == 1){
  //   Right_ = 1;
  // }
  // if(side_vals[3] == 1){
  //   Left_ = 1;
  // }

  if(cam_flag == 1){
    timer_no_cam.reset();
  }
  // if(return_flag == 1){
  //   if(abs(OMNICAM_opposite) < 100 && Line_angle != 999){
  //     return_flag = 0;
  //   }
  // }

  if(uart_catch == 0){
    timer_hold.reset();
  }
  float dash_judge_angle_1 = 0;
  float dash_judge_angle_2 = 0;
  if(Back_cam_angle_2 < 0){
    dash_judge_angle_1 = 50;
    dash_judge_angle_2 = 325;
  }
  else{
    dash_judge_angle_1 = 35;
    dash_judge_angle_2 = 315;
  }
  if((get_Ball_angle <= dash_judge_angle_1 || dash_judge_angle_2 <= get_Ball_angle) && 30 <= get_Ball_far_ave){ 
    dash_judge = 1;
  }
  else{
    dash_judge = 0; 
  }
  dash_judge_ave = moveave_dash.add(dash_judge);

  if(0.3 <= dash_judge_ave && cam_flag == 1){
    dash_flag = 1;
  }
  else{
    dash_flag = 0;
  }

  // float dash_judge_angle_near_1 = 0;
  // float dash_judge_angle_near_2 = 0;
  // if(Back_cam_angle_2 < 0){
  //   dash_judge_angle_near_1 = 20;
  //   dash_judge_angle_near_2 = 355;
  // }
  // else{
  //   dash_judge_angle_near_1 = 5;
  //   dash_judge_angle_near_2 = 340;
  // }
  // if((get_Ball_angle <= dash_judge_angle_near_1 || dash_judge_angle_near_2 <= get_Ball_angle) && 40 <= get_Ball_far_ave){
  //   dash_judge_near = 1;
  // }
  // else{
  //   dash_judge_near = 0;
  // }
  // dash_judge_naer_ave = moveave_dash.add(dash_judge_near);

  // if(0.8 <= dash_judge_naer_ave && cam_flag == 1){
  //   dash_flag_near = 1;
  // }
  // else{
  //   dash_flag_near = 0;
  // }
  

  // if(get_Ball_far_ave <= 40 && abs(Ball_angle) < 20 && Back_cam_angle_2 * Ball_angle < 0){
  //   dash_flag_2 = 1;
  // }

  if(A == 0) //モーターを止める
  {
    B = A;
    if(Line_angle != 999){ 
      A = 110;
    }
    else{
      A = 250;
    }

    if(B == A){
      move_angle = 0;
      pid_angle = bnopid->get_pd(0);
      motor_speed = 0;
      motor_flag = 0;
    }
  }

  /*--------------------------------------------------------------------------------------------------------------------------*/

  if(A == 10) //ボールが前にある時
  {
    if(B != A){ //初めてここにはいったとき
      timer_dash.reset();
    }

    B = A;

    if(dash_flag == 0 || get_Ball_angle == 999){
      A = 110;
    }
    if(Line_angle == 999){
      A = 199;
    }
    if(100 <= timer_dash.read_ms()){
      // dash_flag_near = 0;
      A = 11;
    }

    if(B == A){
      move_angle = 0;
      pid_angle = bnopid->get_pd(0);
      motor_speed = 0;
      motor_flag = 1;
    }
  }

  if(A == 11) //ボールを追いかける時
  {
    // float dash_time = 0;
    if(B != A){
      timer_dash.reset();
      // timer_dash_near.reset();
      // if(dash_flag_near == 1) dash_flag_keep = 0;
      // else dash_flag_keep = 1;
    }

    B = A;

    
    // if(dash_flag_keep == 1){
    //   dash_time = 400;
    // }
    // else{
    //   dash_time = 250;
    // }

    // if(dash_time <= timer_dash.read_ms()){
    //   if(uart_catch == 1 && Lost_cam_flag == 0){
    //     A = 12;
    //   }
    //   else if(Lost_cam_flag == 1){
    //     A = 199;
    //   }
    //   else{
    //     A = 250;
    //   }
    // }

    if(400 <= timer_dash.read_ms()){
      // uart_catch = 1;
      if(uart_catch == 1 && Lost_cam_flag == 0){
        A = 12;
      }
      else if(Lost_cam_flag == 1){
        A = 199;
      }
      else{
        A = 250;
      }
    }

    if(B == A){
      // if(dash_flag_keep == 0 && uart_catch ==  1 && active_Kick == 0){
      //   active_Kick = 1;
      // }
      move_angle = simplify->FixLimit(Ball_angle * 1.5);
      // if(dash_flag_keep == 0) move_angle = get_Ball_angle;
      // if(Last_dash_flag_2 == 1) move_angle = get_Ball_angle;
      // if(uart_catch == 1){
      //   if(flont_cam_angle > 20) move_angle = 30;
      //   else if(flont_cam_angle < -20) move_angle = -30;
      // }
      motor_speed = 250;
      pid_angle = bnopid->get_pd(0);
      // if(uart_catch == 1) pid_angle = bnopid->get_pd(flont_cam_angle + bnopid->getDIR());
      motor_flag = 1;
    }
  }

  if(A == 12) //ボールを補足して前に出る時
  {
    if(B != A){
      timer_dash.reset();
    }

    B = A;
    
    if(440 < timer_dash.read_ms() && active_Kick == 0){
      active_Kick = 1;
    }
    if(500 <= timer_dash.read_ms()){
      if(uart_catch == 0){
        A = 250;
      }
    }

    // if(uart_catch == 0){
    //   A = 250;
    // }

    if(B == A){
      // if(dash_flag_keep == 0 && active_Kick == 0) active_Kick = 1;
      // if(440 < timer_dash.read_ms() && dash_flag_keep == 1 && active_Kick == 0) active_Kick = 1;
      float thisthis = 0;
      if(bnopid->getDIR() < 0) thisthis = cam_dot+camm_width/2;
      else thisthis = cam_dot-camm_width/2;
      flont_cam_angle = -1 * (float)(thisthis - CENTER_VAL) * (HFOV / 250.0);
      move_angle = 0;
      // if(isCam_read == 1){
      //   if(flont_cam_angle > 15) move_angle = 20;
      //   else if(flont_cam_angle < -15) move_angle = -20;
      // }
      if(flont_cam_angle > 15) move_angle = 20;
      else if(flont_cam_angle < -15) move_angle = -20;
      pid_angle = bnopid->get_pd(flont_cam_angle + bnopid->getDIR());
      // if(isCam_read == 0){
      //   if(Back_cam_angle_2 < 0) pid_angle = bnopid->get_pd(-45);
      //   else pid_angle = bnopid->get_pd(45);
      // }
      // pid_angle = bnopid->get_pd(0);
      motor_speed = 250;
      motor_flag = 1;
    }
  }

  /*--------------------------------------------------------------------------------------------------------------------------*/

  if(A == 110) //ライントレース
  {
    double n_x = 0;
    double n_y = 0;
    double go_x = 0;
    double go_y = 0;
    float border_angle[2];
    float theta = 0;
    int go_flag = 0;
    float move_angle_W;
    float x_up = 0;
    float y_up = 0;
    float g = 1.0;
    // int neutral_point_flag = 0;

    B = A;
    
    if(return_flag == 1){
      A = 250;
    }
    else if(Line_angle == 999){
      A = 199;
    }
    // else if(dash_flag_near == 1){
    //   if(Last_dash_flag_near == 0 || uart_catch == 1){
    //     timer_dash_near.reset();
    //   }
    //   if(200 <= timer_dash_near.read_ms()){
    //     A = 11;
    //   }
    // }
    else if(dash_flag == 1){ 
      if(Last_dash_flag == 0 || uart_catch == 1){
        timer_dash.reset();
      }
      if(2000 <= timer_dash.read_ms()){
        A = 10;
      }
    }
    
    if(B == A){
      theta = simplify->goPM(Line_angle);
      // theta = OMNICAM_opposite;

      float theta_2 = abs(simplify->goPM(Line_angle));
      if(90 <= theta_2){
        theta_2 = 180 - theta_2;
      }
      if(L == 1){
        if(theta_2 > 45) L = 3;       
        else if(theta_2 > 20) L = 2;  // （20度以上傾かないとL=2にならない）
      } 
      else if(L == 2){
        if(theta_2 <= 15) L = 1;      // （15度まで姿勢が戻ればすぐL=1に復帰）
        else if(theta_2 > 45) L = 3;  // （L=2からL=3へのしきい値も少し上げて余裕を持たせた）
      } 
      else if(L == 3){
        if(theta_2 <= 15) L = 1;      // （直行条件も15に合わせる）
        else if(theta_2 <= 42) L = 2; // 45度でL=3に入っても、42度になればすぐL=2へ復帰）
      }
      else{
        L = 1; // 例外処理（初期化）
      }
      
      // if(L == 2 || L == 3){
      //   theta = OMNICAM_opposite;
      // }
      
      if(theta < 0){  //角度を-π~πに収めるための場合分け
        border_angle[0] = theta;
        border_angle[1] = theta + 180;
      }
      else{
        border_angle[0] = theta - 180;
        border_angle[1] = theta;
      }

      if(border_angle[0] < Ball_angle && Ball_angle < border_angle[1]){  //ボールの角度を区分けする
        go_flag = 0;
      }
      else{
        go_flag = 1;
      }
      move_angle_W = border_angle[go_flag] + 90;  //進む角度決定
      move_angle_W = simplify->FixLimit(move_angle_W);

      if((170 < move_angle_W && move_angle_W < 190)){ //これあるとカクカクするけど、端の安定感はます。可能性を潰さないために一旦消す
        move_angle_W = 0;
      }

      if(L == 1){
        g = 0.2;
        if(cam_flag == 0){ //110ピッタでライン上、保険で120が良いかも
          y_up = 0.5;
          if(Back_cam_angle_2 < 0){
            move_angle_W = 270;
          }
          else{
            move_angle_W = 90;
          }
          n_x = cos(radians(simplify->RoboToMath(move_angle_W)));
          n_y = sin(radians(simplify->RoboToMath(move_angle_W)));
        }
        else if(500 <= timer_hold.read_ms()){
          g = 1.9;
          move_angle_W = 0;
          n_x = cos(radians(simplify->RoboToMath(move_angle_W)));
          n_y = sin(radians(simplify->RoboToMath(move_angle_W)));
        }
        else{
          n_x = cos(radians(simplify->RoboToMath(move_angle_W)));
          n_y = 0;
        }
      }
      else if(L == 2){
        g = 2.3;
        if(cam_flag == 0) move_angle_W = 0;
        n_x = cos(radians(simplify->RoboToMath(move_angle_W)));
        n_y = sin(radians(simplify->RoboToMath(move_angle_W))); 
      }
      else if(L == 3){
        if(get_Ball_angle == 999 && cam_flag == 0){
          move_angle_W = 0;
        }
        g = 1.2;
        n_x = 0;
        n_y = sin(radians(simplify->RoboToMath(move_angle_W))); 
        if(0 <= Back_cam_angle_2){ //これないと端で壁に当たる
          x_up = -0.5;
        }   
        else{
          x_up = 0.5;   
        }
        // if((timer_no_cam.read_ms() >= 1500) && (abs(OMNICAM_opposite) > 120)){
        //   x_up *= -1;
        // }
         
      }
      else{
        g = 1;
        n_x = cos(radians(simplify->RoboToMath(move_angle_W)));   //ラインと並行移動するベクトル決定
        n_y = sin(radians(simplify->RoboToMath(move_angle_W))); 
      }

      go_x = n_x + (Line_vec[1] + x_up)  * g;   //ラインと並行移動するベクトルと白線に留まるベクトルを合成
      go_y = n_y + (Line_vec[0] + y_up) * g;
      move_angle = degrees(atan2(go_y,go_x));
      move_angle = simplify->MathToRobo(move_angle);

      // 1. ボールが後ろ半分（90度〜180度）にある時
      // if(abs(Ball_angle) > 90){
      //   // 真後ろ(180度)に近いほど遅く(150)、横(90度)に近いほど速く(250)なるように計算
      //   motor_speed = 150 + ((180.0 - abs(Ball_angle)) / 90.0) * 100;
        
      //   // motor_speed = 150; 
      // }
      // // 2. ボールが横（25度〜90度）、または指定距離の時（従来通りフルスピード）
      // else if(abs(Ball_angle) > 25 || (10 < get_Ball_far_ave && get_Ball_far_ave < 25)){
      //   motor_speed = 250;
      // }
      // // 3. ボールが前（0度〜25度）にある時減速
      // else{
      //   motor_speed = 150 + (abs(Ball_angle) / 25.0) * 100;
      // }

      if(abs(Ball_angle) > 25 || (10 < get_Ball_far_ave && get_Ball_far_ave < 25)){
        motor_speed = 250;
      }
      else{
        motor_speed = 150 + (abs(Ball_angle) / 25.0) * 100;
      }
      
      if(uart_catch == 1 && timer_hold.read_ms() < 200){
        motor_speed = 0;
      }
      else if(500 <= timer_hold.read_ms()){
        motor_speed = 250;
        if(600 <= timer_hold.read_ms()){
          active_Kick = 1;
        }
      }
      else if(dash_flag == 1){
        if(2000 < timer_dash.read_ms()){
          motor_speed = 150;
        }
      }
      else if(L == 1 && cam_flag == 0){
        motor_speed = 200;
      }
      else if(get_Ball_angle == 999){
        if(L == 3){
          motor_speed = 100;
        }
        else{
          motor_speed = 0;
        }
        // neutral_point_flag = 1;
      }
      else if(L == 1 && 160 <= abs(Ball_angle)){
        motor_speed = 0; 
      }
      else if(L == 2){
        if(175 < motor_speed){
          if(90 < abs(simplify->goPM(move_angle))){
            motor_speed = 150;
          }
          else{
            motor_speed = 175;
          }
        }
      }
      else if(L == 3){
        if(150 < motor_speed){
          if(90 < abs(simplify->goPM(move_angle))){
            motor_speed = 125;
          }
          else{
            motor_speed = 150;
          }
        }
      }
    }

    // if(motor_speed == 0){
    //   // 前後移動成分(n_x, n_y)を消し、ラインキープ成分のみでベクトルを再合成
    //   go_x = (-Line_vec[1] + x_up) * g;
    //   go_y = (-Line_vec[0] + y_up) * g;
      
    //   // ベクトルの大きさ（目標位置からのズレ具合）を計算
    //   float offset_mag = sqrt(go_x * go_x + go_y * go_y);
    //   float offset_g = 0;

    //   if(get_Ball_angle == 999){
    //     offset_g = 0.02;
    //   }
    //   else{
    //     offset_g = 0.25;
    //   }
      
    //   // ズレが閾値より大きければ、位置調整のためにゆっくり動く
    //   if(offset_mag > offset_g){ 
    //     move_angle = degrees(atan2(go_y, go_x));
    //     move_angle = simplify->MathToRobo(move_angle);
    //     if(simplify->FixLimit(move_angle - 10) <= get_Ball_angle && get_Ball_angle <= simplify->FixLimit(move_angle + 10)){
    //       motor_speed = 0;
    //     }
    //     else{
    //        motor_speed = 100;  // 位置調整用の弱めのスピード
    //     }
    //   }
    // }

    // if(neutral_point_flag == 1){
    //   // 前後移動成分(n_x, n_y)を消し、ラインキープ成分のみでベクトルを再合成
    //   go_x = (-Line_vec[1] + x_up) * g;
    //   go_y = (-Line_vec[0] + y_up) * g;
      
    //   move_angle = degrees(atan2(go_y, go_x));
    //   move_angle = simplify->MathToRobo(move_angle);
    //   if(simplify->FixLimit(move_angle - 10) <= get_Ball_angle && get_Ball_angle <= simplify->FixLimit(move_angle + 10)){
    //     motor_speed = 0;
    //   }
    //   else{
    //     motor_speed = 100;  // 位置調整用の弱めのスピード
    //   }
    // }

    pid_angle = bnopid->get_pd(0);
    if(uart_catch == 1){
      if(0 <= Back_cam_angle_2){
        pid_angle = bnopid->get_pd(45);
      }
      else{
        pid_angle = bnopid->get_pd(-45);
      }
      
    }
    motor_flag = 1;
  }

  /*--------------------------------------------------------------------------------------------------------------------------*/

  if(A == 199) //ラインに戻る
  {
    B = A;

    if(return_flag == 1){
      A = 250;
    }
    else if(Line_angle != 999){
      A = 110;
    }

    if(B == A){
      move_angle = Last_Line_angle;
      pid_angle = bnopid->get_pd(0);
      motor_speed = 200;
      motor_flag = 1;
    }
  }
  
  /*--------------------------------------------------------------------------------------------------------------------------*/

  // if(A == 200){
  //   B = A;

  //   if(cam_flag == 1){
  //     A = 250;
  //   }

  //   if(B == A){
  //     move_angle = 0;
  //     pid_angle = bnopid->get_pd(0);
  //     motor_speed = 230;
  //     motor_flag = 1;
  //   }
  // }

  /*--------------------------------------------------------------------------------------------------------------------------*/

  if(A == 250){ //ゴールに戻る時
    if(B != A && B == 110){
      timer_return.reset();
    }
    B = A;

    if(Line_angle != 999 && 500 <= timer_return.read_ms()){
      if(return_flag == 0){
        A = 299;
      }
    } 

    if(B ==A){
      // if(cam_flag == 0 && OMNICAM_opposite != 999){
      //   move_angle = OMNICAM_opposite;
      // }
      // else{
      //   move_angle = Back_cam_angle;
      // }
      move_angle = Back_cam_angle;

      if((get_Ball_angle - 30) <= move_angle && move_angle <= (get_Ball_angle + 30)){
        move_angle = simplify->FixLimit(Ball_angle * 1.2);
      }
      // move_angle = Back_cam_angle;
      pid_angle = bnopid->get_pd(0);
      motor_speed = 230;
      motor_flag = 1;
    }
  }

  if(A == 299){ //ゴールに戻った時速度ベクトルを殺
    if(B != A){
      timer_dash_kill.reset();
    }

    B = A;

    if(timer_dash_kill.read_ms() > 100){
      A = 110;
    }
    
    if(B ==A){
      move_angle = 0;
      pid_angle = bnopid->get_pd(0);
      motor_speed = 200;
      motor_flag = 1;
    }
  }

  /*--------------------------------------------------------------------------------------------------------------------------*/

  // if(A == 999){
  //   Serial.println(get_Ball_angle);
  // }
  /*--------------------------------------------------------------------------------------------------------------------------*/
  if(motor_flag == 1){
    if(A == 12) motor->calc(float(move_angle), pid_angle, int(motor_speed), 1, flont_cam_angle);
    else motor->calc_a(float(move_angle), pid_angle, int(motor_speed), 0);
  }
  else{
    motor->calc_a(float(move_angle), pid_angle, int(motor_speed), 1);
  }

  if(active_Kick == 1){
    active_Kick = kick->shoot();
  }

  if(Line_angle != 999){
    Last_Line_angle = Line_angle;
  }
  Last_dash_flag = dash_flag;
  Last_cam_flag = cam_flag;
  // Last_dash_flag_near = dash_flag_near;
  defence_step = A;
}
