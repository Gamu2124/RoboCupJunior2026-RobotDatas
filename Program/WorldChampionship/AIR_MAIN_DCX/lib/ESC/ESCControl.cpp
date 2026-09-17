#include "ESCControl.h"

ESCControl* ESCControl::instance = nullptr;

ESCControl::ESCControl(uint8_t pin)
    : escPin(pin),
      currentPulseUs(1400),
      state(State::CALIB_HIGH),
      stateStartMillis(0) {}

void ESCControl::begin() {
    pinMode(escPin, OUTPUT);
    digitalWrite(escPin, LOW);

    state = State::CALIB_HIGH;
    stateStartMillis = millis();

    instance = this;

    // 20msごとに立ち上がり処理
    riseTimer.begin(riseISR, pulsePeriodUs);
}

void ESCControl::update() {
    updateState();
}

void ESCControl::setPulseUs(int us) {
    if (us < 1000) us = 1000;
    if (us > 2000) us = 2000;
    currentPulseUs = us;
}

int ESCControl::getPulseUs() const {
    return currentPulseUs;
}

bool ESCControl::isReady() const {
    return state == State::READY;
}

ESCControl::State ESCControl::getState() const {
    return state;
}

void ESCControl::updateState() {
    unsigned long now = millis();

    switch (state) {
        case State::CALIB_HIGH:
            if (now - stateStartMillis >= 3000) {
                state = State::CALIB_NEUTRAL;
                stateStartMillis = now;
            }
            break;

        case State::CALIB_NEUTRAL:
            if (now - stateStartMillis >= 4000) {
                state = State::READY;
                stateStartMillis = now;
            }
            break;

        case State::READY:
            break;
    }
}

int ESCControl::getTargetPulseForCurrentState() const {
    switch (state) {
        case State::CALIB_HIGH:
            return 2000;
        case State::CALIB_NEUTRAL:
            return 1500;
        case State::READY:
            return currentPulseUs;
    }
    return 1500;
}

void ESCControl::riseISR() {
    if (instance) instance->handleRise();
}

void ESCControl::fallISR() {
    if (instance) instance->handleFall();
}

void ESCControl::handleRise() {
    digitalWrite(escPin, HIGH);

    int pulse = getTargetPulseForCurrentState();
    fallTimer.begin(fallISR, pulse);   // pulse us 後に LOW にする
}

void ESCControl::handleFall() {
    digitalWrite(escPin, LOW);
    fallTimer.end(); // 次のriseまで待つ
}