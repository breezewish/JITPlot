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

                virtual void drawPlotLine(vector<Gdiplus::PointF> points, Gdiplus::Color color)
                {
                    UNREFERENCED_PARAMETER(points);
                    UNREFERENCED_PARAMETER(color);
                }

                virtual void drawGridLine(BOOL vertical, vector<int> points, int width, int height)
                {
                    UNREFERENCED_PARAMETER(vertical);
                    UNREFERENCED_PARAMETER(points);
                    UNREFERENCED_PARAMETER(width);
                    UNREFERENCED_PARAMETER(height);
                }

                virtual void beginDraw()
                {
                }

                virtual void endDraw()
                {
                }
            };
        }
    }
}