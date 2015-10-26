#pragma once

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
                int width, height;
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

                    HPEN pen = CreatePen(PS_SOLID, 2, color.ToCOLORREF());
                    HGDIOBJ oldPen = SelectObject(hdc, pen);

                    MoveToEx(hdc, static_cast<int>(points.get()[0].x), static_cast<int>(points.get()[0].y), NULL);
                    for (int i = 1; i < length; ++i)
                    {
                        LineTo(hdc, static_cast<int>(points.get()[i].x), static_cast<int>(points.get()[i].y));
                    }

                    SelectObject(hdc, oldPen);
                    DeleteObject(pen);
                }

                virtual void drawGridLine(BOOL vertical, const shared_ptr<int> & points, int length)
                {
                    HPEN pen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
                    HGDIOBJ oldPen = SelectObject(hdc, pen);

                    for (int i = 0; i < length; ++i)
                    {
                        int p = points.get()[i];

                        if (vertical)
                        {
                            MoveToEx(hdc, 0, p, NULL);
                            LineTo(hdc, width, p);
                        }
                        else
                        {
                            MoveToEx(hdc, p, 0, NULL);
                            LineTo(hdc, p, height);
                        }
                    }

                    SelectObject(hdc, oldPen);
                    DeleteObject(pen);
                }

                virtual void beginDraw(int w, int h)
                {
                    width = w;
                    height = h;
                    HBRUSH background = CreateSolidBrush(RGB(255, 255, 255));
                    HGDIOBJ oldBackground = SelectObject(hdc, background);
                    Rectangle(hdc, 0, 0, width, height);
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