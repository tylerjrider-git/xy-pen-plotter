#include "StepperController.h"
#include "XboxController.h"
#include "XYPenPlotter.h"
#include "StepperTests.h"

#include <cmath>
#include <csignal>
#include <algorithm>
#include <vector>
#include <tuple>

#include <unistd.h>
using namespace std::chrono_literals;

// MIN = -23148
static constexpr float MAX_SPEED = 5.0;

static inline float axis2velocity(int axis)
{
    axis = std::clamp(axis, X_AXIS_MIN, X_AXIS_MAX);
    return MAX_SPEED * ((float)axis / (float)X_AXIS_MAX);
}

static void handleEvent(XYPenPlotter& plotter, const XboxEvent &ev)
{
    switch (ev.type)
    {
    case XboxEvent::Button:
    {
        switch (ev.button)
        {
        case XboxButton::A:
            plotter.m_xAxis.sendCommand({StepperCommandType::Velocity, 2.0});
            break;
        case XboxButton::Y:
            plotter.m_xAxis.sendCommand({StepperCommandType::Velocity, -2.0});
            break;
        case XboxButton::X:
            plotter.m_yAxis.sendCommand({StepperCommandType::Velocity, -2.0});
            break;
        case XboxButton::B:
            plotter.m_yAxis.sendCommand({StepperCommandType::Velocity, 2.0});
            break;
        case XboxButton::START:
            plotter.m_xAxis.sendCommand({StepperCommandType::AbsolutePosition, 4.0, 0.0});
            plotter.m_yAxis.sendCommand({StepperCommandType::AbsolutePosition, 4.0, 0.0});
            break;
        case XboxButton::SELECT:
            plotter.m_xAxis.resetOrigin();
            plotter.m_yAxis.resetOrigin();
            break;
        case XboxButton::LBUMPER:
            plotter.drawArc(plotter.currentPosition(), 10,0,360, true);
            break;
        default:
            std::fprintf(stderr, "Unknown button %d, exitting\n", (int)ev.button);
            exit(1);
            break;
        }
    }
    break;
    case XboxEvent::LeftAxis:
    {
        float x_v = axis2velocity(ev.ypos);
        float y_v = axis2velocity(ev.xpos);
        // std::fprintf(stderr, "LAxis, val = (%lf, %lf)", ev.xpos, ev.ypos);
        // std::fprintf(stderr, "Speed: %lf\n", velocity);
        plotter.m_xAxis.setTargetVelocity(x_v);
        plotter.m_yAxis.setTargetVelocity(y_v);
        break;
    }
    case XboxEvent::RightAxis:
    {
        float x_v = axis2velocity(ev.ypos) / 3.0;
        float y_v = axis2velocity(ev.xpos) / 3.0;
        // std::fprintf(stderr, "LAxis, val = (%lf, %lf)", ev.xpos, ev.ypos);
        // std::fprintf(stderr, "Speed: %lf\n", velocity);
        plotter.m_xAxis.setTargetVelocity(x_v);
        plotter.m_yAxis.setTargetVelocity(y_v);
        break;
    }
    default:
        fprintf(stderr, "Unhandled event\n");
        break;
    }
    return;
}


static volatile bool running = true;
static void controllerLoop(XYPenPlotter& plotter, const std::string& controllerDevice)
{
    // TODO args
    XboxController controller(controllerDevice);
    static int tick = 0;
    controller.startEventThread();

    while (running) {
        std::optional<XboxEvent> ret = controller.getNextEvent();
        if (ret) {
            handleEvent(plotter, ret.value());
            // Keep draining event queue while its active(in the case of analog stick.)
            continue;
        }
        if ((tick++ % 1000) == 0) {
            Coordinate pos = plotter.currentPosition();
            printf("{%.02f,%.02f}\n", pos.x, pos.y);
        }
        std::this_thread::sleep_for(1ms);
    }
}

int main(int argc, char **argv)
{
    XYPenPlotter plotter;
    std::string controllerDevice = "/dev/input/event5";

    if (argc > 1) {
        if (std::string(argv[1]) == "Text") {
            testCoordinates(plotter);
        } else if (std::string(argv[1]) == "Origin") {
            testOriginReset(plotter);
        } else if (std::string(argv[1]) == "arcs") {
            testArcs(plotter);
        } else if (std::string(argv[1]) == "Circle") {
            testCircles(plotter);
        } else if (std::string(argv[1]) == "Symbol") {
            testSymbol(plotter);
        } else if (std::string(argv[1]) == "SymbolRaw") {
            testSymbolRaw(plotter);
        }
    }
    if (argc > 2) {
        if (std::string(argv[1]) == "--xbox") {
            controllerDevice = std::string(argv[2]);
            std::printf("Using %s for input device\n", controllerDevice.c_str());
        }
    }
    controllerLoop(plotter, controllerDevice);
    return 0;
}
