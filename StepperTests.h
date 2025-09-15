#pragma once
#include "StepperController.h"
#include <chrono>
#include <vector>
#include <cmath>

using Coordinate = std::pair<float,float>;
using namespace std::chrono_literals;

static void testOriginReset(StepperController& xAxis, StepperController& yAxis)
{
    xAxis.sendCommand({StepperCommandType::AbsolutePosition, 1.0, -10});
    yAxis.sendCommand({StepperCommandType::AbsolutePosition, 1.0, -10});

    xAxis.sendCommand({StepperCommandType::RelativePosition, 1.0, 15});
    yAxis.sendCommand({StepperCommandType::RelativePosition, 1.0, 15});

    xAxis.sendCommand({StepperCommandType::AbsolutePosition, 1.0, 0});
    yAxis.sendCommand({StepperCommandType::AbsolutePosition, 1.0, 0});
}

static void testCoordinates(StepperController& xAxis, StepperController& yAxis)
{
    std::vector<Coordinate> coordinates = {
            {-5, 0},
            {0 , -1},
            {4.0, 0},
            {0, -10},
            {2.0, 0},
            {0,  10.0},
            {4.0, 0},
            {0, 1.0},
            {-5, 0}
    };
    for (auto coordinate : coordinates) {
        float x = coordinate.first;
        float y = coordinate.second;
        xAxis.sendCommand({StepperCommandType::RelativePosition, 5.0, x});
        yAxis.sendCommand({StepperCommandType::RelativePosition, 5.0, y});
        std::this_thread::sleep_for(100ms);
    }
}


static void testArcs(StepperController& xAxis, StepperController& yAxis)
{
    for (float t = 0; t < 3; ) {
        xAxis.setTargetVelocity(sin(2*3.14159*t));
        yAxis.setTargetVelocity(cos(2*3.14159*t));
        t += 0.010;
        std::this_thread::sleep_for(10ms);
    }
    yAxis.setTargetVelocity(0);
    xAxis.setTargetVelocity(0);
}