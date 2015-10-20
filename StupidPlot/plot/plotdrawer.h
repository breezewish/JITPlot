#pragma once

#include <vector>
#include <windows.h>
#include <gdiplus.h>

#include <formula/formulaobject.h>
#include <plot/plotoptions.h>
#include <plot/provider/provider.h>
#include <plot/provider/gdiprovider.h>
#include <plot/provider/gdiplusprovider.h>

using std::vector;
using StupidPlot::Formula::FormulaObject;

namespace StupidPlot
{
    namespace Plot
    {
        class PlotDrawer
        {
        protected:
            HDC                     hdc;
            PlotOptions             * options = NULL;
            Provider::Provider      * provider = NULL;

            vector<Gdiplus::PointF> getFormulaPoints(FormulaObject * formula, int width, int height)
            {
                vector<Gdiplus::PointF> points;
                for (int pos_x = 0; pos_x < width; ++pos_x)
                {
                    double x = (static_cast<double>(pos_x) / width) * (options->right - options->left) + options->left;
                    double y = formula->evaluate(x);
                    double pos_y = (y - (options->bottom)) / (options->top - options->bottom) * height;
                    if (pos_y >= 0 && pos_y <= height)
                    {
                        points.push_back(Gdiplus::PointF(static_cast<float>(pos_x), static_cast<float>(pos_y)));
                    }
                }
                return points;
            }

        public:
            PlotDrawer(PlotOptions * _options, HDC _hdc)
            {
                options = _options;
                hdc = _hdc;
                provider = new Provider::GdiPlusProvider(hdc);
            }

            ~PlotDrawer()
            {
                delete provider;
            }

            void draw(int canvasWidth, int canvasHeight)
            {
                provider->beginDraw();

                // Draw formulas
                for (size_t i = 0; i < options->formulaObjects.size(); ++i)
                {
                    provider->drawPlotLine(
                        getFormulaPoints(options->formulaObjects[i], canvasWidth, canvasHeight),
                        options->formulaColors[i]
                        );
                }

                provider->endDraw();
            }
        };
    }
}