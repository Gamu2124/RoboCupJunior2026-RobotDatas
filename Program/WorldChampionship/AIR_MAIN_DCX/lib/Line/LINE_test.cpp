#include <LINE_test.h>

LINE_TEST::LINE_TEST()
{

}

void LINE_TEST::begin(unsigned long baud){
  Serial1.begin(baud);

  // 【修正】メインラインセンサーの初期化を 24 -> 32 に拡張
  for(int i = 0; i < 32; i++){
      Line_vals[i] = 0;
  }

  // ※ side_vals, grid_vals の初期化は不要になったため削除

  Line_angle = 999;
}

void LINE_TEST::updateSensor(){
  static uint8_t packet[6];
  static int p_idx = 0;

  while (Serial1.available() > 0) {
    uint8_t c = Serial1.read();

    if (p_idx == 0) {
      if (c == 255) { // Header
        packet[0] = c;
        p_idx = 1;
      }
    } else {
      packet[p_idx++] = c;
      if (p_idx == 6) {
        if (packet[5] == 128) { // Footer OK
          
          // 【修正】4バイト（32ビット分）すべてをそのままメインラインセンサー（32個）に展開
          for (int i = 0; i < 8; i++) {
            Line_vals[i]      = (packet[1] >> i) & 0x01; // b0 (0~7)
            Line_vals[i + 8]  = (packet[2] >> i) & 0x01; // b1 (8~15)
            Line_vals[i + 16] = (packet[3] >> i) & 0x01; // b2 (16~23)
            Line_vals[i + 24] = (packet[4] >> i) & 0x01; // b3 (24~31)
          }

          // ※ side_vals, grid_vals にマッピングしていた古い処理は全削除

          updated = true;
        }
        p_idx = 0; // インデックスリセット
      }
    }
  }
}

void LINE_TEST::setRaw(int role)
{
  selected_role = role;
}

void LINE_TEST::vec(float* vec)
{
  Line_side_on = 0;
  vec[0] = 0;
  vec[1] = 0;
  for(int i = 0; i < 5; i++){
    for(int j = 0; j < 2; j++){
      Line_range_num[i][j] = 0;
      Line_vec[i][j] = 0;
    }
    val_count[i] = 0;
  }
  isLineContinue = 0;
  Line_flag_count = 0;
  Line_on = 0;
  Line_angle = 0;
  
  // 【修正】判定を 24 -> 32 に拡張
  for(int i = 0; i < 32; i++){
    if(Line_vals[i] == 1){
      Line_on = 1;
    }
  }

  if(Line_on != 1){
    for(int i = 0; i < 2; i++){
      vec[i] = 9999;
      Last_vec[i] = vec[i];
    }
    return;
  }

  // 【修正】範囲検知アルゴリズムのループを 24 -> 32 に拡張
  for(int i = 0; i < 32; i++){ 
    if(isLineContinue == 1){ 
      if(Line_vals[i] == 1){ 
        val_count[Line_flag_count - 1]++;
        isLineContinue = 1;
      }
      else if(Line_vals[i] == 0){ 
        Line_range_num[Line_flag_count - 1][1] = i - 1; 
        isLineContinue = 0;
      }
    }
    else if(isLineContinue == 0){ 
      if(Line_vals[i] == 1){ 
        isLineContinue = 1; 
        Line_flag_count += 1; 
        val_count[Line_flag_count - 1]++;
        Line_range_num[Line_flag_count - 1][0] = i; 
      }
      else if(Line_vals[i] == 0){ 
        isLineContinue = 0; 
      }
    }
  }

  // 【修正】0番ピンと31番ピンの跨ぎ（リング状の結合）判定を 23 -> 31 に修正
  if(Line_range_num[0][0] == 0 && Line_vals[31] == 1){
    Line_range_num[0][0] = Line_range_num[Line_flag_count - 1][0];
    val_count[0] += val_count[Line_flag_count - 1];
    for(int i = 0; i < 2; i++){
      Line_range_num[Line_flag_count - 1][i] = 0;
    }
    Line_flag_count -= 1;
  }

  for(int j = 0; j < Line_flag_count; j++){
    for(int i = 0; i < val_count[j]; i++){ 
      int this_num = Line_range_num[j][0] + i;
      // 【修正】32個のリングバッファ構造に対応 (23 -> 31, 24 -> 32)
      if(this_num > 31){
        this_num -= 32;
      }
      Line_vec[j][0] += LINE_X[this_num]; // cos
      Line_vec[j][1] += LINE_Y[this_num]; // sin
    }
  }
  
  for(int i = 0; i < Line_flag_count; i++){
    Line_vec[i][0] /= val_count[i];
    Line_vec[i][1] /= val_count[i];
  }  

  for(int i = 0; i < Line_flag_count; i++){ 
    vec[0] += Line_vec[i][0]; 
    vec[1] += Line_vec[i][1];
  }

  for(int i = 0; i < 2; i++){
    vec[i] /= Line_flag_count;
  }

  float cross_threthold = 0.001; 
  if(abs(vec[0]) < cross_threthold && abs(vec[1]) < cross_threthold){
    for(int i = 0; i < 2; i++){
      vec[i] = Last_vec[i];
    }
  }

  DOTproduct_first = vec[0] * First_vec[0] + vec[1] * First_vec[1];
  DOTproduct_last = vec[0] * Last_vec[0] + vec[1] * Last_vec[1];
  if(selected_role == 3){
    if(Last_vec[0] != 9999){
      if(DOTproduct_first < 0){
        for(int i = 0; i < 2; i++){
          vec[i] *= -1;
        }
        overhalf_flag = 1;
      }
      else{
        overhalf_flag = 0;
      }
    }
    else{
      overhalf_flag = 0;
    }
  }

  if(Last_vec[0] == 9999){
    for(int i = 0; i < 2; i++){
      First_vec[i] = vec[i];
    }
  }
  for(int i = 0; i < 2; i++){
    Last_vec[i] = vec[i];
  }
  Last_overhalf_flag = overhalf_flag;
}

/*--------------------------------------------------------------------------------------------------------------------------*/

// 【注意】side_vec関数は呼び出し元との互換性のために残していますが、sideセンサーが無いためダミー（検知なし）として返します
void LINE_TEST::side_vec(float* vec)
{
  for(int i = 0; i < 2; i++){
    vec[i] = 9999;
    Last_vec_side[i] = vec[i];
  }
  Line_side_on = 0;
}

float LINE_TEST::angle(float* veec, float* veeec)
{
  isOverHalf = 0;

  side_vec(veeec); // 常に 9999 が返る
  vec(veec);       // 32個対応のメインベクトル計算

  if(selected_role == 4) veeec[0] = 9999;

  if(veec[0] == 9999 && veeec[0] == 9999){
    for(int i = 0; i < 2; i++){
      First_vec[i] = 9999;
      Last_vec[i] = 9999;
    }
    Line_angle = 999;
    read = 0;
  }
  else if(veec[0] == 9999){
    Line_angle = simplify.FixLimit(degrees(atan2(veeec[1],veeec[0])));
    read = 0;
  }
  else{ 
    Line_angle = degrees(atan2(veec[1],veec[0]));
    if(Line_angle < 0) Line_angle += 360;
    // Line_angle = simplify.FixLimit(Line_angle + 180);
    read = 1;
  }

  Last_overLine_flag = overLine_flag;
  Last_Line_angle = Line_angle;
  Last_read = read;

  return Line_angle;
}

float LINE_TEST::getDepth()
{
  if(read == 1) return DOTproduct_first;
  else return 999;
}

int LINE_TEST::isRead_Angel()
{
  return read;
}

int LINE_TEST::isHalfOver()
{
  return isOverHalf;
}

bool LINE_TEST::isRead_Line()
{
  return updated;
}