#include <Arduino.h>
#pragma once

class SIMPLIFY
{
private:

public:
    SIMPLIFY();
    float FixLimit(float);
    float RoboToMath(float); //rcj座標系からC++座標系に角度を変換します 丸め済み
    float MathToRobo(float); //C++座標系からrcj座標系に角度を変換します 丸め済み
    int speed_change(int,int,int,int); //min,max,farmax
};
