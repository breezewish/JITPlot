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

                int canvasWidth, canvasHeight;
                int canvasOffsetX, canvasOffsetY;

            public:
                Provider(HDC _hdc)
                {
                    hdc = _hdc;
                }

                virtual void drawGridLine(BOOL vertical, const shared_ptr<int> & points, int length, Gdiplus::Color color) = 0;

                virtual void drawPlotLine(const shared_ptr<POINTF> & points, int length, Gdiplus::Color color) = 0;

                virtual void beginDraw(int left, int top, int width, int height)
                {
                    canvasWidth = width;
                    canvasHeight = height;
                    canvasOffsetX = left;
                    canvasOffsetY = top;
                }

                virtual void endDraw() = 0;
            };

            typedef std::shared_ptr<Provider> ProviderPtr;
        }
    }
}