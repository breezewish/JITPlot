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

            int                 clipWidth;
            int                 clipHeight;
            int                 canvasWidth;
            int                 canvasHeight;

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

            void setSize(int clipW, int clipH, int cW, int cH)
            {
                clipWidth = clipW;
                clipHeight = clipH;
                canvasWidth = cW;
                canvasHeight = cH;
                transformBuffer.resize(clipWidth);
            }

            void transformFormulaYToScreenY(double canvasBottom, double canvasTop)
            {
                // height - ((y - (bottom)) / (top - bottom) * height)
                // (XMM0 - XMM1) * XMM2 + XMM3
                double last = -1.0 / (canvasTop - canvasBottom) * canvasHeight;
                __m128d xmm0, xmm1, xmm2, xmm3;
                xmm1 = _mm_set1_pd(canvasBottom);
                xmm2 = _mm_set1_pd(last);
                xmm3 = _mm_set1_pd(canvasHeight);
                double * pt;
                for (int i = 0, max = clipWidth; i < max; i += 2)
                {
                    pt = &transformBuffer[i];
                    xmm0 = _mm_loadu_pd(pt);
                    xmm0 = _mm_sub_pd(xmm0, xmm1);
                    xmm0 = _mm_mul_pd(xmm0, xmm2);
                    xmm0 = _mm_add_pd(xmm0, xmm3);
                    _mm_storeu_pd(pt, xmm0);
                }
                if ((clipWidth & 1) > 0)
                {
                    transformBuffer[clipWidth - 1] = canvasHeight + (transformBuffer[clipWidth - 1] - canvasBottom) * last;
                }
            }

            vector<double> & evalAndTransform(double xMin, double xMax, double cMin, double cMax)
            {
                auto formulaY = expression->eval(xMin, xMax, clipWidth);
                memcpy(&transformBuffer[0], &formulaY[0], clipWidth * sizeof(double));
                transformFormulaYToScreenY(cMin, cMax);
                return transformBuffer;
            }
        };

        typedef std::shared_ptr<ExpDrawer> ExpDrawerPtr;
    }
}