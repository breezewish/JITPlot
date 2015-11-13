#pragma once

#include <memory>
#include <string>

#include <util.h>
#include <pointset/csvloader.h>
#include <plot/graphic.h>

namespace StupidPlot
{
    namespace Pointset
    {
        class PolylineDrawer : public Plot::Graphic
        {
        public:
            std::vector<double> _x;
            std::vector<double> _y;

            PolylineDrawer(
                std::wstring csvfile,
                Gdiplus::Color _color
                )
                : Plot::Graphic(Plot::GraphicType::POINTSET_POLYLINE, _color)
            {
                try
                {
                    CSVLoader::load(csvfile, _x, _y);
                    isValid = true;
                }
                catch (std::runtime_error ex)
                {
                    errorMessage = Util::utf8_decode(ex.what());
                    isValid = false;
                }
            }

            virtual bool evalAndTransform(
                double xMin,
                double xMax,
                double canvasLeft,
                double canvasRight,
                double canvasBottom,
                double canvasTop,
                int & n,
                Plot::DoubleArr & x,
                Plot::DoubleArr & y
                )
            {
                UNREFERENCED_PARAMETER(xMin);
                UNREFERENCED_PARAMETER(xMax);

                if (!isValid) return false;

                n = _x.size();
                x = std::shared_ptr<double>(new double[n], array_deleter<double>());
                y = std::shared_ptr<double>(new double[n], array_deleter<double>());
                memcpy(x.get(), &_x[0], n * sizeof(double));
                memcpy(y.get(), &_y[0], n * sizeof(double));
                batchTransformX(canvasLeft, canvasRight, x.get(), n);
                batchTransformY(canvasBottom, canvasTop, y.get(), n);

                return true;
            }
        };
    }
}