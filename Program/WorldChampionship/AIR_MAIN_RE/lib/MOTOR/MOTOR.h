#pragma once
#include <Arduino.h>
#include <math.h>
#include <MA.h>

class MOTOR
{
private:
    MA moveave_1;
    MA moveave_2;
    MA moveave_3;
    MA moveave_4;

    const int LIMIT = 250;
    const int LAP[4] = {9,6,5,4}; //正転逆転の指定
    double Ave_motor_speed[4] = {0,0,0,0}; //前回のモーターの出力
    double Now_motor_speed[4] = {0,0,0,0}; //モーターそれぞれの出力
    double First_motor_speed[4] = {0,0,0,0}; //モーターそれぞれの出力
    double Second_motor_speed[4] = {0,0,0,0}; //モーターそれぞれの出力
    int flont_flag = 0;
    int mAngle[4] = {45,135,225,315}; //それぞれのモーターの取り付け角
    double Last_pd_VAL;
    float NOW_DIR;
    float LAST_DIR;


public:
    MOTOR();
    void calc(float,float,int,int,float);
    void calc_a(float,float,int,int);
    void output_();
    void stop();
};
