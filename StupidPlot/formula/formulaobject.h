#pragma once

#include <cmath>

namespace StupidPlot
{
    namespace Formula
    {
        class FormulaObject
        {
        public:
            double evaluate(double x)
            {
                return std::sin(x);
            }
        };
    }
}