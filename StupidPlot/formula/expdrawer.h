#pragma once

#include <memory>
#include <string>
#include <intrin.h>

#include <formula/exp.h>

using std::wstring;

namespace StupidPlot
{
    namespace Formula
    {
        class ExpDrawer
        {
        protected:
            ExpressionPtr       expression;

            int                 width;
            int                 height;

            vector<double>      transformBuffer;

        public:
            ExpDrawer(const ExpressionPtr & exp)
            {
                expression = exp;
            }

            ExpDrawer(wstring _exp,
                map<wstring, double> & _constVars)
            {
                expression = ExpressionPtr(new Expression(_exp, _constVars));
            }

            void setCanvasSize(int _width, int _height)
            {
                width = _width;
                height = _height;
                transformBuffer.resize(width);
            }

            void transformFormulaYToScreenY(double bottom, double top)
            {
                // (y - (bottom)) / (top - bottom) * height
                // (XMM0 - XMM1) * XMM2
                double last = 1.0 / (top - bottom) * height;
                __m128d xmm0, xmm1, xmm2;
                xmm2 = _mm_set1_pd(last);
                xmm1 = _mm_set1_pd(bottom);
                double * pt;
                for (int i = 0, max = width; i < max; i += 2)
                {
                    pt = &transformBuffer[i];
                    xmm0 = _mm_loadu_pd(pt);
                    xmm0 = _mm_sub_pd(xmm0, xmm1);
                    xmm0 = _mm_mul_pd(xmm0, xmm2);
                    _mm_storeu_pd(pt, xmm0);
                }
                if ((width & 1) > 0)
                {
                    transformBuffer[width - 1] = (transformBuffer[width - 1] - bottom) * last;
                }
            }

            vector<double> & evalAndTransform(double xMin, double xMax, double yMin, double yMax)
            {
                auto formulaY = expression->eval(xMin, xMax, width);
                memcpy(&transformBuffer[0], &formulaY[0], width * sizeof(double));
                transformFormulaYToScreenY(yMin, yMax);
                return transformBuffer;
            }
        };

        typedef std::shared_ptr<ExpDrawer> ExpDrawerPtr;
    }
}