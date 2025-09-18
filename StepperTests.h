#pragma once
#include "StepperController.h"
#include "XYPenPlotter.h"
#include <chrono>
#include <vector>
#include <cmath>

using namespace std::chrono_literals;

static void testOriginReset(XYPenPlotter& plotter)
{
    plotter.moveAbsolute({-10.0, -10.0});
    plotter.moveRelative({15.0, 15.0});
    plotter.moveAbsolute({0.0, 0.0});
}

static void testCoordinates(XYPenPlotter& plotter)
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
        plotter.moveRelative(coordinate, 0.5);
        std::this_thread::sleep_for(1000ms);
    }
}


static void testArcs(XYPenPlotter& plotter)
{
    StepperController& xAxis = plotter.m_xAxis;
    StepperController& yAxis = plotter.m_yAxis;
    for (float t = 0; t < 3; ) {
        xAxis.setTargetVelocity(sin(2*3.14159*t));
        yAxis.setTargetVelocity(cos(2*3.14159*t));
        t += 0.010;
        std::this_thread::sleep_for(10ms);
    }
    yAxis.setTargetVelocity(0);
    xAxis.setTargetVelocity(0);
}

static void testCircles(XYPenPlotter& plotter)
{
    // plotter.drawArc({0,0}, 10.0, 0, 180, true);


    plotter.drawArc({0,0}, {0, 50}, 25, -1, true);
}

static void testSymbolRaw(XYPenPlotter& plotter)
{
    std::vector<Coordinate> coordinates = {
        {0, 10},
        {-10, 10},
        {0 , 10},
        {10, 10},
        {10, -10}, // top
        {0, -10},
        {-5, -5},
        {-5, 5},
        {0, 10}, // top middle vertical line
        {0, -10},
        {10, -10},
        {0, -10},
        {-10, -10},
        {-10, 10},
        {0, 10},
        {5, 5},
    };
    for (auto coordinate : coordinates) {
        plotter.moveRelative({coordinate.y, coordinate.x}, 0.5);
        std::this_thread::sleep_for(1000ms);
    }
}
static void testSymbol(XYPenPlotter& plotter)
{
    std::vector<Line> lines = {
        {{0, 0},  {0, 10}},
        {{0, 10}, {-10, 20}}, // {-10, 10} 
        {{-10, 20}, {-10, 30}}, // {0 , 10}, 
        {{-10, 30}, {0, 40}}, // {10, 10}, 
        {{0, 40}, {10, 30}}, //  {10, -10} top 
        {{10, 30}, {10, 20}}, //  {0, -10}, 

        {{10, 20}, {5, 15}}, //  {-5, -5}, 
        {{5, 15}, {0, 20}}, // {-5, 5}, 
        {{0, 20}, {0, 30}}, // {0, 10} top middle vertical line  
        {{0, 30}, {0, 20}}, // {0, -10} *

        {{0, 20}, {10, 10}}, // {10, -10} 
        {{10, 10}, {10, 0}}, //  {0, -10}, 
        {{10, 0}, {0, -10}}, // {-10, -10}, 
        {{0, -10}, {-10, 0}}, // {-10, 10},
        {{-10, 0}, {-10,10}}, // {0, 10}
        {{-10,10}, {-5, 15}} // {5,5}

    };
        for (auto line : lines) {
            Line line_t = {{line.start.y, line.start.x}, {line.end.y, line.end.x}};
            plotter.drawLine(line_t, 0.5);
        }
    // plotter.moveAbsolute({0,0});
}