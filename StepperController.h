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
    void setTargetVelocity(float velocity);
    void resetOrigin();

    int stepperCount() const;
    float currentPosition() const;

private:
    void step(double velocity, unsigned int steps);
    bool stepValid(int step, bool direction) const;

    void handleCommand(struct StepperCommand& cmd);
    void handleTargetCommand();
    double safeSpeed(double in); // TODO add in mechanical configuration
    unsigned long timeSinceLastCommand() const;

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
