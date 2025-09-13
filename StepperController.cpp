#include "StepperController.h"
#include "bgt_tcm2209.h" // TODO could be different driver "backends"
#include <algorithm> // clamp

using namespace std::chrono_literals;

StepperController::StepperController(int stepper_n):
    nStepper(stepper_n)
{
    mControllerHandle = (struct tcm2209_handle*)::malloc(sizeof(struct tcm2209_handle));
    mControllerHandle->nStep = stepper_n; // TODO map to correct impl
    int rc = tcm2209_init(mControllerHandle);
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

void StepperController::step(double speed, bool direction)
{
    std::printf("%d: Stepping at %lf dir: %d\n", nStepper, speed, direction);
    // TODO
    // * Add limit switches before issuing command.
    // * Perhaps run PID here as well.
    // DO Step.
    tcm2209_enable(mControllerHandle, true);
    tcm2209_setdir(mControllerHandle, direction);
    tcm2209_angle_step(mControllerHandle, 360, speed);
    tcm2209_enable(mControllerHandle, false);
}

double StepperController::safeSpeed(double d)
{
    return std::clamp(0.0, TCM2209_MAX_SPEED, d);
}



