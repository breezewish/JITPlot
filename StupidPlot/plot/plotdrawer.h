#pragma once

#include <vector>
#include <memory>

#include <windows.h>
#include <gdiplus.h>

#include <formula/expdrawer.h>
#include <plot/plotoptions.h>
#include <plot/provider/provider.h>
#include <plot/provider/gdiprovider.h>
#include <plot/provider/gdiplusprovider.h>

using std::vector;

namespace StupidPlot
{
    namespace Plot
    {
        using Provider::ProviderPtr;

        class PlotDrawer
        {
        public:
            HDC                     hdc;
            PlotOptionsPtr          options = NULL;
            ProviderPtr             provider = NULL;

            int                     width;
            int                     height;

            inline double translateCanvasW(int w)
            {
                return (static_cast<double>(w) / width) * (options->right - options->left);
            }

            inline double translateCanvasH(int h)
            {
                return (static_cast<double>(h) / height) * (options->top - options->bottom);
            }

            inline double translateCanvasX(int x)
            {
                return (static_cast<double>(x) / width) * (options->right - options->left) + options->left;
            }

            inline double translateCanvasY(int y)
            {
                return (static_cast<double>(y) / height) * (options->top - options->bottom) + options->top;
            }

            inline double translateFormulaX(double x)
            {
                return (x - (options->left)) / (options->right - options->left) * width;
            }

            inline double translateFormulaY(double y)
            {
                return (y - (options->bottom)) / (options->top - options->bottom) * height;
            }

            void drawPlotLine(const ExpDrawerPtr & formulaDrawer, Gdiplus::Color color)
            {
                int length = 0;
                auto points = shared_ptr<Provider::POINTF>(new Provider::POINTF[width], array_deleter<Provider::POINTF>());

                auto formulaPoints = formulaDrawer->evalAndTransform(options->left, options->right, options->bottom, options->top);
                auto pt = points.get();

                for (int i = 0; i < width; ++i)
                {
                    pt[length].x = static_cast<float>(i);
                    pt[length].y = static_cast<float>(formulaPoints[i]);
                    length++;
                }

                provider->drawPlotLine(points, length, color);
            }

            void drawGridLine(BOOL vertical)
            {
                int min = static_cast<int>(vertical ? options->bottom : options->left);
                int max = static_cast<int>(vertical ? options->top : options->right);
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

            PlotDrawer(const PlotOptionsPtr & _options, HDC _hdc)
            {
                options = _options;
                hdc = _hdc;
                provider = ProviderPtr(new Provider::GdiProvider(hdc));
            }

            void draw(int canvasWidth, int canvasHeight)
            {
                if (width != canvasWidth || height != canvasHeight)
                {
                    for (auto formula : options->formulaObjects)
                    {
                        formula->setViewportSize(canvasWidth, canvasHeight);
                    }
                }

                width = canvasWidth;
                height = canvasHeight;

                provider->beginDraw(width, height);

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

        typedef std::shared_ptr<PlotDrawer> PlotDrawerPtr;
    }
}