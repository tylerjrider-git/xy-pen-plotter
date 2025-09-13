#include "StepperController.h"
#include "bgt_tmc2209.h" // TODO could be different driver "backends"
#include <algorithm> // clamp

using namespace std::chrono_literals;

StepperController::StepperController(int stepper_n):
    nStepper(stepper_n)
{
    mTargetSpeed.store(0);
    mControllerHandle = (struct tmc2209_handle*)::malloc(sizeof(struct tmc2209_handle));
    mControllerHandle->nStep = stepper_n; // TODO map to correct impl
    int rc = tmc2209_init(mControllerHandle);
    if (rc != 0) {
        std::fprintf(stderr, "Failed to init tcm controller\n");
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
                    while (cmd.duration > 0) {
                        step(cmd.speed, cmd.direction);
                        cmd.duration = cmd.duration - 1;// TODO.
                        std::printf("%d duration : %lf\n", nStepper, cmd.duration);
                    }
                mTargetSpeed.store(0); // clear targets.
                } else if (mTargetSpeed > TMC2209_MIN_SPEED) {
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - mlastUpdateTp);
                    step(mTargetSpeed, mTargetDirection, 1);
                    if (duration.count() > 2000) {
                        mTargetSpeed.store(0);
                        printf("Stopping auto run\n");
                    }
                    continue;
                } else {
                    mTargetSpeed.store(0);
                }
            }
            std::this_thread::sleep_for(100ms);
        }
    });
}

StepperController::~StepperController()
{
    mRunning = false;
    mWorkerThread.join();

    if (mControllerHandle)
        ::free(mControllerHandle);
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

void StepperController::step(double speed, bool direction, int angle)
{
    speed = safeSpeed(speed);
    std::fprintf(stderr, "%d: Stepping at %lf dir: %d\n", nStepper, speed, direction);
    // TODO
    // * Add limit switches before issuing command.
    // * Perhaps run PID here as well.
    // DO Step.
    tmc2209_enable(mControllerHandle, true);
    tmc2209_setdir(mControllerHandle, direction);
    tmc2209_angle_step(mControllerHandle, angle, speed);
    tmc2209_enable(mControllerHandle, false);
}

double StepperController::safeSpeed(double d)
{
    return std::clamp(0.0, TMC2209_MAX_SPEED, d);
}

