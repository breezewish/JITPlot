#pragma once

#include <windows.h>
#include <plot/provider/provider.h>

namespace StupidPlot
{
    namespace Plot
    {
        namespace Provider
        {
            class GdiProvider : public Provider
            {
            public:
                GdiProvider(HDC _hdc) : Provider(_hdc)
                {
                }
            };
        }
    }
}