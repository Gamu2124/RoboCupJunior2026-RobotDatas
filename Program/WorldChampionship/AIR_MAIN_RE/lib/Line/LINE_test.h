#pragma once
#include <Arduino.h>
#include <simplify.h>
#include <TIMER.h>

class LINE_TEST  //クラスLINEで草 俺もクラスLINEに入れてくれよ...
{
private:
    // const int Line_sensor[16] = {A14,A12,A10,A8,A6,A4,A2,A0,A1,A3,A5,A7,A9,A11,A13,A15}; //ここに値を入れないと、結果帰ってこないよ
    // const int Line_sensor[16] = {A0,A2,A4,A6,A8,A10,A12,A14,A1,A3,A5,A7,A9,A11,A13,A15}; 
    // const int multi_analog[3] = {7,8,9};
    // const int multi_select[3] = {33,34,35}; 
    TIMER timer_overhalf;
    int bacKLine_flag = 0;
    int Last_overLine_flag = 0;  
    int overLine_flag = 0;
    int Last_read = 0;
    int read = 0;   
    int count_num_side = 0;;
    float Last_vec_side[2];
    float Line_side_vec[2];
    int Line_range_num[5][2];
    double Line_vec[5][2];  // 初期化
    int isLineContinue = 0;
    int Line_flag_count = 0;
    int val_count[5];
    int Line_on = 0;
    float Last_Line_angle = 0;
    float LINE_X[32] = {
    1.0, 0.9808, 0.9239, 0.8315, 0.7071, 0.5556, 0.3827, 0.1951,
    0.0, -0.1951, -0.3827, -0.5556, -0.7071, -0.8315, -0.9239, -0.9808,
    -1.0, -0.9808, -0.9239, -0.8315, -0.7071, -0.5556, -0.3827, -0.1951,
    0.0, 0.1951, 0.3827, 0.5556, 0.7071, 0.8315, 0.9239, 0.9808
    };

    float LINE_Y[32] = {
    0.0, 0.1951, 0.3827, 0.5556, 0.7071, 0.5556, 0.9239, 0.9808,
    1.0, 0.9808, 0.9239, 0.8315, 0.7071, 0.5556, 0.3827, 0.1951,
    0.0, -0.1951, -0.3827, -0.5556, -0.7071, -0.8315, -0.9239, -0.9808,
    -1.0, -0.9808, -0.9239, -0.8315, -0.7071, -0.5556, -0.3827, -0.1951
    };
    float LINE_SIDE_X[4] = {0.0, 0.0, -1.0, 1.0}; //左、右、後、前 DC
    float LINE_SIDE_Y[4] = {-1.0, 1.0, 0.0, 0.0}; //前、右、後、左 IR
    SIMPLIFY simplify;
    int overhalf_flag = 0;
    int Last_overhalf_flag = 0;
    int Line_side_val[4];
    int Line_side_on = 0;
    int isOverHalf = 0;
    int Line_vals[32];
    int side_vals[4];
    int grid_vals[3];  // L, R, B
    int selected_role = 0;
    float main_finalvec[2];
    bool updated = false;

    float DOTproduct_first = 0;
    float DOTproduct_last = 0;
    

public:
    LINE_TEST();
    void begin(unsigned long baud);
    void updateSensor();
    void vec(float* vec);
    float angle(float* veec, float* veeec);
    void side_vec(float* vec);
    double Line_angle;
    double Last_vec[2] = {0,0};
    double First_vec[2] = {0,0};
    int isRead_Angel();
    int isHalfOver();
    float getDepth();
    void setRaw(int role);
    bool isRead_Line();
};