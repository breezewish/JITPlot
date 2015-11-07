#pragma once

#include <windows.h>
#include <gdiplus.h>

#include <UIRibbon.h>

namespace StupidPlot
{
    class App
    {
    public:
        static bool hasInitialized();
        static void init(HWND _hWnd);
        static void terminate();
        static void updateSize();
        static BOOL handleEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);
        static HRESULT handleRibbonUpdateProperty(UINT, REFPROPERTYKEY, const PROPVARIANT*, PROPVARIANT*);
        static void handleRibbonExecute(UINT, UI_EXECUTIONVERB, const PROPERTYKEY*, const PROPVARIANT*, IUISimplePropertySet*);
    };
}