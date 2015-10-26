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
                int                     width, height;
                Gdiplus::Graphics       * g = NULL;

            public:
                GdiPlusProvider(HDC _hdc) : Provider(_hdc)
                {
                }

                ~GdiPlusProvider()
                {
                }

                virtual void drawPlotLine(shared_ptr<vector<Gdiplus::PointF>> _points, Gdiplus::Color color)
                {
                    if (_points->size() == 0) return;
                    Gdiplus::Pen pen(color, 2.0f);
                    g->DrawCurve(&pen, &_points->operator[](0), _points->size());
                }

                virtual void drawGridLine(BOOL vertical, vector<int> points)
                {
                    Gdiplus::Pen pen(Gdiplus::Color(128, 128, 128, 128), 1.0f);
                    for (int p : points)
                    {
                        if (vertical)
                        {
                            g->DrawLine(&pen, 0, p, width, p);
                        }
                        else
                        {
                            g->DrawLine(&pen, p, 0, p, height);
                        }
                    }
                }

                virtual void beginDraw(int w, int h)
                {
                    width = w;
                    height = h;
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