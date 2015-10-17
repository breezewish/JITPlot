#pragma once

#include <gdiplus.h>

#include "control.h"

using namespace Gdiplus;

namespace StupidPlot
{
    class Canvas : public Control
    {
    public:
        Canvas(HWND _hWnd) : Control(_hWnd, true)
        {
        }

        virtual void redraw()
        {
            Graphics graphics(getDC());

            graphics.Clear(Color(255, 255, 255, 255));
            graphics.SetSmoothingMode(SmoothingMode::SmoothingModeAntiAlias);
            graphics.SetTextRenderingHint(TextRenderingHint::TextRenderingHintClearTypeGridFit);

            WCHAR string[] = L"Canvas";

            Font myFont(L"Segoe UI", 16);
            RectF layoutRect(0.0f, 0.0f, (float)width, (float)height);
            StringFormat format;
            format.SetAlignment(StringAlignmentFar);
            SolidBrush blackBrush(Color(255, 0, 0, 0));

            graphics.DrawString(
                string,
                -1,
                &myFont,
                layoutRect,
                &format,
                &blackBrush);
        }
    };
}