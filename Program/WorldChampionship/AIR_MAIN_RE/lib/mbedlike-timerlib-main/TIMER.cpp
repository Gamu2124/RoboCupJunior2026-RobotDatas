#include "TIMER.h"

TIMER::TIMER(){
	tm = millis();
	tu = micros();
}

void TIMER::reset(){
	tm = millis();
	tu = micros();
}

unsigned long TIMER::read_ms(){
	return (millis() - tm);
}

unsigned long TIMER::read_us(){
	return (micros() - tu);
}
