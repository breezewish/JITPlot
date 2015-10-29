#pragma once

#include <vector>
#include <memory>

#include <windows.h>
#include <gdiplus.h>

#include <formula/expdrawer.h>
#include <plot/optionbag.h>
#include <plot/provider/provider.h>
#include <plot/provider/gdiprovider.h>
#include <plot/provider/gdiplusprovider.h>

using std::vector;

namespace StupidPlot
{
    namespace Plot
    {
        using Provider::ProviderPtr;

        class Drawer
        {
        public:
            HDC                     hdc;
            PlotOptionsPtr          options = NULL;
            ProviderPtr             provider = NULL;

            int                     width;
            int                     height;

            inline double translateCanvasW(int w)
            {
                return (static_cast<double>(w) / width) * (options->drawRight - options->drawLeft);
            }

            inline double translateCanvasH(int h)
            {
                return (static_cast<double>(h) / height) * (options->drawTop - options->drawBottom);
            }

            inline double translateCanvasX(int x)
            {
                return translateCanvasW(x) + options->drawLeft;
            }

            inline double translateCanvasY(int y)
            {
                return translateCanvasH(y) + options->drawTop;
            }

            inline double translateFormulaX(double x)
            {
                return (x - (options->drawLeft)) / (options->drawRight - options->drawLeft) * width;
            }

            inline double translateFormulaY(double y)
            {
                return (y - (options->drawBottom)) / (options->drawTop - options->drawBottom) * height;
            }

            inline void drawPlotLine(const ExpDrawerPtr & formulaDrawer, Gdiplus::Color color)
            {
                int length = 0;
                auto points = shared_ptr<Provider::POINTF>(new Provider::POINTF[width], array_deleter<Provider::POINTF>());

                auto formulaPoints = formulaDrawer->evalAndTransform(options->drawLeft, options->drawRight, options->drawBottom, options->drawTop);
                auto pt = points.get();

                for (int i = 0; i < width; ++i)
                {
                    pt[length].x = static_cast<float>(i);
                    pt[length].y = static_cast<float>(formulaPoints[i]);
                    length++;
                }

                provider->drawPlotLine(points, length, color);
            }

            inline void drawGridLine(BOOL vertical)
            {
                int min = static_cast<int>(vertical ? options->drawBottom : options->drawLeft);
                int max = static_cast<int>(vertical ? options->drawTop : options->drawRight);
                int sz = ((max - min + 1) / options->gridSpacing) + 1;

                int length = 0;
                auto points = shared_ptr<int>(new int[sz], array_deleter<int>());

                int lastDrawScreenPos = -100;

                for (int p = min; p < max; ++p)
                {
                    if (p % options->gridSpacing == 0)
                    {
                        int canvasPos = static_cast<int>(vertical ? translateFormulaY(p) : translateFormulaX(p));
                        if (canvasPos - lastDrawScreenPos >= 10)
                        {
                            points.get()[length] = canvasPos;
                            length++;
                            lastDrawScreenPos = canvasPos;
                        }
                    }
                }

                provider->drawGridLine(vertical, points, length);
            }

            Drawer(const PlotOptionsPtr & _options, HDC _hdc)
            {
                options = _options;
                hdc = _hdc;
                provider = ProviderPtr(new Provider::GdiProvider(hdc));
            }

            inline void setCanvasSize(int w, int h)
            {
                if (width != w || height != h)
                {
                    for (auto formula : options->formulaObjects)
                    {
                        formula->setCanvasSize(w, h);
                    }
                }

                width = w;
                height = h;
            }

            inline void draw(int canvasWidth, int canvasHeight)
            {
                setCanvasSize(canvasWidth, canvasHeight);
                provider->beginDraw(canvasWidth, canvasHeight);

                // Draw grid lines
                drawGridLine(false);
                drawGridLine(true);

                // Draw formulas
                for (size_t i = 0; i < options->formulaObjects.size(); ++i)
                {
                    drawPlotLine(options->formulaObjects[i], options->formulaColors[i]);
                }

                provider->endDraw();
            }
        };

        typedef std::shared_ptr<Drawer> PlotDrawerPtr;
    }
}