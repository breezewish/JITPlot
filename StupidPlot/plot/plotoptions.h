#pragma once

#include <vector>
#include <memory>

#include <windows.h>
#include <gdiplus.h>

#include <formula/expression.h>

using std::shared_ptr;
using std::vector;

namespace StupidPlot
{
    namespace Plot
    {
        using Formula::ExpressionPtr;

        class PlotOptions
        {
        public:
            double                  left = -30.0;
            double                  right = 30.0;
            double                  top = 5.0;
            double                  bottom = -5.0;

            bool                    keepRatio = false;

            int                     gridSpacing = 1;

            vector<Gdiplus::Color>     formulaColors;
            vector<ExpressionPtr>      formulaObjects;
        };

        typedef std::shared_ptr<PlotOptions> PlotOptionsPtr;
    }
}