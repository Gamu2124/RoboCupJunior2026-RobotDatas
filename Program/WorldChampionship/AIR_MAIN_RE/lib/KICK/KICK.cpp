#include <KICK.h>

KICK::KICK()
{
  pinMode(kick, OUTPUT);
  pinMode(charge, OUTPUT);
  digitalWrite(charge, HIGH);
}

/*--------------------------------------------------------------------------------------------------------------------------*/


int KICK::shoot()
{
  if(Kick_flag == 0){
    Kick_flag = 1;
    timer_kick.reset(); // キック開始時間を記録
  }
  if(Kick_flag == 1){
    if(timer_kick.read_ms() <= 60){ //コンデンサ充電停止
      digitalWrite(charge,LOW);
    }
    else if(timer_kick.read_ms() < 150){
      digitalWrite(kick,HIGH);
    }
    else if(timer_kick.read_ms() < 250){
      digitalWrite(kick,LOW);
    }
    else if(timer_kick.read_ms() < 1050){
      digitalWrite(charge,HIGH);
      digitalWrite(kick,LOW);
    }
    else{
      Kick_flag = 0;
      digitalWrite(kick,LOW);
    }
  }
  return Kick_flag;
}
/*--------------------------------------------------------------------------------------------------------------------------*/