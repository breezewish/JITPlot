#pragma once

#include <memory>
#include <vector>
#include <string>

#include <gdiplus.h>

#include <plot/types.h>

namespace StupidPlot
{
    namespace Plot
    {
        enum class GraphicType
        {
            FORMULA_EXPRESSION,
            POINTSET_POLYLINE,
        };

        class Graphic
        {
        protected:
            int                 clipWidth;
            int                 clipHeight;
            int                 canvasPxWidth;
            int                 canvasPxHeight;
        public:
            GraphicType         type;
            Gdiplus::Color      color;

            bool                isValid;
            std::wstring        errorMessage;

            Graphic(GraphicType _type, Gdiplus::Color _color)
            {
                type = _type;
                color = _color;
            }

            virtual void setSize(int clipW, int clipH, int cW, int cH)
            {
                clipWidth = clipW;
                clipHeight = clipH;
                canvasPxWidth = cW;
                canvasPxHeight = cH;
            }

            virtual bool evalAndTransform(
                double xMin,
                double xMax,
                double canvasLeft,
                double canvasRight,
                double canvasBottom,
                double canvasTop,
                int & n,
                DoubleArr & x,
                DoubleArr & y
                ) = 0;

            void batchTransformX(double canvasLeft, double canvasRight, double * buffer, int size)
            {
                // (x - left) / (right - left) * width
                // (XMM0 - XMM1) * XMM2
                double last = 1.0 / (canvasRight - canvasLeft) * canvasPxWidth;
                __m128d xmm0, xmm1, xmm2;
                xmm1 = _mm_set1_pd(canvasLeft);
                xmm2 = _mm_set1_pd(last);
                double * pt;
                for (int i = 0; i + 1 < size; i += 2)
                {
                    pt = buffer + i;
                    xmm0 = _mm_loadu_pd(pt);
                    xmm0 = _mm_sub_pd(xmm0, xmm1);
                    xmm0 = _mm_mul_pd(xmm0, xmm2);
                    _mm_storeu_pd(pt, xmm0);
                }
                if ((size & 1) > 0)
                {
                    buffer[size - 1] = (buffer[size - 1] - canvasLeft) * last;
                }
            }

            void batchTransformY(double canvasBottom, double canvasTop, double * buffer, int size)
            {
                // height - ((y - (bottom)) / (top - bottom) * height)
                // (XMM0 - XMM1) * XMM2 + XMM3
                double last = -1.0 / (canvasTop - canvasBottom) * canvasPxHeight;
                __m128d xmm0, xmm1, xmm2, xmm3;
                xmm1 = _mm_set1_pd(canvasBottom);
                xmm2 = _mm_set1_pd(last);
                xmm3 = _mm_set1_pd(canvasPxHeight);
                double * pt;
                for (int i = 0; i + 1 < size; i += 2)
                {
                    pt = buffer + i;
                    xmm0 = _mm_loadu_pd(pt);
                    xmm0 = _mm_sub_pd(xmm0, xmm1);
                    xmm0 = _mm_mul_pd(xmm0, xmm2);
                    xmm0 = _mm_add_pd(xmm0, xmm3);
                    _mm_storeu_pd(pt, xmm0);
                }
                if ((size & 1) > 0)
                {
                    buffer[size - 1] = canvasPxHeight + (buffer[size - 1] - canvasBottom) * last;
                }
            }
        };

        typedef std::shared_ptr<Graphic> GraphicPtr;
    }
}
