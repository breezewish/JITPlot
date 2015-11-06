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
                HFONT       axisFont;

            public:
                GdiProvider(HDC _hdc) : Provider(_hdc)
                {
                    LOGFONT lfont;
                    lfont.lfHeight = -MulDiv(12, GetDeviceCaps(hdc, LOGPIXELSY), 72);
                    lfont.lfWidth = 0;
                    lfont.lfEscapement = 0;
                    lfont.lfOrientation = 0;
                    lfont.lfWeight = FW_REGULAR;
                    lfont.lfItalic = 0;
                    lfont.lfUnderline = 0;
                    lfont.lfStrikeOut = 0;
                    lfont.lfCharSet = ANSI_CHARSET;
                    lfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
                    lfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
                    lfont.lfQuality = CLEARTYPE_QUALITY;
                    lfont.lfPitchAndFamily = VARIABLE_PITCH | FF_ROMAN;
                    lstrcpyW(lfont.lfFaceName, L"Segoe UI");
                    axisFont = CreateFontIndirect(&lfont);
                }

                ~GdiProvider()
                {
                    DeleteObject(axisFont);
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

                virtual void drawGridLine(bool vertical, const shared_ptr<int> & points, int length, Gdiplus::Color color)
                {
                    HPEN pen = CreatePen(PS_SOLID, 2, color.ToCOLORREF());
                    HGDIOBJ oldPen = SelectObject(hdc, pen);

                    auto pt = points.get();

                    int minv, maxv;

                    if (vertical)
                    {
                        minv = canvasOffsetX;
                        maxv = canvasOffsetX + canvasWidth;
                    }
                    else
                    {
                        minv = canvasOffsetY;
                        maxv = canvasOffsetY + canvasHeight;
                    }

                    for (int i = 0; i < length; ++i)
                    {
                        int p = pt[i];

                        if (vertical)
                        {
                            MoveToEx(hdc, minv, p, NULL);
                            LineTo(hdc, maxv, p);
                        }
                        else
                        {
                            MoveToEx(hdc, p, minv, NULL);
                            LineTo(hdc, p, maxv);
                        }
                    }

                    SelectObject(hdc, oldPen);
                    DeleteObject(pen);
                }

                virtual void drawAxis(bool vertical, int axisPos, const shared_ptr<int> & ticks, const shared_ptr<double> & tickLabels, int length, int tickRadius, Gdiplus::Color color)
                {
                    HPEN pen = CreatePen(PS_SOLID, 2, color.ToCOLORREF());
                    HGDIOBJ oldPen = SelectObject(hdc, pen);
                    HGDIOBJ oldFont = SelectObject(hdc, axisFont);

                    // axis
                    if (vertical)
                    {
                        MoveToEx(hdc, axisPos, canvasOffsetY, NULL);
                        LineTo(hdc, axisPos, canvasOffsetY + canvasHeight);
                    }
                    else
                    {
                        MoveToEx(hdc, canvasOffsetX, axisPos, NULL);
                        LineTo(hdc, canvasOffsetX + canvasWidth, axisPos);
                    }

                    // ticks
                    int minv = axisPos - tickRadius;
                    int maxv = axisPos + tickRadius;

                    auto ptTick = ticks.get();
                    auto ptLabels = tickLabels.get();

                    for (int i = 0; i < length; ++i)
                    {
                        int p = ptTick[i];

                        wstring str = Util::to_string_with_precision(ptLabels[i], 5);

                        if (vertical)
                        {
                            MoveToEx(hdc, minv, p, NULL);
                            LineTo(hdc, maxv, p);
                            TextOutW(hdc, axisPos + 2, p + 2, str.c_str(), str.size());
                        }
                        else
                        {
                            MoveToEx(hdc, p, minv, NULL);
                            LineTo(hdc, p, maxv);
                            if (std::fabs(ptLabels[i]) > DBL_EPSILON)
                            {
                                TextOutW(hdc, p + 2, axisPos + 2, str.c_str(), str.size());
                            }
                        }
                    }

                    SelectObject(hdc, oldFont);
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