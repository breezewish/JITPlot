#pragma once

#include <CommCtrl.h>
#include <windows.h>

#include "debug.h"

namespace StupidPlot
{
    class Control
    {
    protected:
        HWND    hWnd;
        BOOL    enableDoubleBuffer;

        HDC     hDC;
        HDC     memDC;

        LONG    width;
        LONG    height;

        static LRESULT CALLBACK doubleBufferProc(
            HWND hWnd,
            UINT uMsg,
            WPARAM wParam,
            LPARAM lParam,
            UINT_PTR uIdSubclass,
            DWORD_PTR dwRefData
            )
        {
            UNREFERENCED_PARAMETER(uIdSubclass);

            Control * control = reinterpret_cast<Control *>(dwRefData);

            switch (uMsg)
            {
            case WM_SIZE:
            case WM_SIZING:
                control->resize();
                control->resizeDoubleBuffer();
                InvalidateRect(control->hWnd, NULL, false);
                break;
            case WM_PAINT:
                BitBlt(control->hDC, 0, 0, control->width, control->height, control->memDC, 0, 0, SRCCOPY);
                break;
            }

            return DefSubclassProc(hWnd, uMsg, wParam, lParam);
        }

        void resize()
        {
            RECT rect;
            GetWindowRect(hWnd, &rect);

            width = rect.right - rect.left;
            height = rect.bottom - rect.top;
        }

        void initDoubleBuffer()
        {
            if (!enableDoubleBuffer)
            {
                return;
            }

            memDC = CreateCompatibleDC(hDC);
            resizeDoubleBuffer();

            SetWindowSubclass(hWnd, doubleBufferProc, 0, reinterpret_cast<DWORD_PTR>(this));
        }

        void destroyDoubleBuffer()
        {
            if (!enableDoubleBuffer)
            {
                return;
            }

            DeleteDC(memDC);
            // TODO: shall we delete bitmap?
        }

        void resizeDoubleBuffer()
        {
            if (!enableDoubleBuffer)
            {
                return;
            }

            // create new bitmap
            HBITMAP newBitmap = CreateCompatibleBitmap(hDC, width, height);

            // select new bitmap into memDC
            HGDIOBJ oldBitmap = SelectObject(memDC, newBitmap);
            DeleteObject(oldBitmap);

            redraw();
        }

    public:
        Control(HWND _hWnd, BOOL _enableDoubleBuffer = false)
        {
            hWnd = _hWnd;
            hDC = GetDC(hWnd);
            enableDoubleBuffer = _enableDoubleBuffer;
            resize();
            initDoubleBuffer();
        }

        ~Control()
        {
            destroyDoubleBuffer();
        }

        HDC getDC()
        {
            if (enableDoubleBuffer)
            {
                return memDC;
            }
            return hDC;
        }

        HWND getHWND()
        {
            return hWnd;
        }

        virtual void redraw()
        {
            Debug::output(L"control.redraw\n");
        }
    };
}