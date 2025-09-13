#include "StepperController.h"
#include "XboxController.h"
#include <unistd.h>
#include <algorithm>

// MIN = -23148
const int X_AXIS_MIN = INT16_MIN;
const int X_AXIS_MAX = INT16_MAX;
const float MAX_SPEED = 5.0;

static inline float axis2speed(int axis)
{
    axis = std::clamp(axis, X_AXIS_MIN, X_AXIS_MAX);
    return MAX_SPEED * ((float)std::abs(axis) / (float)X_AXIS_MAX);
}

void handleEvent(const XboxEvent &ev, StepperController &controller)
{
    switch (ev.type)
    {
    case XboxEvent::Button:
    {
        switch (ev.button)
        {
        case XboxButton::A:
            controller.sendCommand({0.5, 1.0, true});
            break;
        case XboxButton::B:
            controller.sendCommand({0.5, 1.0, false});
            break;
        case XboxButton::X:
            controller.sendCommand({2.0, 1.0, true});
            break;
        case XboxButton::Y:
            controller.sendCommand({2.0, 1.0, false});
            break;
        default:
            std::fprintf(stderr, "Unknown button %d\n", ev.button);
            break;
        }
    }
    break;
    case XboxEvent::LeftAxis:
    {
        float speed = axis2speed(ev.xpos);
        bool dir = ev.xpos > 0;
        std::fprintf(stderr, "LAxis, val = (%lf, %lf)", ev.xpos, ev.ypos);
        std::fprintf(stderr, "Speed: %lf, dir: %s\n", speed, dir ? "CW" : "CCW");
        controller.setTarget(speed, dir);
        break;
    }
    case XboxEvent::RightAxis:
    {
        float speed = axis2speed(ev.xpos) / 10.0;
        bool dir = ev.xpos > 0;
        std::fprintf(stderr, "RAxis, val = (%lf, %lf)", ev.xpos, ev.ypos);
        std::fprintf(stderr, "Speed: %lf, dir: %s\n", speed, dir ? "CW" : "CCW");
        controller.setTarget(speed, dir);
        break;
    }
    default:
        fprintf(stderr, "Unhandled event\n");
        break;
    }
    return;
}

int main(int argc, char **argv)
{
    StepperController xAxis(0);
    // TODO args
    XboxController controller("/dev/input/event5");
    controller.startEventThread();

    while (1)
    {
        std::optional<XboxEvent> ret = controller.getNextEvent();
        if (ret) {
            handleEvent(ret.value(), xAxis);
        }
        // todo -> select on evdev file descriptor
        usleep(1000);
    }
    return 0;
}
