#pragma once
#include <Arduino.h>
#include <IntervalTimer.h>

class ESCControl {
public:
    enum class State {
        CALIB_HIGH,
        CALIB_NEUTRAL,
        READY
    };

    explicit ESCControl(uint8_t pin);

    void begin();
    void update();

    void setPulseUs(int us);
    int  getPulseUs() const;
    bool isReady() const;
    State getState() const;

private:
    static constexpr uint32_t pulsePeriodUs = 20000; // 20ms

    uint8_t escPin;
    volatile int currentPulseUs;

    State state;
    unsigned long stateStartMillis;

    // ---- タイマ関連 ----
    static ESCControl* instance;
    IntervalTimer riseTimer;
    IntervalTimer fallTimer;

    static void riseISR();
    static void fallISR();

    void handleRise();
    void handleFall();

    void updateState();
    int getTargetPulseForCurrentState() const;
};