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

private:
    void step(double speed, bool direction, int angle=360);
    void handleCommand(struct StepperCommand& cmd);
    void handleTargetCommand();
    double safeSpeed(double in); // TODO add in mechanical configuration
    unsigned long timeSinceLastCommand();

private:
    // HW Specific
    struct tmc2209_handle* mControllerHandle;
    int mStepperId;

    // Worker thread info
    std::thread mWorkerThread;
    std::atomic<bool> mRunning{true};

    // Command mode.
    std::mutex mCommandQueueMutex;
    std::deque<struct StepperCommand> mCommandQueue;

    // Open mode
    std::atomic<float> mTargetSpeed;
    std::atomic<bool> mTargetDirection;
    std::chrono::time_point<std::chrono::steady_clock> mlastUpdateTp;
};
