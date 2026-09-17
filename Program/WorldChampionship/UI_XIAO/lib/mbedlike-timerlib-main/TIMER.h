#ifndef TIMER_H
#define TIMER_H

#include "Arduino.h"

class TIMER{
public:
	TIMER();
	void reset();
	unsigned long read_ms();
	unsigned long read_us();
    unsigned long tm;
    unsigned long tu;
private:
};
#endif