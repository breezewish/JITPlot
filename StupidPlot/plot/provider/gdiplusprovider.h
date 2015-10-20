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

                virtual void drawPlotLine(vector<Gdiplus::PointF> _points, Gdiplus::Color color)
                {
                    Gdiplus::Pen pen(color, 2.0f);
                    g->DrawCurve(&pen, &_points[0], _points.size());
                }

                virtual void beginDraw()
                {
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