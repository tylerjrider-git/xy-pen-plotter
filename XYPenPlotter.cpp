#include "XYPenPlotter.h"
#include <cmath>

static constexpr float PI = 3.14159265;
static constexpr int MAX_RANGE_MM = 1000; // 10cm 
constexpr float ARC_STEP = 0.1; // draw 1 deg of arc at a time.

static inline float deg2rad(float deg)
{
    return 2*PI*(deg / 360);
}

XYPenPlotter::XYPenPlotter() :
    m_xAxis(0, MAX_RANGE_MM, StepperFlags::ContinousDrive),
    m_yAxis(1, MAX_RANGE_MM, StepperFlags::ContinousDrive)
{
}

XYPenPlotter::~XYPenPlotter()
{
}

void XYPenPlotter::drawLine(Line line)
{
    //m_zAxis.up();
    moveAbsolute(line.start);
    //m_zAxis.down();
    moveAbsolute(line.end);
    //m_zAxis.up();
}

void XYPenPlotter::moveAbsolute(Coordinate p, float speed)
{
    m_xAxis.sendCommand({StepperCommandType::AbsolutePosition, speed, p.x});
    m_yAxis.sendCommand({StepperCommandType::AbsolutePosition, speed, p.y});

    m_xAxis.waitCommand();
    m_yAxis.waitCommand();
}

void XYPenPlotter::moveRelative(Coordinate p, float speed)
{
    m_xAxis.sendCommand({StepperCommandType::RelativePosition, speed, p.x});
    m_yAxis.sendCommand({StepperCommandType::RelativePosition, speed, p.y});

    m_xAxis.waitCommand();
    m_yAxis.waitCommand();
}

Coordinate XYPenPlotter::currentPosition(void) const
{
     return { m_xAxis.currentPosition(), m_yAxis.currentPosition() };
}

void XYPenPlotter::resetOrigin(void)
{
    m_xAxis.resetOrigin();
    m_yAxis.resetOrigin();
}


void XYPenPlotter::drawArc(Coordinate center, float radius, float startAngle, float stopAngle, bool clockwise)
{
    if (startAngle > stopAngle) {
        std::fprintf(stderr, "startAngle(%.02f) cannot be larger than stopAngle(%.02f)\n",
            startAngle, stopAngle);
        return;
    }
    // slice up into small incremental steps.
    for (float theta = startAngle; theta < stopAngle; theta += ARC_STEP) {
        float x_rel = center.x + radius * std::cos(deg2rad(theta));
        float y_rel = center.y + radius * std::sin(deg2rad(theta));
        moveAbsolute({x_rel, y_rel});
    }
}