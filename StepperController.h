#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>
#include <atomic>
#include <thread>

// Super dumb command.
enum StepperCommandType : uint8_t {
    Velocity = 0,
    AbsolutePosition = 1,
    RelativePosition = 2
};

struct StepperCommand {
    StepperCommandType type;
    double velocity; // rps?
    double position; // mm, lets go with mm.
};

enum StepperFlags {
    ContinousDrive = (1 << 0),
};

// TODO virtualize to allow different stepper motor controllers.
class StepperController {
public:
    StepperController(int stepper_n, int maxRange, StepperFlags flags);
    virtual ~StepperController();

    void sendCommand(struct StepperCommand cmd);
    void waitCommand();
    void setTargetVelocity(float velocity);

    void resetOrigin();
    float currentPosition() const;

private:
    bool stepValid(float mm) const;
    double safeSpeed(double in) const;
    unsigned long timeSinceLastCommand() const;

    void handleCommand(struct StepperCommand& cmd);
    void handleTargetCommand();
    void step(double speed, int steps);

private:
    // HW Specific
    struct tmc2209_handle* mControllerHandle;
    int mStepperId;
    int mStepperCount;
    float mCurrentPosition;
    const int mMaxRange;

    StepperFlags mStepperFlags;
    // Worker thread info
    std::thread mWorkerThread;
    std::atomic<bool> mRunning{true};

    // Command mode.
    std::mutex mCommandQueueMutex;
    std::deque<struct StepperCommand> mCommandQueue;
    std::condition_variable mCommandCv;

    // Open mode
    std::atomic<float> mTargetVelocity;
    std::chrono::time_point<std::chrono::steady_clock> mlastUpdateTp;
};
