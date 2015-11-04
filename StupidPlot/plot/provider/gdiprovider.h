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

                    int bufLen = 0;
                    POINT * buf = new POINT[length];

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
                            if (bufLen > 1) Polyline(hdc, buf, bufLen);
                            bufLen = 0;
                            if (!breakPoint) start = false;
                        }

                        buf[bufLen].x = static_cast<int>(pt[i].x);
                        buf[bufLen].y = static_cast<int>(pt[i].y);
                        bufLen++;
                    }

                    if (bufLen > 1) Polyline(hdc, buf, bufLen);

                    delete[] buf;

                    SelectObject(hdc, oldPen);
                    DeleteObject(pen);
                }

                virtual void drawGridLine(BOOL vertical, const shared_ptr<int> & points, int length, Gdiplus::Color color)
                {
                    HPEN pen = CreatePen(PS_SOLID, 2, color.ToCOLORREF());
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
                    Provider::beginDraw(left, top, width, height);

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