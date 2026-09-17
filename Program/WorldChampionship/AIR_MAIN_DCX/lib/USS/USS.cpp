#include "USS.h"

void USS::begin(unsigned long baud){
  Serial5.begin(baud);
}

void USS::update(){
  // 送信側が [255, dist1, dist2, 0] の4バイトなので、4バイト以上あるか確認
  while (Serial5.available() >= 4) {
    
    // 1. 先頭の 255 (ヘッダー) を探す
    if (Serial5.peek() == 255) {
      Serial5.read(); // 255 を取り出す
      
      // 2. 残りの3バイトを確実に読み取る
      right_diss = Serial5.read();
      left_diss  = Serial5.read();
      Serial5.read(); // 最後の 0 (フッター) を捨てる
      
      // ここで受信成功時の処理ができる
    } 
    else {
      // 255 でなければ、1バイト捨てて次を探す（同期合わせ）
      Serial5.read();
    }
  }
}
