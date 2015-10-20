#pragma once

#include <vector>
#include <windows.h>
#include <gdiplus.h>

#include <formula/formulaobject.h>

using std::vector;
using StupidPlot::Formula::FormulaObject;

namespace StupidPlot
{
    namespace Plot
    {
        class PlotOptions
        {
        public:
            double                  left = -30.0;
            double                  right = 30.0;
            double                  top = 5.0;
            double                  bottom = -5.0;

            bool                    keepRatio = false;

            double                  gridSpacing = 10;

            vector<Gdiplus::Color>  formulaColors;
            vector<FormulaObject *> formulaObjects;
        };
    }
}