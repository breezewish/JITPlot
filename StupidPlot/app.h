#pragma once

#include <windows.h>
#include <gdiplus.h>

#include <resource.h>
#include <debug.h>
#include <ui/control/control.h>
#include <ui/control/checkbox.h>
#include <ui/control/canvas.h>
#include <ui/container.h>
#include <ui/layout/layoutmanager.h>
#include <ui/event/event.h>
#include <ui/event/eventmanager.h>

using namespace Gdiplus;
using namespace StupidPlot::UI;

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