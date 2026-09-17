#pragma once
#include <Arduino.h>
#include <MA.h>
#include <simplify.h>
#include <TIMER.h>

class BALL
{
private:
    //ボールセンサー
    int ball_bug_flag = 0;
    int Last_get_Ball_angle = 0;
    int Last_get_Ball_far = 0;
    int Ball_angle = 0;
    int get_Ball_far = 0;
    float get_Ball_far_ave = 0;
    byte Last_receivedBuf[6];
    byte receivedBuf[6];

    static constexpr float BALL_VEC_X[16] = {
    1.000,  0.924,  0.707,  0.383,
    0.000, -0.383, -0.707, -0.924,
    -1.000, -0.924, -0.707, -0.383,
    0.000,  0.383,  0.707,  0.924
    };

    static constexpr float BALL_VEC_Y[16] = {
    0.000,  0.383,  0.707,  0.924,
    1.000,  0.924,  0.707,  0.383,
    0.000, -0.383, -0.707, -0.924,
    -1.000, -0.924, -0.707, -0.383
    };

    float Ball_Diss;
    float x = 0;
    float Last_x = 0;
    double dt = 0;
    double pre_time = 0;
    int way = 0; //1右、2左
    float originaly_ang;
    int isOverHalf;
    float Last_far = 0;
    float Last_vxy[2];

    // 補足
    const int catch_photo_pin = A8;
    int catch_val = 0;
    int MAX_CATCHVAL = 0;
    const int KICK_VAL = 20;
    int isHold = 0;
    float isHold_ave = 0;
    int isBall_catch = 0;
    int wait_catch = 0;

    MA moveave_hold;
    MA moveave_far;
    MA moveave_ballhold;
    SIMPLIFY simplify;
    TIMER timer_pd;
    TIMER timer_no_hold;
    TIMER timer_wait_hold;

public: 
    void begin(unsigned long baud = 115200);
    bool updateBallSensor();           // serialEvent3 相当
    int  getAngle() const;
    int  getFar()   const;
    std::pair<float, int> around(int, float, int, int, int);
    double get_DirAdder(float);
    double calc_dt();
    void updateCatch(float);
    int isCaught() const;
    int whichway();
    int catch_sent();
    int Kick_val_sent() const;
    float vec_speed(float,int);
    // --- パケット受信・フラグ関係 ---
    uint8_t rxBuf[4];        // 4バイトパケット受信バッファ
    int rxIndex;             // バッファの現在のインデックス
    volatile bool ballFound; // ボール発見フラグ

    // --- ボールデータ直値格納 ---
    volatile int Ball_dist;  // ボールの生距離 (元の smoothedDist 相当)

    // --- 姿勢制御用 アダプティブ角度フィルタ（EMA）用変数 ---
    float filteredAttitude;  // フィルタ後の姿勢角度
    bool attitudeInit;       // 姿勢フィルタ初期化フラグ

    // --- 回り込み角度（GoPM）チャタリング防止用変数 ---
    float lastGoPM;          // 前回のGoPM角度
    bool hasLastGoPM;        // 前回値が存在するかどうかのフラグ
};