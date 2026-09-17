#include <Arduino.h>
#include <math.h>
#include "hardware/pwm.h" 
#include "hardware/gpio.h" // 【重要】RP2350専用のGPIO制御関数を使うために追加

uint slice_num_all;

// 全GPIOを1クロックでキャプチャするインライン関数
__force_inline static uint64_t get_all_gpios_fast(void) {
  uint32_t hi, lo;
  __asm volatile ("mrrc p0, #0, %0, %1, c8" : "=r" (lo), "=r" (hi));
  return ((uint64_t)hi << 32) | lo;
}

const uint8_t gpio_pins[32] = {
  17, 16, 15, 14, 13, 12, 11, 10,
   6,  3,  2,  1, 47, 46, 45, 44,
  43, 39, 38, 37, 36, 35, 34, 33,
  29, 28, 23, 22, 21, 20, 19, 18
};

void debugPrint(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3);

void setup()
{
  Serial.begin(115200);
  
  Serial2.setTX(4);
  Serial2.setRX(5);
  Serial2.begin(921600);

  // PWM初期化
  gpio_set_function(26, GPIO_FUNC_PWM);
  gpio_set_function(27, GPIO_FUNC_PWM);
  
  slice_num_all = pwm_gpio_to_slice_num(27); 
  pwm_set_wrap(slice_num_all, 255);          

  pwm_set_chan_level(slice_num_all, pwm_gpio_to_channel(27), 200); //LED 200
  pwm_set_chan_level(slice_num_all, pwm_gpio_to_channel(26), 160); //しきい値 180
  pwm_set_enabled(slice_num_all, true);

  pinMode(25, OUTPUT); 
  pinMode(24, OUTPUT); 
  digitalWrite(25, HIGH); 
  delay(1000); 
  digitalWrite(25, LOW); 

  // 【修正】ArduinoのpinModeを使わず、RP2350のSDK関数で30番以上のピンも強制的に確実に初期化する
  for (int i = 0; i < 32; i++) {
    uint8_t pin = gpio_pins[i];
    gpio_init(pin);             // ピンをSIO（GPIO機能）として初期化
    gpio_set_dir(pin, GPIO_IN); // 入力方向に設定
    gpio_set_pulls(pin, false, true); // プルアップ=false, プルダウン=true
  }
}

void loop()
{
  const uint64_t g = get_all_gpios_fast(); 

  // 64ビット空間でのパッキング
  uint8_t b0 = (((g >> 17) & 1ULL) << 0) | (((g >> 16) & 1ULL) << 1) | (((g >> 15) & 1ULL) << 2) | (((g >> 14) & 1ULL) << 3) |
               (((g >> 13) & 1ULL) << 4) | (((g >> 12) & 1ULL) << 5) | (((g >> 11) & 1ULL) << 6) | (((g >> 10) & 1ULL) << 7);

  uint8_t b1 = (((g >>  6) & 1ULL) << 0) | (((g >>  3) & 1ULL) << 1) | (((g >>  2) & 1ULL) << 2) | (((g >>  1) & 1ULL) << 3) |
               (((g >> 47) & 1ULL) << 4) | (((g >> 46) & 1ULL) << 5) | (((g >> 45) & 1ULL) << 6) | (((g >> 44) & 1ULL) << 7);

  uint8_t b2 = (((g >> 43) & 1ULL) << 0) | (((g >> 39) & 1ULL) << 1) | (((g >> 38) & 1ULL) << 2) | (((g >> 37) & 1ULL) << 3) |
               (((g >> 36) & 1ULL) << 4) | (((g >> 35) & 1ULL) << 5) | (((g >> 34) & 1ULL) << 6) | (((g >> 33) & 1ULL) << 7);

  uint8_t b3 = (((g >> 29) & 1ULL) << 0) | (((g >> 28) & 1ULL) << 1) | (((g >> 23) & 1ULL) << 2) | (((g >> 22) & 1ULL) << 3) |
               (((g >> 21) & 1ULL) << 4) | (((g >> 20) & 1ULL) << 5) | (((g >> 19) & 1ULL) << 6) | (((g >> 18) & 1ULL) << 7);

  Serial2.write(255);       
  Serial2.write(b0);
  Serial2.write(b1);
  Serial2.write(b2);
  Serial2.write(b3);
  Serial2.write(128);       

  // debugPrint(b0, b1, b2, b3);
}

void debugPrint(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3) {
  static uint32_t lastPrint = 0;
  if (millis() - lastPrint < 50) return;
  lastPrint = millis();

  Serial.print("Pins(32bit): ");
  for(int i=0; i<8; i++) Serial.print((b0 >> i) & 1); Serial.print(" ");
  for(int i=0; i<8; i++) Serial.print((b1 >> i) & 1); Serial.print(" ");
  for(int i=0; i<8; i++) Serial.print((b2 >> i) & 1); Serial.print(" ");
  for(int i=0; i<8; i++) Serial.print((b3 >> i) & 1);
  Serial.println();
}