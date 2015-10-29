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

                    float ALLOWED_MIN = -2.0F * canvasHeight;
                    float ALLOWED_MAX = +2.0F * canvasHeight;

                    HPEN pen = CreatePen(PS_SOLID, 2, color.ToCOLORREF());
                    HGDIOBJ oldPen = SelectObject(hdc, pen);

                    bool start = true;
                    bool breakPoint = false;

                    auto pt = points.get();

                    for (int i = 1; i < length; ++i)
                    {
                        if (std::isnan(pt[i].y) || pt[i].y < ALLOWED_MIN || pt[i].y > ALLOWED_MAX)
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
                            MoveToEx(hdc, 0, p, NULL);
                            LineTo(hdc, canvasWidth, p);
                        }
                        else
                        {
                            MoveToEx(hdc, p, 0, NULL);
                            LineTo(hdc, p, canvasHeight);
                        }
                    }

                    SelectObject(hdc, oldPen);
                    DeleteObject(pen);
                }

                virtual void beginDraw(int cw, int ch)
                {
                    canvasWidth = cw;
                    canvasHeight = ch;
                    HBRUSH background = CreateSolidBrush(RGB(255, 255, 255));
                    HGDIOBJ oldBackground = SelectObject(hdc, background);
                    Rectangle(hdc, 0, 0, canvasWidth, canvasHeight);
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