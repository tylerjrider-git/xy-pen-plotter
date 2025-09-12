#include <deque>
#include <mutex>
#include <atomic>
#include <thread>
#include <unistd.h>
#include <chrono>
#include <algorithm>  // clamp
#include "bgt_tcm2209.h"


using namespace std::chrono_literals;


struct StepperCommand {
    double speed;
    double duration;
    bool direction;
};

class StepperThread{
public:
    StepperThread(int stepper_n);
    virtual ~StepperThread();

    void sendCommand(struct StepperCommand cmd);
    void step(double speed, bool direction);

private:
    double safeSpeed(double in);

private:
     struct tcm2209_handle* mControllerHandle;
    int nStepper;
    std::thread mWorkerThread;
    std::atomic<bool> mRunning{true};

    std::mutex mCommandQueueMutex;
    std::deque<struct StepperCommand> mCommandQueue;
};

StepperThread::StepperThread(int stepper_n):
    nStepper(stepper_n)
{
    mControllerHandle = (struct tcm2209_handle*)::malloc(sizeof(struct tcm2209_handle));
    mControllerHandle->nStep = stepper_n; // TODO;
    int rc = tcm2209_init(mControllerHandle);
    if (rc != 0) {
        fprintf(stderr, "Failed to init tcm controller\n");
        return;
    }
    tcm2209_enable(mControllerHandle, true);
    tcm2209_setdir(mControllerHandle, CCW);
    tcm2209_angle_step(mControllerHandle, 360, 1.0);

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
                        printf("%d duration : %lf\n", nStepper, cmd.duration);
                    }
                }
            }
            std::this_thread::sleep_for(1s);
        }
    });
}

StepperThread::~StepperThread()
{
    mRunning = false;
    mWorkerThread.join();

    tcm2209_enable(mControllerHandle, false);
    free(mControllerHandle);
}

void StepperThread::sendCommand(struct StepperCommand cmd)
{
    std::lock_guard<std::mutex> lg(mCommandQueueMutex);
    mCommandQueue.push_back(cmd);
}

void StepperThread::step(double speed, bool direction)
{
    std::printf("%d: Stepping at %lf dir: %d\n", nStepper, speed, direction);
    // DO Step.
    tcm2209_setdir(mControllerHandle, direction);
    tcm2209_angle_step(mControllerHandle, 360, speed);
}

double StepperThread::safeSpeed(double d) {
    return std::clamp(0.0, TCM2209_MAX_SPEED, d);
}



int main(int argc, char** argv) {


    StepperThread xAxisThread(0);
    // StepperThread yAxisThread(1);

    xAxisThread.sendCommand({1.0, 5.0, CW});
    xAxisThread.sendCommand({1.0, 5.0, CCW});

    // yAxisThread.sendCommand({1.0, 10.0, CW});
    // yAxisThread.sendCommand({1.0, 10.0, CCW});
    
    while (1) {
        sleep(1);
        fprintf(stderr, "Still running\n");
    }

}
