#pragma once

#include <cmath>

#include <windows.h>
#include <plot/provider/provider.h>

namespace StupidPlot
{
    namespace Plot
    {
        namespace Provider
        {
            class GdiProvider : public Provider
            {
            protected:
                int canvasWidth, canvasHeight;
                int canvasOffsetX, canvasOffsetY;

            public:
                GdiProvider(HDC _hdc) : Provider(_hdc)
                {
                }

                ~GdiProvider()
                {
                }

                virtual void drawPlotLine(const shared_ptr<POINTF> & points, int length, Gdiplus::Color color)
                {
                    if (length == 0) return;

                    float ALLOWED_MIN_PX = -2.0F * canvasHeight + canvasOffsetY;
                    float ALLOWED_MAX_PX = +3.0F * canvasHeight + canvasOffsetY;

                    HPEN pen = CreatePen(PS_SOLID, 2, color.ToCOLORREF());
                    HGDIOBJ oldPen = SelectObject(hdc, pen);

                    bool start = true;
                    bool breakPoint = false;

                    auto pt = points.get();

                    for (int i = 1; i < length; ++i)
                    {
                        if (std::isnan(pt[i].y) || pt[i].y < ALLOWED_MIN_PX || pt[i].y > ALLOWED_MAX_PX)
                        {
                            breakPoint = true;
                            start = true;
                        }
                        else
                        {
                            breakPoint = false;
                        }

                        if (start)
                        {
                            MoveToEx(hdc, static_cast<int>(pt[i].x), static_cast<int>(pt[i].y), NULL);
                            if (!breakPoint) start = false;
                        }
                        else
                        {
                            LineTo(hdc, static_cast<int>(pt[i].x), static_cast<int>(pt[i].y));
                        }
                    }

                    SelectObject(hdc, oldPen);
                    DeleteObject(pen);
                }

                virtual void drawGridLine(BOOL vertical, const shared_ptr<int> & points, int length)
                {
                    HPEN pen = CreatePen(PS_SOLID, 2, RGB(233, 233, 233));
                    HGDIOBJ oldPen = SelectObject(hdc, pen);

                    for (int i = 0; i < length; ++i)
                    {
                        int p = points.get()[i];

                        if (vertical)
                        {
                            MoveToEx(hdc, canvasOffsetX, p, NULL);
                            LineTo(hdc, canvasOffsetX + canvasWidth, p);
                        }
                        else
                        {
                            MoveToEx(hdc, p, canvasOffsetY, NULL);
                            LineTo(hdc, p, canvasOffsetY + canvasHeight);
                        }
                    }

                    SelectObject(hdc, oldPen);
                    DeleteObject(pen);
                }

                virtual void beginDraw(int left, int top, int width, int height)
                {
                    canvasWidth = width;
                    canvasHeight = height;
                    canvasOffsetX = left;
                    canvasOffsetY = top;
                    HBRUSH background = CreateSolidBrush(RGB(255, 255, 255));
                    HGDIOBJ oldBackground = SelectObject(hdc, background);
                    Rectangle(hdc, left, top, width + left, height + top);
                    SelectObject(hdc, oldBackground);
                    DeleteObject(background);
                }

                virtual void endDraw()
                {
                }
            };
        }
    }
}