#pragma once

#include <windows.h>
#include <gdiplus.h>

#include <resource.h>
#include <ui/controls/control.h>
#include <ui/controls/checkbox.h>
#include <ui/controls/canvas.h>
#include <ui/container.h>
#include <ui/layout/layoutmanager.h>
#include <ui/events/event.h>
#include <ui/events/eventmanager.h>

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