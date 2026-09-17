#include <Arduino.h>
#pragma once

class MA {
  private:
    static const int MAX_SIZE = 50;
    double buffer[MAX_SIZE];
    int windowSize, index, count;
    double total;

  public:
    MA();                    // コンストラクタ
    void setup(int);         // 初期化
    double add(double value);  // 値を追加＋平均を返す
    void reset();              // リセット
};
