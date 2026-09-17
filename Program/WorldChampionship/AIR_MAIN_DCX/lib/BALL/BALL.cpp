#include <BALL.h>

void BALL::begin(unsigned long baud){
  Serial3.begin(baud);
  pinMode(catch_photo_pin, INPUT);
  moveave_hold.setup(40);
  moveave_ballhold.setup(60);
  // moveave_far は生データ運用の利益のため廃止
  MAX_CATCHVAL = 100;
  
  // バッファ等の初期化
  rxIndex = 0;
  ballFound = false;
  Ball_angle = 0;
  Ball_dist = 0;
}

// =============================
// 新・パケット処理関数（生データを即座に反映）
// =============================
bool BALL::updateBallSensor() {
  int availableBytes = Serial3.available();
  if (availableBytes == 0) return false;

  bool updated = false;

  while (availableBytes-- > 0) {
    uint8_t inByte = Serial3.read();

    // 1バイト目：ヘッダー(0xAA)の確認
    if (rxIndex == 0) {
      if (inByte == 0xAA) {
        rxBuf[0] = inByte;
        rxIndex = 1;
      }
      continue;
    }

    // 2〜4バイト目の受信
    rxBuf[rxIndex++] = inByte;

    // 4バイト揃ったら解析
    if (rxIndex >= 4) {
      if (rxBuf[1] == 0x80) {
        // ボールが見つからない場合
        ballFound = false;
        Ball_dist = 0;
        // ※見失った時の角度の扱いは、状況に応じて 999 や前回の値を維持してください
        Ball_angle = 999; 
      } 
      else {
        // ボールを発見した場合
        ballFound = true;
        Ball_angle = (rxBuf[1] << 8) | rxBuf[2]; // 上位・下位バイトを結合して角度を算出
        int rawDist = rxBuf[3];

        if (rawDist == 0) {
          Ball_dist = 0;
        } else {
          // 一切のフィルタを通さず、生の距離データを即座に格納
          Ball_dist = rawDist;
        }
      }
      rxIndex = 0; // バッファインデックスをリセット
      updated = true;
    }
  }
  return updated;
}

int BALL::getAngle() const {
  return Ball_angle;
}

// 元の getFar() は新しい Ball_dist を返すように変更
int BALL::getFar() const {
  return Ball_dist; 
}

/*--------------------------------------------------------------------------------------------------------------------------*/

void BALL::updateCatch(float angle)
{
  catch_val = moveave_ballhold.add(analogRead(catch_photo_pin));
  // Serial.println(catch_val);
  if(catch_val > 100 && (abs(simplify.goPM(angle))) < 8.0f){
    isHold = 1;
  }
  else{
    isHold = 0;
  }
  
  isHold_ave = moveave_hold.add(isHold);
  // Serial.println(isHold_ave);

  if(isHold_ave > 0.1){
    if(wait_catch == 0){
      timer_wait_hold.reset();
      wait_catch = 1; 
    }
  }
  else{
    isBall_catch = 0;
    wait_catch = 0;
  }

  if(timer_wait_hold.read_ms() > 250 && wait_catch == 1){
    isBall_catch = 1;
  }

  if(isBall_catch == 1){
    timer_no_hold.reset();
  }
  else{
    if(200 < timer_no_hold.read_ms()){
      isBall_catch = 0; 
    }
    else{
      isBall_catch = 1;
    }
  }
  // Serial.println(isHold);
}

int BALL::isCaught() const
{
  return isBall_catch;
}

int BALL::catch_sent()
{
  return catch_val;
}

int BALL::Kick_val_sent() const
{
  return KICK_VAL;
}

/*--------------------------------------------------------------------------------------------------------------------------*/

double BALL::calc_dt()
{
  dt = 0;
  dt = (timer_pd.read_ms() - pre_time);
  pre_time = timer_pd.read_ms();
  return dt;
}

/*--------------------------------------------------------------------------------------------------------------------------*/

// 周り込み関数（最初のコードで使われていた引数構成に合わせられるよう、適宜整理しています）
std::pair<float, int> BALL::around(int angle, float far, int type, int farval, int speed) //テスト
{
  float nomal_angle = angle;
  float use_ang = 0;
  originaly_ang = 0;
  isOverHalf = 0;
  
  // 生データ運用のため、前回の値との平均化（Last_far）を廃止し、直値で処理
  // angle = simplify.FixLimit(angle+3);
  if(angle > 180)
  {
    originaly_ang = angle;
    angle = 360 - angle;
    isOverHalf = 1;
  }
  
  if(isOverHalf == 0){
    way = 1;
  }
  else{
    way = 2;
  }

  // 最初のメインループで使われていた type == 0 のアルゴリズムを基準に処理
  if(type == 0) 
  {
    if(angle < 30){
      speed *= 0.75;
    }

    if(8 <= angle && angle < 20){
      // use_ang = angle * 2;
      use_ang = 25;
    }
    else if(20 <= angle && angle < 50){
      use_ang = 40;
    }
    else if(50 <= angle && angle < 90){
      use_ang = 60;
    }
    else if(90 <= angle && angle < 120){
      use_ang = 60;
    }
    else if(120 <= angle){
      use_ang = 60;
    }
    else{
      use_ang = 0;
    }
    if(use_ang > 25){
      angle += use_ang * (far / 30); //近いとき4000の時 
    }
    else{
      angle += use_ang;
    }
    if(angle <= 10){
      speed *= 1.2;
    }
    
  }

  if((isOverHalf == 1))
  {
    angle = 360 - angle;
  }

  angle = simplify.FixLimit(angle);

  // 最初のコードの戻り値 (myAngle, mySpeed) のペアに対応
  return std::make_pair((float)angle, speed);
}

int BALL::whichway(){
  return way;
}

/*--------------------------------------------------------------------------------------------------------------------------*/

double BALL::get_DirAdder(float angle)
{
  double DIR_adder;
  if(angle > 180)
  {
    originaly_ang = angle;
    angle = 360 - angle;
    isOverHalf = 1;
  }

  if(0 <= angle && angle <= 45)
  {
    // DIR_adder = angle + x_DIR;
  }
  else
  {
    DIR_adder = 0;
  }

  if(isOverHalf == 1)
  {
    DIR_adder *= -1; 
  }

  return DIR_adder;
}

// 古い車輪速計算用の vec_speed は、生データ化に伴い不要なため形だけ維持、または廃止可能
float BALL::vec_speed(float angle, int far)
{
  return 0.0f;
}