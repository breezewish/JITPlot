#pragma once

#include <vector>
#include <windows.h>
#include <gdiplus.h>

using std::vector;

namespace StupidPlot
{
    namespace Plot
    {
        namespace Provider
        {
            class Provider
            {
            protected:
                HDC         hdc;
            public:
                Provider(HDC _hdc)
                {
                    hdc = _hdc;
                }

                virtual void drawPlotLine(vector<Gdiplus::PointF> points, Gdiplus::Color color) = 0;
                virtual void drawGridLine(BOOL vertical, vector<int> points, int width, int height) = 0;
                virtual void beginDraw() = 0;
                virtual void endDraw() = 0;
            };
        }
    }
}