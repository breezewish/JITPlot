#pragma once

#include <vector>
#include <memory>

#include <windows.h>
#include <gdiplus.h>

#include <formula/expdrawer.h>

using std::shared_ptr;
using std::vector;

namespace StupidPlot
{
    namespace Plot
    {
        using Formula::ExpDrawerPtr;

        class OptionBag
        {
        public:
            double                  drawLeft, drawRight, drawTop, drawBottom;
            double                  vpLeft, vpRight, vpTop, vpBottom;

            bool                    keepRatio = false;

            int                     gridSpacing = 1;

            vector<Gdiplus::Color>  formulaColors;
            vector<ExpDrawerPtr>    formulaObjects;

            OptionBag()
            {
                vpLeft = -5.0;
                vpRight = 5.0;
                vpTop = 5.0;
                vpBottom = -5.0;
                drawLeft = vpLeft;
                drawTop = vpTop;
                drawRight = vpRight;
                drawBottom = vpBottom;
            }

            void calculateOuterBoundaryInCenter(double enlargeFactor)
            {
                double vpW = vpRight - vpLeft;
                double vpH = vpTop - vpBottom;
                drawLeft = vpLeft - (enlargeFactor - 1) * vpW / 2;
                drawRight = drawLeft + vpW * enlargeFactor;
                drawBottom = vpBottom - (enlargeFactor - 1) * vpH / 2;
                drawTop = drawBottom + vpH * enlargeFactor;
            }

            void calculateViewportBoundaryInCenter(double enlargeFactor)
            {
                double dW = drawRight - drawLeft;
                double dH = drawTop - drawBottom;
                vpLeft = drawLeft + (dW - dW / enlargeFactor) / 2;
                vpRight = vpLeft + dW / enlargeFactor;
                vpBottom = drawBottom + (dH - dH / enlargeFactor) / 2;
                vpTop = vpBottom + dH / enlargeFactor;
            }

            // in formula units
            void scaleViewportBoundary(double centerX, double centerY, double scale)
            {
                double vpW = vpRight - vpLeft;
                double vpH = vpTop - vpBottom;
                double px = (centerX - vpLeft) / vpW;
                double py = (centerY - vpBottom) / vpH;
                vpLeft = vpLeft + vpW * px * (1 - scale);
                vpRight = vpLeft + vpW * scale;
                vpBottom = vpBottom + vpH * py * (1 - scale);
                vpTop = vpBottom + vpH * scale;
            }
        };

        typedef std::shared_ptr<OptionBag> PlotOptionsPtr;
    }
}