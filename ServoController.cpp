#include "ServoController.h"
#include "XboxController.h"
#include <unistd.h>  // for sleep

const std::string& PWM_CHIP_PATH = "/sys/class/pwm/pwmchip0";

template <typename T>
float lerp(T a, T b, float t)
{
    return static_cast<float>(a + t*(b-a));
}

PwmController::PwmController(const std::string& pwmChipPath,
                             unsigned int device) :
                             mDutyCycle(MIN_PWM_DUTY_CYCLE),
                             mPeriod(PWM_PERIOD)
{
    mDevicePath = pwmChipPath + "/pwm" + std::to_string(device);
    // TODO error check
    writeToFile(pwmChipPath + "/export", std::to_string(device));

    enable(false);
    setPeriod(PWM_PERIOD);
    setDutyCycle(MIN_PWM_DUTY_CYCLE);
    enable(true);
}

PwmController::~PwmController()
{
    enable(false);
}

// Helper to write to sysfs
void PwmController::writeToFile(const std::string& path, const std::string& value)
{
    std::ofstream fs(path);
    if (!fs.is_open()) {
        std::cerr << "Error: cannot open " << path << std::endl;
        return;
    }
    fs << value;
    fs.close();
}

void PwmController::setPeriod(unsigned long period_ns)
{
    writeToFile(mDevicePath + "/period", std::to_string(period_ns));
    mPeriod = period_ns;
}

void PwmController::setDutyCycle(unsigned long duty_cycle_ns)
{
    writeToFile(mDevicePath + "/duty_cycle", std::to_string(duty_cycle_ns));
    mDutyCycle = duty_cycle_ns;
}

void PwmController::enable(bool enable)
{
    writeToFile(mDevicePath + "/enable", "1");
}

ServoController::ServoController()
    : mPwmController(PWM_CHIP_PATH, 0)
{
    mPwmController.setPeriod(PWM_PERIOD);
    mPwmController.enable(true);
}

ServoController::~ServoController()
{
    mPwmController.enable(false);
}

float ServoController::safeAngle(float f) const
{
    return std::clamp(f, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
}

unsigned long ServoController::angle2duty(float angle) const
{
    return lerp(MIN_PWM_DUTY_CYCLE, MAX_PWM_DUTY_CYCLE, angle/SERVO_MAX_ANGLE);
}

float ServoController::duty2angle(unsigned long dutyCycle) const
{
    return lerp(SERVO_MIN_ANGLE, SERVO_MAX_ANGLE, (dutyCycle - MIN_PWM_DUTY_CYCLE) / (MAX_PWM_DUTY_CYCLE - MIN_PWM_DUTY_CYCLE));
}

void ServoController::setAngle(float f)
{
    unsigned int duty_cycle_ns = angle2duty(safeAngle(f));
    duty_cycle_ns = std::clamp(duty_cycle_ns, MIN_PWM_DUTY_CYCLE, MAX_PWM_DUTY_CYCLE);
    mPwmController.setDutyCycle(duty_cycle_ns);
}

float ServoController::currentAngle() const
{
    return duty2angle(mPwmController.dutyCycle());
}

//
// Test app with XboXController
//
const float SERVO_MID_ANGLE = SERVO_MAX_ANGLE / 2; // 45.f
static inline float axis2angle(const int axis)
{
     // 0->0, X_AXIS_MAX -> 1.0.   X_AXIS_MIN -> 0, 0 -> 0.5, X_AXIS_MAX -> 1
    const float t = (float)(axis - X_AXIS_MIN) / (float)(2*X_AXIS_MAX);
    return SERVO_MID_ANGLE + lerp(-SERVO_MID_ANGLE, SERVO_MID_ANGLE, t);
}

__attribute__((weak)) int main(int argc, char** argv)
{
    float angle = 45;             // default
    ServoController servo;

    // Control loop
    XboxController xbox("/dev/input/event5");
    xbox.startEventThread();
    while (true) {
        auto ret  = xbox.getNextEvent();
        if (ret) {
            XboxEvent ev = ret.value();
            if (ev.type == XboxEvent::LeftAxis) {
                angle = axis2angle(ev.xpos);
                servo.setAngle(angle);
                std::cout << "Controller pos: " << ev.xpos << " -> angle: " << angle << std::endl;
            }
        } else {
            usleep(1000); // update every second
        }

    }

    return 0;
}
