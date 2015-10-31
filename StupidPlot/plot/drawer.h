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

            int                     canvasWidth;
            int                     canvasHeight;
            int                     viewportWidth;
            int                     viewportHeight;
            int                     viewportLeft;
            int                     viewportTop;

            int                     clipLeft;
            int                     clipTop;
            int                     clipWidth;
            int                     clipHeight;
            bool                    clipEnabled;

            inline double translateCanvasW(int w)
            {
                return (static_cast<double>(w) / canvasWidth) * (options->drawRight - options->drawLeft);
            }

            inline double translateCanvasH(int h)
            {
                return (static_cast<double>(h) / canvasHeight) * (options->drawTop - options->drawBottom);
            }

            inline double translateCanvasX(int x)
            {
                return translateCanvasW(x) + options->drawLeft;
            }

            inline double translateCanvasY(int y)
            {
                return translateCanvasH(canvasHeight - y) + options->drawBottom;
            }

            inline double translateFormulaX(double x)
            {
                return (x - (options->drawLeft)) / (options->drawRight - options->drawLeft) * canvasWidth;
            }

            inline double translateFormulaY(double y)
            {
                return canvasHeight - ((y - (options->drawBottom)) / (options->drawTop - options->drawBottom) * canvasHeight);
            }

            inline void drawPlotLine(const ExpDrawerPtr & formulaDrawer, Gdiplus::Color color)
            {
                int length = 0;
                auto points = shared_ptr<Provider::POINTF>(new Provider::POINTF[clipWidth], array_deleter<Provider::POINTF>());

                vector<double> formulaPoints;

                if (clipEnabled)
                {
                    formulaPoints = formulaDrawer->evalAndTransform(options->vpLeft, options->vpRight, options->drawBottom, options->drawTop);
                }
                else
                {
                    formulaPoints = formulaDrawer->evalAndTransform(options->drawLeft, options->drawRight, options->drawBottom, options->drawTop);
                }

                auto pt = points.get();

                for (int i = clipLeft, max = clipLeft + clipWidth; i < max; ++i)
                {
                    pt[length].x = static_cast<float>(i);
                    pt[length].y = static_cast<float>(formulaPoints[length]);
                    length++;
                }

                provider->drawPlotLine(points, length, color);
            }

            inline void drawGridLine(BOOL vertical)
            {
                int min, max;

                if (vertical)
                {
                    if (clipEnabled)
                    {
                        min = static_cast<int>(std::floor(options->vpBottom));
                        max = static_cast<int>(std::ceil(options->vpTop));
                    }
                    else
                    {
                        min = static_cast<int>(std::floor(options->drawBottom));
                        max = static_cast<int>(std::ceil(options->drawTop));
                    }
                }
                else
                {
                    if (clipEnabled)
                    {
                        min = static_cast<int>(std::floor(options->vpLeft));
                        max = static_cast<int>(std::ceil(options->vpRight));
                    }
                    else
                    {
                        min = static_cast<int>(std::floor(options->drawLeft));
                        max = static_cast<int>(std::ceil(options->drawRight));
                    }
                }

                int n = ((max - min + 1) / options->gridSpacing) + 1;

                // too dense
                if (n > clipWidth / 10) return;

                int length = 0;
                auto points = shared_ptr<int>(new int[n], array_deleter<int>());

                for (int p = min; p < max; ++p)
                {
                    if (p % options->gridSpacing == 0)
                    {
                        int canvasPos = static_cast<int>(vertical ? translateFormulaY(p) : translateFormulaX(p));
                        points.get()[length++] = canvasPos;
                    }
                }

                provider->drawGridLine(vertical, points, length, Gdiplus::Color(233, 233, 233));
            }

            Drawer(const PlotOptionsPtr & _options, HDC _hdc, bool antialias)
            {
                options = _options;
                hdc = _hdc;
                setAntialias(antialias);
            }

            inline void setAntialias(bool enabled)
            {
                if (enabled)
                {
                    provider = ProviderPtr(new Provider::GdiPlusProvider(hdc));
                }
                else
                {
                    provider = ProviderPtr(new Provider::GdiProvider(hdc));
                }
            }

            inline void updateFormulaSize()
            {
                for (auto formula : options->formulaObjects)
                {
                    formula->setSize(clipWidth, clipHeight, canvasWidth, canvasHeight);
                }
            }

            inline void clipToViewport()
            {
                clipEnabled = true;
                clipLeft = viewportLeft;
                clipTop = viewportTop;
                clipWidth = viewportWidth;
                clipHeight = viewportHeight;
                updateFormulaSize();
            }

            inline void resetClipToCanvas()
            {
                clipEnabled = false;
                clipLeft = 0;
                clipTop = 0;
                clipWidth = canvasWidth;
                clipHeight = canvasHeight;
                updateFormulaSize();
            }

            inline void setSize(int cw, int ch, int vpx, int vpy, int vpw, int vph)
            {
                canvasWidth = cw;
                canvasHeight = ch;
                viewportLeft = vpx;
                viewportTop = vpy;
                viewportWidth = vpw;
                viewportHeight = vph;

                if (clipEnabled)
                {
                    clipToViewport();
                }
                else
                {
                    resetClipToCanvas();
                }
            }

            inline void draw()
            {
                provider->beginDraw(clipLeft, clipTop, clipWidth, clipHeight);

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