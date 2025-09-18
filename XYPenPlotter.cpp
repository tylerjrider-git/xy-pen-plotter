#include "XYPenPlotter.h"
#include <cmath>

static constexpr float PI = 3.14159265;
static constexpr int MAX_RANGE_MM = 1000; // 10cm 
constexpr float ARC_STEP = 0.1; // draw 1 deg of arc at a time.

static inline float deg2rad(float deg)
{
    return 2*PI*(deg / 360);
}

static inline float rad2deg(float rad)
{
    return (rad / (2*PI)) * 360;
}

XYPenPlotter::XYPenPlotter() :
    m_xAxis(0, MAX_RANGE_MM, StepperFlags::ContinousDrive),
    m_yAxis(1, MAX_RANGE_MM, StepperFlags::ContinousDrive)
{
}

XYPenPlotter::~XYPenPlotter()
{
}

void XYPenPlotter::drawLine(Line line, float speed)
{
    //m_zAxis.up();
    moveAbsolute(line.start, speed);
    //m_zAxis.down();
    moveAbsolute(line.end, speed);
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
    // slice up into small incremental steps, counterclockwise
    for (float theta = startAngle; theta < stopAngle; theta += ARC_STEP) {
        float x_rel = center.x + radius * std::cos(deg2rad(theta));
        float y_rel = center.y + radius * std::sin(deg2rad(theta));
        moveAbsolute({y_rel, x_rel});
    }
}

void XYPenPlotter::drawArc(Coordinate start, Coordinate end, float I, float J, bool clockwise)
{
    // GCode style plot. (G2=>clockwise)
    moveAbsolute(start);

    // Build center point.
    Coordinate C  = {start.x + I, start.y + J};
    float R = std::sqrt(I*I + J*J);

    /*
    S.            E
        .
          . 
             .
               C............ E
              
    Want:
      theta_s = 135, tan2(I, J) => tan2(-1, 1) => 
      theta_e = 0;
    Have:
      I = 1.
      J = -1.

    Thus:
        Theta_ECS => atan2(S.y - C.y, S.x -C.x)
    but:
        C.y = S.y + J => S.y -C.y = -J
        C.x = S.x + I => S.x - C.x = -I
        Theta_ECS => atan2(-J, -I)
        Theta_ECS => atan2(1, -1) => 
    */
    float startAngle = rad2deg(std::atan2(-I, -J));
    //  I = R*cos(startANgle) = I;
    //  J = R*sin(startANgle) = J;
    float stopAngle = rad2deg(std::atan2(end.y - C.y, end.x - end.y));

    // Note there is no verification that dist(C,E) == R, only use R from I,J.
    printf("S:{%.02f,%.02f}, C:{%.02f, %.02f}, R:{%.02f}, startAng:{%.02f}, end:{%.02f}\n",
        start.x, start.y, C.x, C.y, R, startAngle, stopAngle);
    drawArc(C, R, startAngle, stopAngle, clockwise);
}