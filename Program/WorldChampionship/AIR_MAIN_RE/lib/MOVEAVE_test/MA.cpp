#include <MA.h>

MA::MA() {

}

void MA::setup(int size){
  if(size > MAX_SIZE) size = MAX_SIZE;
  windowSize = size;
  index = 0;
  count = 0;
  total = 0.0;
  for(int i = 0; i < MAX_SIZE; i++){
    buffer[i] = 0.0;
  }
}

double MA::add(double value){
  total -= buffer[index];      // 古い値を合計から引く
  buffer[index] = value;       // 新しい値を入れる
  total += value;              // 合計に加える

  index = (index + 1) % windowSize;  // 次の書き込み位置へ
  if (count < windowSize) count++;   // データ数を更新

  return total / count;              // 平均を返す
}

void MA::reset() {
  for(int i = 0; i < windowSize; ++i) buffer[i] = 0.0;
  index = 0;
  count = 0;
  total = 0.0;
}
