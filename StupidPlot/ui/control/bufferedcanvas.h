#pragma once

#include <windows.h>
#include <gdiplus.h>

#include <ui/control/control.h>

using namespace Gdiplus;

namespace StupidPlot
{
    namespace UI
    {
        namespace Control
        {
            const int BUFFER_ENLARGE = 2;

            class BufferedCanvas : public Control
            {
            protected:
                Graphics        g;
                Bitmap          * memBitmap = NULL;

                void destroyBuffer()
                {
                    if (memBitmap != NULL) delete memBitmap;
                }

                void updateOrCreateBuffer()
                {
                    destroyBuffer();
                    memBitmap = new Bitmap(width * BUFFER_ENLARGE, height * BUFFER_ENLARGE, &g);
                }

            public:
                BufferedCanvas(HWND _hWnd, int _id) : Control(_hWnd, _id), g(_hWnd)
                {
                    updateOrCreateBuffer();
                }

                ~BufferedCanvas()
                {
                    destroyBuffer();
                }
            };
        }
    }
}