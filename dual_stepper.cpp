
#include <atomic>
#include <thread>
#include <unistd.h>
#include <chrono>

using namespace std::chrono_literals;

constexpr bool CCW = true;
// constexpr bool CW = false;

class StepperThread{
public:
    StepperThread(int stepper_n);
    virtual ~StepperThread();
    void workFunc();
private:
    int nStepper;
    std::thread mWorkerThread;
    std::atomic<double> mTargetSpeed;
    std::atomic<bool> mTargetDirection;
    std::atomic<bool> mRunning{true};
};

StepperThread::StepperThread(int stepper_n):
    nStepper(stepper_n)
{
    mTargetSpeed.store(0);
    mTargetDirection.store(CCW);

    mWorkerThread = std::thread([this]() -> void {
        while (mRunning) {
            std::this_thread::sleep_for(1s);

            std::printf("[%d] Stepping at %lf speed, in %s direction\n",
                nStepper, mTargetSpeed.load(), mTargetDirection.load() ? "CCW" : "CW");

        }
    });
}
StepperThread::~StepperThread()
{
        mRunning = false;
        mWorkerThread.join();
}

void StepperThread::workFunc()
{

}


int main(int argc, char** argv) {


    StepperThread xAxisThread(0);
    StepperThread yAxisThread(1);

    while (1) {
        sleep(1);
        fprintf(stderr, "Still running\n");
    }

}
