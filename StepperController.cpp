#include "StepperController.h"
#include "bgt_tmc2209.h" // TODO could be different driver "backends"

#include <algorithm> // clamp
#include <iostream>

using namespace std::chrono_literals;

static constexpr int MAX_TARGET_DURATION_MS = 2000;

StepperController::StepperController(int stepper_n):
    mStepperId(stepper_n),
    mTargetSpeed{0}
{
    mControllerHandle = tmc2209_create();
    int rc = tmc2209_init(mControllerHandle, mStepperId);
    if (rc != 0) {
        std::cerr <<  "Failed to init TCM controller for id: " << mStepperId << std::endl;
        return; // TODO throw.
    }

    mWorkerThread = std::thread([this]() -> void {
        while (mRunning) {
            {
                std::unique_lock<std::mutex> lock(mCommandQueueMutex);
                if (mCommandQueue.empty() == false) {
                    struct StepperCommand cmd = mCommandQueue.front();
                    mCommandQueue.pop_front();
                    lock.unlock();
                    handleCommand(cmd);
                    mTargetSpeed.store(0); // Disables target mode.
                } else if (mTargetSpeed > TMC2209_MIN_SPEED) {
                    handleTargetCommand();
                } else {
                    // No commands or active target, wait. TODO cv.wait(std::unique_lock<mMutex>, [] { !mCommandQueue.empty()})
                    std::this_thread::sleep_for(100ms);
                    mTargetSpeed.store(0);
                }
            }
        }
    });
}

StepperController::~StepperController()
{
    mRunning = false;
    mWorkerThread.join();

    if (mControllerHandle) {
        tmc2209_teardown(mControllerHandle);
        tmc2209_destroy(&mControllerHandle);
    }
}

void StepperController::sendCommand(struct StepperCommand cmd)
{
    std::lock_guard<std::mutex> lg(mCommandQueueMutex);
    mCommandQueue.push_back(cmd);
}

void StepperController::setTarget(float speed, bool dir)
{
    mlastUpdateTp = std::chrono::steady_clock::now();
    mTargetSpeed.store(speed);
    mTargetDirection.store(dir);
}

unsigned long StepperController::timeSinceLastCommand()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - mlastUpdateTp).count();
}

void StepperController::handleTargetCommand()
{
    if (timeSinceLastCommand() < MAX_TARGET_DURATION_MS) {
        step(mTargetSpeed, mTargetDirection, 1);
    } else {
        std::cout << "Stopping autotarget" << std::endl;
        mTargetSpeed.store(0);
    }
}

void StepperController::handleCommand(struct StepperCommand& cmd)
{
    while (cmd.duration > 0) {
        step(cmd.speed, cmd.direction);
        cmd.duration = cmd.duration - 1;// TODO calculate duration based on cmd.
        std::printf("[%d] duration : %lf\n", mStepperId, cmd.duration);
    }
}

double StepperController::safeSpeed(double d)
{
    return std::clamp(0.0, TMC2209_MAX_SPEED, d);
}

void StepperController::step(double speed, bool direction, int angle)
{
    speed = safeSpeed(speed);
    std::fprintf(stderr, "%d: Stepping at %lf dir: %d\n", mStepperId, speed, direction);
    // TODO
    // * Add limit switches before issuing command.
    // * Perhaps run PID here as well.
    tmc2209_enable(mControllerHandle, true);
    tmc2209_setdir(mControllerHandle, direction);
    tmc2209_angle_step(mControllerHandle, angle, speed);
    tmc2209_enable(mControllerHandle, false);
}

