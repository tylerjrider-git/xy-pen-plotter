#include "StepperController.h"

int main(int argc, char** argv)
{
    StepperController xAxis(0);
    // StepperThread yAxisThread(1);

    xAxis.sendCommand({1.0, 5.0, CW});
    xAxis.sendCommand({1.0, 5.0, CCW});

    // yAxisThread.sendCommand({1.0, 10.0, CW});
    // yAxisThread.sendCommand({1.0, 10.0, CCW});
    
    while (1) {
        sleep(1);
        fprintf(stderr, "Still running\n");
    }

}
