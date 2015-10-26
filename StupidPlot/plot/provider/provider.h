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
            struct POINTF
            {
                float x;
                float y;
            };

            class Provider
            {
            protected:
                HDC         hdc;
            public:
                Provider(HDC _hdc)
                {
                    hdc = _hdc;
                }

                virtual void drawGridLine(BOOL vertical, const shared_ptr<int> & points, int length) = 0;
                virtual void drawPlotLine(const shared_ptr<POINTF> & points, int length, Gdiplus::Color color) = 0;
                virtual void beginDraw(int w, int h) = 0;
                virtual void endDraw() = 0;
            };
        }
    }
}