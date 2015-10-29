#pragma once

#include <windows.h>
#include <gdiplus.h>

namespace StupidPlot
{
    class App
    {
    public:
        static void init(HWND _hWnd);
        static void terminate();
        static void updateSize();
        static BOOL handleEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);
    };
}