#include <BNOPID.h>

void BNOPID::setup()
{
  Wire.setSDA(18); // Wire1のSDAピン
  Wire.setSCL(19); // Wire1のSCLピン
  Wire.begin();
  Wire.setClock(400000); 
  delay(100);
  bno.begin(OPERATION_MODE_IMUPLUS);
  // I2Cクロックを高速化（Teensy 4.1のパワーを活かす。BNO055はWire1にあるためWire1を設定）


  delay(1500);
  pre_time = micros();
}

/*--------------------------------------------------------------------------------------------------------------------------*/

void BNOPID::set_target()
{
  // getEventの代わりに軽量なgetVectorを使用
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  event.orientation.x = euler.x();
  First_DIR = event.orientation.x; //初期角度 
  change_DIR = 360 - First_DIR; //MAX角度 - 初期角度
  First_DIR = First_DIR + change_DIR; //360度になるように調整
  if(First_DIR == 360)
  {
    First_DIR = 0;
  }
}

/*--------------------------------------------------------------------------------------------------------------------------*/

double BNOPID::get_x(double adder_ex)
{
  Now_DIR = event.orientation.x;
  Now_DIR += change_DIR;

  adder_ex *= -1;
  Now_DIR += adder_ex;

  if(Now_DIR >= 360){
    Now_DIR -= 360;
  }
  else if(Now_DIR < 0){
    Now_DIR += 360;
  }

  this_DIR = Now_DIR;
  if(this_DIR >= 180){
    this_DIR = Now_DIR - 360;
  }

  return this_DIR;
}

/*--------------------------------------------------------------------------------------------------------------------------*/

double BNOPID::calc_dt()
{
  double dt_t = timer1.read_ms();
  dt = (dt_t - pre_time);
  pre_time = dt_t;
  return dt;
}

double BNOPID::get_pd(double adder)
{
  double pid;
  static double last_pid = 0;
  static uint32_t last_bno_read = 0;
  static double last_d = 0; // LPF用
  uint32_t now_ms = millis();

  // 制御周期を5ms（200Hz）に高速化
  if (now_ms - last_bno_read < 5) return last_pid;
  double dt_sec = (now_ms - last_bno_read) / 1000.0;
  if (dt_sec <= 0.0) dt_sec = 0.001; // ゼロ除算防止の防御的コード
  last_bno_read = now_ms;

  // 1. 通信と角度取得（軽量なgetVectorを用いて物理読み出しのオーバーヘッドを削減）
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  event.orientation.x = euler.x();
  
  // 物理的なロボットの向き（adderを含まない実測値）と、目標を含めたNow_DIRを取得
  double physical_DIR = get_x(0);
  Now_DIR = get_x(adder);

  // 2. 誤差Pの計算（境界またぎ補正）
  p = First_DIR - Now_DIR;
  if (p > 180) p -= 360;
  if (p < -180) p += 360;

  // 3. I分の計算（ダイナミックdtを適用）
  if (abs(p) < 20) { // 誤差が小さい時（安定させたい時）だけIを効かせる
    i += p * dt_sec; 
  } else {
    i = 0; // 誤差が大きい時はリセット（ワインドアップ防止）
  }
  i = constrain(i, -10, 10); // I分の最大値を制限

  // 4. D分の計算（微分先行型：測定値の微分を用いて目標値変更によるキックを防止）
  static double pre_physical_DIR = 0;
  static bool first_run = true;
  if (first_run) {
    pre_physical_DIR = physical_DIR;
    first_run = false;
  }

  // 物理角度の変動幅（境界またぎ補正付き）
  double d_angle = physical_DIR - pre_physical_DIR;
  if (d_angle > 180) d_angle -= 360;
  if (d_angle < -180) d_angle += 360;

  // 測定値の微分（D-on-Measurement）: 出力は -Kd * (dy/dt) となるため符号はマイナス
  double raw_d = -d_angle / dt_sec;

  // 前回のD分を80%、今回のD分を20%で混ぜるローパスフィルタ
  double filtered_d = (last_d * 0.8) + (raw_d * 0.2); 
  last_d = filtered_d;
  pre_physical_DIR = physical_DIR;

  // 5. PID合計（極めて迅速に復帰し、かつブレを防ぐ爆速パラメータ）
  double kp = 1.5;  // 1.2から2.2へ引き上げ、より素早く復帰
  double ki = 0.0;  // まずは0で安定
  double kd = 0.02; // 0.03から0.05へ引き上げ、オーバーシュートを抑える急制動0.07

  pid = (p * kp) + (i * ki) + (filtered_d * kd);
  
  pre_p = p;
  last_pid = pid;

  // Teleplot用の出力を追加（正面0度を基準とした現在角度、目標値0、PIDの操作量）
  // Serial.print(">relative_angle3:");
  // Serial.print(get_x(0)); // 現在の相対角度（-180〜180度、正面が0）
  // Serial.print(";target_angle:0"); // 目標値（0度）の基準線
  // Serial.print(";pid_output:");
  // Serial.println(pid); // PIDの出力（操作量）

  return pid;
}

double BNOPID::get_camPD(double adder)
{
  // get_pdとロジックが共通なため、本来は内部でget_pdを呼ぶか、
  // パラメータを共通化して10msガードを効かせるのが望ましいです
  return get_pd(adder); 
}

float BNOPID::getDIR()
{
  float now_Dir;
  now_Dir = 0;
  now_Dir = get_x(0);
  if(now_Dir >= 180){
    now_Dir -= 360;
  }
  return now_Dir;
}

/*--------------------------------------------------------------------------------------------------------------------------*/

void BNOPID::print()
{
  Serial.print("DIR : ");
  Serial.print(get_x(0));
  Serial.print("  ");
  Serial.print("±DIR : ");
  Serial.print(get_pd(0));
  Serial.println();

  // BNO055から線形加速度（重力を除いた加速度）を取得
  imu::Vector<3> accel = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
  // X軸とY軸の加速度をベクトル合成 (合成値 = √(x^2 + y^2))
  double accel_xy = sqrt(accel.x() * accel.x() + accel.y() * accel.y());

  // Teleplot用のフォーマットで出力 (>変数名:タイムスタンプ:値)
  // 横軸が時間(t)、縦軸が合成加速度(accel_xy)になります
  Serial.print(">accel_xy:");
  Serial.print(millis());
  Serial.print(":");
  Serial.println(accel_xy);
}