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

            void calculateEnlargedBounary(double enlargeFactor)
            {
                double vpW = vpRight - vpLeft;
                double vpH = vpTop - vpBottom;
                drawLeft = vpLeft - (enlargeFactor - 1) * vpW / 2;
                drawRight = drawLeft + vpW * enlargeFactor;
                drawBottom = vpBottom - (enlargeFactor - 1) * vpH / 2;
                drawTop = drawBottom + vpH * enlargeFactor;
            }
        };

        typedef std::shared_ptr<OptionBag> PlotOptionsPtr;
    }
}