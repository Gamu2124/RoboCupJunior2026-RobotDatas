#include <MOTOR.h>

MOTOR::MOTOR()
{
  int NUM_MOVEAVE = 10;
  for(int i = 0; i < 4; i++)
  {
    pinMode(LAP[i], OUTPUT);
  }
  for(int i = 0; i < 4; i++){
    analogWriteFrequency(LAP[i], 80000);
  }
  moveave_1.setup(NUM_MOVEAVE);
  moveave_2.setup(NUM_MOVEAVE);
  moveave_3.setup(NUM_MOVEAVE);
  moveave_4.setup(NUM_MOVEAVE);
}


/*--------------------------------------------------------------------------------------------------------------------------*/


void MOTOR::stop()
{
  for(int i = 0; i < 4; i++)
  {
    analogWrite(LAP[i],HIGH);
  }
}


/*--------------------------------------------------------------------------------------------------------------------------*/



void MOTOR::calc(float ang, float DIR, int speed, int detect, float cam_angle)
{
  if(speed > 170) speed = 170; //175
  for(int i = 0; i < 4; i++) //モーターの個数だけ回す
  {
    Now_motor_speed[i] = sin(radians(ang - mAngle[i])) * speed;
  }

  // if(detect == 1){
  //   moveave_1.reset();
  //   moveave_2.reset();
  //   moveave_3.reset();
  //   moveave_4.reset();
  // }

  Ave_motor_speed[0] = moveave_1.add(Now_motor_speed[0]);
  Ave_motor_speed[1] = moveave_2.add(Now_motor_speed[1]);
  Ave_motor_speed[2] = moveave_3.add(Now_motor_speed[2]);
  Ave_motor_speed[3] = moveave_4.add(Now_motor_speed[3]);

  if(detect == 1){
    float use_val = 0;
    use_val = (abs(cam_angle) * 0.04) + 2.5;

    if(DIR < 0){
      Ave_motor_speed[0] += DIR * use_val; 
      Ave_motor_speed[1] += DIR * use_val+0.5;
      Ave_motor_speed[2] -= DIR/2; 
      Ave_motor_speed[3] -= DIR/2; 
    }
    if(DIR >= 0){
      Ave_motor_speed[0] -= DIR/2; 
      Ave_motor_speed[1] -= DIR/2; 
      Ave_motor_speed[2] += DIR * use_val+0.5; 
      Ave_motor_speed[3] += DIR * use_val; 
    }
  }
  else{
    for(int i = 0; i < 4; i++){
      Ave_motor_speed[i] += DIR; 
    }
  }

  for(int i = 0; i < 4; i++){
    if(Ave_motor_speed[i] > 230) Ave_motor_speed[i] = 230;
    if(Ave_motor_speed[i] < -230) Ave_motor_speed[i] = -230;
  }

  output_();
}

/*--------------------------------------------------------------------------------------------------------------------------*/


void MOTOR::calc_a(float ang, float DIR, int speed, int detect)
{
  int MAX_VAL = speed - abs(DIR);
  for(int i = 0; i < 4; i++) //モーターの個数だけ回す
  {
    // if(DIR >= 35){
    //   DIR -= 3;
    // }
    // if(DIR <= -35){
    //   DIR += 3;
    // }
    Now_motor_speed[i] = sin(radians(ang - mAngle[i])) * MAX_VAL;
    // Now_motor_speed[i] = Now_motor_speed[i];
  }

  // if(detect == 1){
  //   moveave_1.reset();
  //   moveave_2.reset();
  //   moveave_3.reset();
  //   moveave_4.reset();
  // }
  // Now_motor_speed[0] = moveave_1.add(double(Now_motor_speed[0]));
  // Now_motor_speed[1] = moveave_2.add(double(Now_motor_speed[1]));
  // Now_motor_speed[2] = moveave_3.add(double(Now_motor_speed[2]));
  // Now_motor_speed[3] = moveave_4.add(double(Now_motor_speed[3]));

  for(int i = 0; i < 4; i++){
    Ave_motor_speed[i] = Now_motor_speed[i];
  }
  
  for(int i = 0; i < 4; i++){
    Ave_motor_speed[i] += DIR;
  }
  
  
  if(DIR > 40) DIR = 40;
  if(DIR < -40) DIR = -40;

  // const int LIMIT = 200;

  // // ① 最大絶対値を探す
  // int max_val = 0;
  // for(int i = 0; i < 4; i++){
  //   if(abs(Ave_motor_speed[i]) > abs(max_val)){
  //     max_val = Ave_motor_speed[i];
  //   }
  // }

  // // ② 超過していたら全体を平行シフト
  // if(abs(max_val) > LIMIT){
  //   int over = max_val - (max_val > 0 ? LIMIT : -LIMIT);

  //   for(int i = 0; i < 4; i++){
  //     Ave_motor_speed[i] -= over;
  //   }
  // }


  output_();
}


/*--------------------------------------------------------------------------------------------------------------------------*/


void MOTOR::output_()
{
  for(int i = 0; i < 4; i++)
  {
    // Ave_motor_speed[i] *= -1;
     // -255〜255 → 0〜255 に変換
    Ave_motor_speed[i] = static_cast<int>((Ave_motor_speed[i] * 127.0 / 255.0) + 128.0);

    // 念のため 0〜255 に制限
    if(Ave_motor_speed[i] > 255) Ave_motor_speed[i] = 255;
    if(Ave_motor_speed[i] < 0) Ave_motor_speed[i] = 0;
    Ave_motor_speed[i] = 255 - Ave_motor_speed[i];
  }

  // PWM出力（反転）
  for(int i = 0; i < 4; i++)
  {
    analogWrite(LAP[i], Ave_motor_speed[i]);
  } 
  // Serial.println();
}

