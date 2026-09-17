#include <simplify.h>

SIMPLIFY::SIMPLIFY()
{
  
}

float SIMPLIFY::FixLimit(float value)
{
  if(value >= 360)
  {
    value -= 360;
  }
  else if(value < 0)
  {
    value += 360;
  }
  return value;
}

float SIMPLIFY::RoboToMath(float value)
{
  value = FixLimit(90 - value);
  return value;
}

float SIMPLIFY::MathToRobo(float value)
{
  value = FixLimit(90 - value);
  return value;
}

int SIMPLIFY::speed_change(int speed_min, int speed_max, int far_max, int far_now){
  int speed = 0;

  if(far_now < 0) far_now = 0;
  if(far_now > far_max) far_now = 50;
  speed = speed_min + (far_max - far_now) / far_max * 180;
  if(speed > 160){
    speed = 160;
  }

  return speed;
}

float SIMPLIFY::goPM(float value){
  if(value > 180) value -= 360;
  return value;
}

float SIMPLIFY::goMP(float value){
  if(value < 0) value += 360;
  return value;
}

float SIMPLIFY::smooth_angle_vector(float target, float current, float alpha){
  // 1. 度数をラジアンに変換
  float target_rad = target * (M_PI / 180.0f);
  float current_rad = current * (M_PI / 180.0f);

  // 2. それぞれを単位ベクトル (x, y) に変換
  float target_x = cosf(target_rad);
  float target_y = sinf(target_rad);
  float current_x = cosf(current_rad);
  float current_y = sinf(current_rad);

  // 3. ベクトル空間で線形補間（これが「なだらか」にするコア処理）
  float next_x = current_x + alpha * (target_x - current_x);
  float next_y = current_y + alpha * (target_y - current_y);

  // 4. 再び角度（ラジアン）に戻す
  float next_rad = atan2f(next_y, next_x);

  // 5. 度数に戻し、0~360の範囲に整える
  float next_deg = next_rad * (180.0f / M_PI);
  if (next_deg < 0) {
      next_deg += 360.0f;
  }

  return next_deg;
}