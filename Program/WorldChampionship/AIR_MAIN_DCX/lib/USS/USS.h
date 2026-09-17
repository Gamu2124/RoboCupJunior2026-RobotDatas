#pragma once
#include <Arduino.h>

class USS{
private:
    uint8_t right_diss;
    uint8_t left_diss;

public:
  void begin(unsigned long baud);
  void update();  
};
