#pragma once

#include <vector>
#include <memory>
#include <cmath>
#include <cfloat>

#include <windows.h>
#include <gdiplus.h>

#include <plot/provider/provider.h>

namespace StupidPlot
{
    namespace Plot
    {
        namespace Provider
        {
            class GdiPlusProvider : public Provider
            {
            protected:
                shared_ptr<Gdiplus::Font>           axisFont;
                shared_ptr<Gdiplus::StringFormat>   axisStringformat;
                Gdiplus::Graphics                   * g = NULL;

            public:
                GdiPlusProvider(HDC _hdc) : Provider(_hdc)
                {
                    axisFont = shared_ptr<Gdiplus::Font>(new Gdiplus::Font(L"Segoe UI", 12, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint));
                    axisStringformat = shared_ptr<Gdiplus::StringFormat>(new Gdiplus::StringFormat());
                    axisStringformat->SetAlignment(Gdiplus::StringAlignmentNear);
                    axisStringformat->SetLineAlignment(Gdiplus::StringAlignmentNear);
                }

                ~GdiPlusProvider()
                {
                }

                virtual void drawPlotLine(const shared_ptr<POINTF> & points, int length, Gdiplus::Color color, int width)
                {
                    if (length == 0) return;

                    float ALLOWED_MIN_PX = -2.0F * canvasHeight + canvasOffsetY;
                    float ALLOWED_MAX_PX = +3.0F * canvasHeight + canvasOffsetY;

                    Gdiplus::Pen pen(color, static_cast<float>(width));

                    int bufLen = 0;
                    Gdiplus::PointF * buf = new Gdiplus::PointF[length];

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
                            if (bufLen > 1) g->DrawCurve(&pen, buf, bufLen);
                            bufLen = 0;
                            if (!breakPoint) start = false;
                        }

                        buf[bufLen].X = pt[i].x;
                        buf[bufLen].Y = pt[i].y;
                        bufLen++;
                    }

                    if (bufLen > 1) g->DrawCurve(&pen, buf, bufLen);

                    delete[] buf;
                }

                virtual void drawGridLine(bool vertical, const shared_ptr<int> & points, int length, Gdiplus::Color color)
                {
                    Gdiplus::Pen pen(color, 2.0f);

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
                            g->DrawLine(&pen, minv, p, maxv, p);
                        }
                        else
                        {
                            g->DrawLine(&pen, p, minv, p, maxv);
                        }
                    }
                }

                virtual void drawAxis(bool vertical, int axisPos, const shared_ptr<int> & ticks, const shared_ptr<double> & tickLabels, int length, int tickRadius, Gdiplus::Color color)
                {
                    Gdiplus::Pen pen(color, 2.0f);
                    Gdiplus::SolidBrush fontBrush(color);

                    // axis
                    if (vertical)
                    {
                        g->DrawLine(&pen, axisPos, canvasOffsetY, axisPos, canvasOffsetY + canvasHeight);
                    }
                    else
                    {
                        g->DrawLine(&pen, canvasOffsetX, axisPos, canvasOffsetX + canvasWidth, axisPos);
                    }

                    // tick
                    int minv = axisPos - tickRadius;
                    int maxv = axisPos + tickRadius;

                    auto ptTick = ticks.get();
                    auto ptLabels = tickLabels.get();

                    for (int i = 0; i < length; ++i)
                    {
                        int p = ptTick[i];

                        if (vertical)
                        {
                            g->DrawLine(&pen, minv, p, maxv, p);
                            g->DrawString(
                                Util::to_string_with_precision(ptLabels[i], 5).c_str(),
                                -1,
                                axisFont.get(),
                                Gdiplus::PointF(static_cast<float>(axisPos + 2), static_cast<float>(p + 2)),
                                &fontBrush
                                );
                        }
                        else
                        {
                            g->DrawLine(&pen, p, minv, p, maxv);
                            if (std::fabs(ptLabels[i]) > DBL_EPSILON)
                            {
                                g->DrawString(
                                    Util::to_string_with_precision(ptLabels[i], 5).c_str(),
                                    -1,
                                    axisFont.get(),
                                    Gdiplus::PointF(static_cast<float>(p + 2), static_cast<float>(axisPos + 2)),
                                    &fontBrush
                                    );
                            }
                        }
                    }
                }

                virtual void beginDraw(int left, int top, int width, int height)
                {
                    Provider::beginDraw(left, top, width, height);
                    g = new Gdiplus::Graphics(hdc);
                    g->SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);
                    g->SetTextRenderingHint(Gdiplus::TextRenderingHint::TextRenderingHintClearTypeGridFit);
                    g->Clear(Gdiplus::Color(255, 255, 255, 255));
                }

                virtual void endDraw()
                {
                    delete g;
                }
            };
        }
    }
}