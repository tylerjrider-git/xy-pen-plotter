#pragma once

#include <deque>
#include <mutex>
#include <atomic>
#include <thread>

constexpr bool CCW = true;
constexpr bool CW = false;

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
    void step(double speed, bool direction);

private:
    double safeSpeed(double in); // TODO add in mechanical configuration

private:
     struct tcm2209_handle* mControllerHandle;
    int nStepper;
    std::thread mWorkerThread;
    std::atomic<bool> mRunning{true};

    std::mutex mCommandQueueMutex;
    std::deque<struct StepperCommand> mCommandQueue;
};
