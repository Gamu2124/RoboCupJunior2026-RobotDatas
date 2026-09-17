#pragma once
#include <Arduino.h>
#include <TIMER.h>


class KICK
{
private:
    const int charge = 32;
    const int kick = 31;
    int Kick_flag = 0;
    TIMER timer_kick;
    

public:
    KICK();
    int shoot();
};
