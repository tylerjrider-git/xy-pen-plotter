#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm> // clamp

// characteristics of servo motor
const unsigned int PWM_PERIOD = 20000000;     // 5 ms = 200 Hz
const unsigned int MIN_PWM_DUTY_CYCLE = 500000;     // 0.5 ms
const unsigned int MAX_PWM_DUTY_CYCLE = 2500000;    // 2.5 ms
const float SERVO_MIN_ANGLE = 0;     // 0.5 ms
const float SERVO_MAX_ANGLE = 90;    // 2.5 ms

class PwmController {
public:
    PwmController(const std::string& pwmchip, unsigned int device);
    virtual ~PwmController();

    void setPeriod(unsigned long period_ns);
    void setDutyCycle(unsigned long duty_cycle_ns);
    void enable(bool enable);

    unsigned long dutyCycle() const { return mDutyCycle; }
private:
    void writeToFile(const std::string& path, const std::string& val);

private:
    std::string mDevicePath;
    uint64_t mDutyCycle;
    uint64_t mPeriod;
};

class ServoController {
public:
    ServoController();
    virtual ~ServoController();

    void setAngle(float angle);
    float currentAngle(void) const;
private:
    unsigned long angle2duty(float angle) const;
    float duty2angle(unsigned long dutyCycle) const;
    float safeAngle(float f) const;

private:
    PwmController mPwmController;
};

