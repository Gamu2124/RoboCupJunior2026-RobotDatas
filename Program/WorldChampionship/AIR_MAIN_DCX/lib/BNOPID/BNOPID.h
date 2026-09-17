#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <TIMER.h>
#include <simplify.h>

class BNOPID
{
private:
  // float i;
  double pre_p;
  double p, i, d = 0;
  // double P;
  // double D;
  /* Set the Delay between the Fresh Samples and Variables */
  #define BNO055_SAMPLERATE_DELAY_MS (100)
  Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);
  sensors_event_t event;
  double Now_DIR;
  double change_DIR;
  double PD_DIR;
  int isDirMinus = 0;
  double DIR;
  double Diss;
  double this_DIR = 0;
  double dt = 0;
  double pre_time = 0;
  TIMER timer1;
  SIMPLIFY simplify;
  // double PID_add = 0;
  // double pre_p = 0;

public:
  double get_x(double);
  void setup();
  void print();
  void set_target();
  double get_pd(double);
  double get_camPD(double);
  double calc_dt();
  double First_DIR;
  double Last_DIR = 0;
  float getDIR();};