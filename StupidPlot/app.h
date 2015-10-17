#pragma once

#include <windows.h>
#include <gdiplus.h>
#include "resource.h"
#include "debug.h"
#include "control.h"
#include "canvas.h"

using namespace Gdiplus;

namespace StupidPlot
{
    class App
    {
    protected:
        GdiplusStartupInput gdiplusStartupInput;
        ULONG_PTR           gdiplusToken;

        HWND                hWnd;
        Canvas              * canvas;

    public:
        App(HWND _hWnd)
        {
            GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

            hWnd = _hWnd;
            canvas = new Canvas(GetDlgItem(hWnd, IDC_STATIC_CANVAS));

            updateSize();
        }

        ~App()
        {
            delete canvas;

            GdiplusShutdown(gdiplusToken);
        }

        void updateSize()
        {
            RECT rect;
            GetClientRect(hWnd, &rect);

            HDWP hDefer;
            hDefer = BeginDeferWindowPos(10);

            DeferWindowPos(hDefer, canvas->getHWND(), NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);

            EndDeferWindowPos(hDefer);
        }

        void paint()
        {
            // do nothing
        }
    };
}
