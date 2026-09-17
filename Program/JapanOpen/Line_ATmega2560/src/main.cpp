#include <Arduino.h>

// ピン情報の定義
volatile uint8_t* pins[] = {
  &PIND, &PIND, &PINL, &PINL, &PINL, &PINL, &PINH, &PINH,
  &PINH, &PINH, &PINH, &PINH, &PINJ, &PINJ, &PINJ, &PINJ,
  &PINJ, &PINC, &PINC, &PINC, &PINC, &PINC, &PINC, &PINC
};

uint8_t bits[] = {
  7, 6, 7, 6, 5, 4, 6, 5,
  4, 3, 2, 1, 7, 6, 5, 4,
  3, 7, 6, 5, 4, 3, 2, 1
};

// デバッグ表示用関数
void debugPrint(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t side_grid);

void setup() {
  Serial.begin(9600);  // PCデバッグ用
  Serial1.begin(115200); // Teensy通信用
  // 入力ピン初期化
  DDRA &= ~(_BV(PA2) | _BV(PA6) | _BV(PA5));
  DDRD &= ~(_BV(PD7) | _BV(PD6));
  DDRL &= ~(_BV(PL7) | _BV(PL6) | _BV(PL5) | _BV(PL4));
  DDRH &= ~(_BV(PH6) | _BV(PH5) | _BV(PH4) | _BV(PH3) | _BV(PH2) | _BV(PH1));
  DDRJ &= ~(_BV(PJ7) | _BV(PJ6) | _BV(PJ5) | _BV(PJ4) | _BV(PJ3) | _BV(PJ0));
  DDRC &= ~(_BV(PC7) | _BV(PC6) | _BV(PC5) | _BV(PC4) | _BV(PC3) | _BV(PC2) | _BV(PC1));
  DDRF &= ~(_BV(PF0) | _BV(PF1) | _BV(PF3) | _BV(PF6));
  DDRK &= ~(_BV(PK0) | _BV(PK1) | _BV(PK4) | _BV(PK5) | _BV(PK6));
  pinMode(5, OUTPUT);
  TCCR3B = (TCCR3B & 0xF8) | 0x01; 
  analogWrite(5, 85); //26朝150 //80
}

void loop() {
  // unsigned long startTime = micros(); // 開始（マイクロ秒） 
  // 1. ラインセンサーパッキング
  uint8_t b0 = 0, b1 = 0, b2 = 0;
  for (int i = 0; i < 8; i++)  if ((*pins[i] >> bits[i]) & 0x01) b0 |= (1 << i);
  for (int i = 8; i < 16; i++) if ((*pins[i] >> bits[i]) & 0x01) b1 |= (1 << (i - 8));
  for (int i = 16; i < 24; i++)if ((*pins[i] >> bits[i]) & 0x01) b2 |= (1 << (i - 16));

  // 2. サイド & Gridパッキング
  uint8_t side_grid = 0;
  if ((PINA >> 2) & 0x01) side_grid |= (1 << 7); // F
  if ((PINF >> 6) & 0x01) side_grid |= (1 << 6); // L
  if ((PINA >> 6) & 0x01) side_grid |= (1 << 5); // R
  if ((PINA >> 5) & 0x01) side_grid |= (1 << 4); // B

  if (((PINF >> 0) | (PINF >> 1) | (PINF >> 3)) & 0x01) side_grid |= (1 << 3); // GL
  // if (((PINK >> 0) | (PINK >> 1) | (PINJ >> 0)) & 0x01) side_grid |= (1 << 2); // GR
  if (((PINK >> 4) | (PINK >> 5) | (PINK >> 6)) & 0x01) side_grid |= (1 << 1); // GB

  // 3. パケット送信 (6バイト)
  Serial1.write(255);       // Header
  Serial1.write(b0);
  Serial1.write(b1);
  Serial1.write(b2);
  Serial1.write(side_grid);
  Serial1.write(128);       // Footer

  // debugPrint(b0, b1, b2, side_grid);
}

// デバッグ用関数：シリアルモニタで人間が読みやすい形式で表示
void debugPrint(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t side_grid) {
  static uint32_t lastPrint = 0;
  if (millis() - lastPrint < 50) return; // 表示が速すぎて読めないのを防ぐ(20Hz)
  lastPrint = millis();

  Serial.print("L:");
  // 24bit分をバイナリ表示
  for(int i=0; i<8; i++) Serial.print((b0 >> i) & 1);
  for(int i=0; i<8; i++) Serial.print((b1 >> i) & 1);
  for(int i=0; i<8; i++) Serial.print((b2 >> i) & 1);

  Serial.print(" | S(FLRB):");
  Serial.print((side_grid >> 7) & 1);
  Serial.print((side_grid >> 6) & 1);
  Serial.print((side_grid >> 5) & 1);
  Serial.print((side_grid >> 4) & 1);

  Serial.print(" | G(LRB):");
  Serial.print((side_grid >> 3) & 1);
  Serial.print((side_grid >> 2) & 1);
  Serial.print((side_grid >> 1) & 1);
  Serial.println();
}