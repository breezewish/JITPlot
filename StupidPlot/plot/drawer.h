#pragma once

#include <vector>
#include <memory>
#include <cmath>

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
        protected:
            HPEN            hotTrackPen;

            inline void getClipRange(int & min, int & max, bool vertical)
            {
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
            }

            inline void getTicks(
                int & arrayLength,
                shared_ptr<int> & ticks,
                shared_ptr<double> & labels,
                int interval,
                int minDensity,
                bool vertical,
                bool autoResize = true)
            {
                int min, max;
                getClipRange(min, max, vertical);

                double canvasInterval = vertical ? translateFormulaH(interval) : translateFormulaW(interval);

                int n = ((max - min + 1) / interval) + 1;

                arrayLength = 0;
                ticks = shared_ptr<int>(new int[n], array_deleter<int>());
                labels = shared_ptr<double>(new double[n], array_deleter<double>());

                if (!autoResize && canvasInterval < minDensity) return;

                int scale = static_cast<int>(max(1, static_cast<double>(minDensity) / canvasInterval));

                // scale = 5^ceil(log(5, scale))
                scale = static_cast<int>(std::pow(5, std::ceil(std::log(scale) / std::log(5))));

                auto ptTicks = ticks.get();
                auto ptLabels = labels.get();

                for (int p = min; p < max; ++p)
                {
                    if (p % (interval * scale) == 0)
                    {
                        int canvasPos = static_cast<int>(vertical ? translateFormulaY(p) : translateFormulaX(p));
                        ptLabels[arrayLength] = static_cast<double>(p);
                        ptTicks[arrayLength++] = canvasPos;
                    }
                }
            }

            inline void destroyBuffer()
            {
                if (oldBitmap != NULL)
                {
                    HGDIOBJ buf = SelectObject(memDC, oldBitmap);
                    DeleteObject(buf);
                    oldBitmap = NULL;
                }
            }
        public:
            // ======== Internal object ========
            HDC                     hdc;
            PlotOptionsPtr          options = NULL;
            ProviderPtr             provider = NULL;

            // ======== Viewport ========
            int                     canvasWidth;
            int                     canvasHeight;
            int                     viewportWidth;
            int                     viewportHeight;
            int                     viewportLeft;
            int                     viewportTop;

            // ======== Clip ========
            int                     clipLeft;
            int                     clipTop;
            int                     clipWidth;
            int                     clipHeight;
            bool                    clipEnabled;

            // ======== Hot Track ========
            int                     htCanvasX = -1;
            int                     htCanvasY = -1;

            // ======== Memory Buffer ========
            HDC                     memDC;
            HGDIOBJ                 oldBitmap;

            inline void rebuildBuffer(int width, int height)
            {
                destroyBuffer();
                HBITMAP buf = CreateCompatibleBitmap(hdc, width, height);
                oldBitmap = SelectObject(memDC, buf);
            }

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

            inline double translateFormulaW(double w)
            {
                return w / (options->drawRight - options->drawLeft) * canvasWidth;
            }

            inline double translateFormulaH(double h)
            {
                return h / (options->drawTop - options->drawBottom) * canvasHeight;
            }

            inline double translateFormulaX(double x)
            {
                return translateFormulaW(x - (options->drawLeft));
            }

            inline double translateFormulaY(double y)
            {
                return canvasHeight - translateFormulaH(y - (options->drawBottom));
            }

            inline void drawPlotLine(const ExpDrawerPtr & exp)
            {
                int length = 0;
                auto points = shared_ptr<Provider::POINTF>(new Provider::POINTF[clipWidth], array_deleter<Provider::POINTF>());

                vector<double> formulaPoints;

                if (clipEnabled)
                {
                    formulaPoints = exp->evalAndTransform(options->vpLeft, options->vpRight, options->drawBottom, options->drawTop);
                }
                else
                {
                    formulaPoints = exp->evalAndTransform(options->drawLeft, options->drawRight, options->drawBottom, options->drawTop);
                }

                auto pt = points.get();

                for (int i = clipLeft, max = clipLeft + clipWidth; i < max; ++i)
                {
                    pt[length].x = static_cast<float>(i);
                    pt[length].y = static_cast<float>(formulaPoints[length]);
                    length++;
                }

                provider->drawPlotLine(points, length, exp->color);
            }

            inline void drawGridLine(int spacing, bool vertical, Gdiplus::Color color, bool infinite = true)
            {
                int length;
                shared_ptr<int> ticks;
                shared_ptr<double> labels;
                getTicks(length, ticks, labels, spacing, 20, vertical, infinite);
                provider->drawGridLine(vertical, ticks, length, color);
            }

            inline void drawAxisAndTick(int spacing, bool vertical, int tickRadius, Gdiplus::Color color, bool infinite = true)
            {
                int axisPos;

                if (vertical)
                {
                    // Y axis
                    axisPos = static_cast<int>(translateFormulaX(0));
                    if (axisPos < clipLeft - 50 || axisPos > clipLeft + clipWidth + 50) return;
                }
                else
                {
                    // X axis
                    axisPos = static_cast<int>(translateFormulaY(0));
                    if (axisPos < clipTop - 50 || axisPos > clipTop + clipHeight + 50) return;
                }

                int length;
                shared_ptr<int> ticks;
                shared_ptr<double> labels;
                getTicks(length, ticks, labels, spacing, 80, vertical, infinite);
                provider->drawAxis(vertical, axisPos, ticks, labels, length, tickRadius, color);
            }

            Drawer(const PlotOptionsPtr & _options, HDC _hdc, bool antialias)
            {
                options = _options;
                hdc = _hdc;

                memDC = CreateCompatibleDC(hdc);
                hotTrackPen = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));

                setAntialias(antialias);
            }

            ~Drawer()
            {
                DeleteObject(hotTrackPen);
                destroyBuffer();
                DeleteDC(memDC);
            }

            inline void setAntialias(bool enabled)
            {
                if (enabled)
                {
                    provider = ProviderPtr(new Provider::GdiPlusProvider(memDC));
                }
                else
                {
                    provider = ProviderPtr(new Provider::GdiProvider(memDC));
                }
            }

            inline void updateFormulaSize()
            {
                for (auto formula : options->expressions)
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

            inline void updateDrawingRange(int cw, int ch, int vpx, int vpy, int vpw, int vph)
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

            inline void refresh()
            {
                // Copy plot
                BitBlt(hdc, clipLeft, clipTop, clipWidth, clipHeight, memDC, clipLeft, clipTop, SRCCOPY);

                // Draw hot track
                if (options->enableHotTrack)
                {
                    HGDIOBJ oldPen = SelectObject(hdc, hotTrackPen);
                    MoveToEx(hdc, 0, htCanvasY, NULL);
                    LineTo(hdc, clipWidth + clipLeft, htCanvasY);
                    MoveToEx(hdc, htCanvasX, 0, NULL);
                    LineTo(hdc, htCanvasX, clipHeight + clipTop);
                    SelectObject(hdc, oldPen);
                }
            }

            inline void drawPlot()
            {
                provider->beginDraw(clipLeft, clipTop, clipWidth, clipHeight);

                // Draw grid lines
                if (options->showGrid && options->gridSpacing > 0)
                {
                    Gdiplus::Color glSmall(240, 240, 240);
                    Gdiplus::Color glLarge(200, 200, 200);
                    drawGridLine(options->gridSpacing, false, glSmall);
                    drawGridLine(options->gridSpacing, true, glSmall);
                    drawGridLine(options->gridSpacing * 5, false, glLarge, false);
                    drawGridLine(options->gridSpacing * 5, true, glLarge, false);
                }

                // Draw axis
                if (options->showAxis && options->axisTickInterval > 0)
                {
                    Gdiplus::Color axisColor(50, 50, 50);
                    drawAxisAndTick(options->axisTickInterval, false, 8, axisColor);
                    drawAxisAndTick(options->axisTickInterval, true, 8, axisColor);
                }

                // Draw formulas
                for (size_t i = 0; i < options->expressions.size(); ++i)
                {
                    drawPlotLine(options->expressions[i]);
                }

                provider->endDraw();

                refresh();
            }
        };

        typedef std::shared_ptr<Drawer> PlotDrawerPtr;
    }
}