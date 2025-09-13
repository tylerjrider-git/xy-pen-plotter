#pragma once

#include <deque>
#include <mutex>
#include <atomic>
#include <thread>


// Super dumb command.
struct StepperCommand {
    double speed;
    double duration;
    bool direction;
};

// TODO virtualize to allow different stepper motor controllers.
class StepperController {
public:
    StepperController(int stepper_n);
    virtual ~StepperController();

    void sendCommand(struct StepperCommand cmd);
    void setTarget(float speed, bool dir);
    void step(double speed, bool direction, int angle=360);

private:
    double safeSpeed(double in); // TODO add in mechanical configuration

private:
     struct tmc2209_handle* mControllerHandle;
    int nStepper;
    std::thread mWorkerThread;
    std::atomic<bool> mRunning{true};

    std::mutex mCommandQueueMutex;
    std::deque<struct StepperCommand> mCommandQueue;

    std::atomic<float> mTargetSpeed;
    std::atomic<bool> mTargetDirection;
    std::chrono::time_point<std::chrono::steady_clock> mlastUpdateTp;
};
