#include "StepperController.h"
#include "bgt_tmc2209.h" // TODO could be different driver "backends"

#include <algorithm> // clamp
#include <iostream>

using namespace std::chrono_literals;

static constexpr int MAX_TARGET_DURATION_MS = 2000;
static constexpr float DEFAULT_VELOCITY = 1.0; // 1 RPS

 //

// Step angle: 1.8deg
// uStep Mode: 1/8
// Belt Pitch:2mm
// Pulley tooth count: 20
// Steps/mm:40
static constexpr float STEPS_PER_MM = 40;

static inline float steps2mm(float steps)
{
    return steps/STEPS_PER_MM;
}
static inline float mm2steps(float mm)
{
    return STEPS_PER_MM*mm;
}

StepperController::StepperController(int stepper_n, int maxRange, StepperFlags flags):
    mStepperId(stepper_n),
    mCurrentPosition(0),
    mMaxRange(maxRange),
    mStepperFlags(flags),
    mTargetVelocity{0}
{
    int rc;

    mControllerHandle = tmc2209_create();
    if (mControllerHandle == nullptr) {
        std::cerr << "Failed to create driver handle" << std::endl;
        return; // TODO throw.
    }

    rc = tmc2209_init(mControllerHandle, mStepperId);
    if (rc != 0) {
        std::cerr <<  "Failed to init TCM controller for id: " << mStepperId << std::endl;
        return; // TODO throw.
    }

    mWorkerThread = std::thread([this]() -> void {
        while (mRunning) {
            std::unique_lock<std::mutex> lock(mCommandQueueMutex);
            mCommandCv.wait(lock, [&]() -> bool {
                return !mCommandQueue.empty() || std::abs(mTargetVelocity.load()) > TMC2209_MIN_SPEED;
            });

            if (mCommandQueue.empty() == false) {
                struct StepperCommand cmd = mCommandQueue.front();
                mCommandQueue.pop_front();
                lock.unlock();
                handleCommand(cmd);
                mTargetVelocity.store(0); // Disables target mode.
            } else if (std::abs(mTargetVelocity.load()) > TMC2209_MIN_SPEED) {
                handleTargetCommand();
            }
        }
    });
}

StepperController::~StepperController()
{
    mRunning = false;
    mWorkerThread.join();

    if (mControllerHandle) {
        tmc2209_enable(mControllerHandle, false);
        tmc2209_teardown(mControllerHandle);
        tmc2209_destroy(&mControllerHandle);
    }
}

void StepperController::sendCommand(struct StepperCommand cmd)
{
    std::lock_guard<std::mutex> lg(mCommandQueueMutex);
    mCommandQueue.push_back(cmd);
    mCommandCv.notify_all();
}

void StepperController::waitCommand()
{
    while (mCommandQueue.empty() == false) {
        std::this_thread::sleep_for(10ms);
    }
}

void StepperController::setTargetVelocity(float velocity)
{
    mlastUpdateTp = std::chrono::steady_clock::now();
    mTargetVelocity.store(velocity);
    mCommandCv.notify_all();
}

void StepperController::handleTargetCommand()
{
    // This is just a safety backstop to prevent the analog stick getting
    // stuck.
    if (timeSinceLastCommand() < MAX_TARGET_DURATION_MS) {
        bool dir = mTargetVelocity.load() > 0;
        int steps = tmc2209_degrees_to_steps(1);
        steps = dir ? steps : -steps;
        step(std::abs(mTargetVelocity.load()), steps);
    } else {
        std::cout << "Stopping autotarget" << std::endl;
        mTargetVelocity.store(0);
        mCommandCv.notify_all();
    }
}

unsigned long StepperController::timeSinceLastCommand() const
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - mlastUpdateTp).count();
}

void StepperController::handleCommand(struct StepperCommand &cmd)
{
    switch (cmd.type) {
    case StepperCommandType::Velocity:
    {   bool dir =  cmd.velocity >= 0;
        int steps = tmc2209_degrees_to_steps(360);
        steps = dir ? steps : -steps;
        step(std::abs(cmd.velocity), steps);
        break;
    }
    case StepperCommandType::RelativePosition:
    {
        const float steps = mm2steps(cmd.position);
        step(std::abs(cmd.velocity), steps);
        break;
    }

    case StepperCommandType::AbsolutePosition:
    {
        const float deltaSteps = mm2steps(cmd.position - mCurrentPosition);
        step(std::abs(cmd.velocity), deltaSteps);
        break;
    }

    default:
        fprintf(stderr, "Unhandled Command: %d\n", cmd.type);
        break;
    }
}

double StepperController::safeSpeed(double d) const
{
    return std::clamp(-TMC2209_MAX_SPEED, TMC2209_MAX_SPEED, d);
}

bool StepperController::stepValid(float mm) const
{
    return (mCurrentPosition + mm < mMaxRange) && (mCurrentPosition - mm > -mMaxRange);
}

void StepperController::step(double speed, int steps)
{
    const bool direction = steps > 0;
    // clamp velocity
    speed = safeSpeed(speed);

    // printf("step speed=%lf, steps=%d, mm=%f\n", speed, steps, steps2mm(steps));
    // Soft checks
    if (stepValid(steps2mm(steps)) == false) {
         fprintf(stderr, "Command of %d steps would exceed range %lf > %d\n",
            steps, mCurrentPosition + steps2mm(steps), mMaxRange);
        return;
    }


    // if LIMIT SWITCH == false
    // TODO
    // * Add limit switches before issuing command.
    // * Perhaps run PID here as well.
    tmc2209_enable(mControllerHandle, true);
    tmc2209_setdir(mControllerHandle, direction);
    tmc2209_step(mControllerHandle, std::abs(steps), speed);

    mCurrentPosition += steps2mm(steps);

    // De-energizing the stepper when in microstepper mode can result
    // into motor sliding into full step.
    if (!(mStepperFlags & StepperFlags::ContinousDrive))
        tmc2209_enable(mControllerHandle, false);
}


float StepperController::currentPosition() const
{
    return mCurrentPosition;
}

void StepperController::resetOrigin()
{
    mCurrentPosition = 0;
}
