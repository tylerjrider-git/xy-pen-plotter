#pragma once
#include "StepperController.h"

typedef struct  {
    float x;
    float y;
} Coordinate;

typedef struct  {
    Coordinate start;
    Coordinate end;
} Line;

class XYPenPlotter {
public:
    XYPenPlotter();
    virtual ~XYPenPlotter();

    void drawLine(Line line, float speed=1.0);
    void drawArc(Coordinate p, float radius, float startAngle, float stopAngle, bool clockwise);
    // G-Code style arc creation
    void drawArc(Coordinate start, Coordinate end, float I, float J, bool clockwise);
    void moveAbsolute(Coordinate p, float speed=1.0);
    void moveRelative(Coordinate p, float speed=1.0);

    Coordinate currentPosition(void) const;
    void resetOrigin(void);

    StepperController m_xAxis;
    StepperController m_yAxis;
};

