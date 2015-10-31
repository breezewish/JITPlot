#pragma once

#include <vector>
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
                Gdiplus::Graphics       * g = NULL;

            public:
                GdiPlusProvider(HDC _hdc) : Provider(_hdc)
                {
                }

                ~GdiPlusProvider()
                {
                }

                virtual void drawPlotLine(const shared_ptr<POINTF> & points, int length, Gdiplus::Color color)
                {
                    if (length == 0) return;

                    float ALLOWED_MIN_PX = -2.0F * canvasHeight + canvasOffsetY;
                    float ALLOWED_MAX_PX = +3.0F * canvasHeight + canvasOffsetY;

                    Gdiplus::Pen pen(color, 2.0f);

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

                virtual void drawGridLine(BOOL vertical, const shared_ptr<int> & points, int length, Gdiplus::Color color)
                {
                    Gdiplus::Pen pen(color, 2.0f);

                    auto pt = points.get();

                    for (int i = 0; i < length; ++i)
                    {
                        int p = pt[i];

                        if (vertical)
                        {
                            g->DrawLine(&pen, canvasOffsetX, p, canvasOffsetX + canvasWidth, p);
                        }
                        else
                        {
                            g->DrawLine(&pen, p, canvasOffsetY, p, canvasOffsetY + canvasHeight);
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