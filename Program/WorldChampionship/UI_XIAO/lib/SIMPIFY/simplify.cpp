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