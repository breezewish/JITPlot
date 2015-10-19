#pragma once

#include <gdiplus.h>

#include <ui/control/control.h>
#include <ui/event/event.h>

using namespace Gdiplus;

namespace StupidPlot
{
    namespace UI
    {
        namespace Control
        {
            class Canvas : public Control
            {
            public:

                Canvas(HWND _hWnd, int _id) : Control(_hWnd, _id, true)
                {
                    addEventHandler(Event::EVENT_REDRAW, onRedraw);
                    redraw();
                }

                static void onRedraw(Control * _control, Event::Event *_event)
                {
                    UNREFERENCED_PARAMETER(_event);

                    Canvas * canvas = dynamic_cast<Canvas *>(_control);
                    canvas->redraw();
                }

                void redraw()
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
    }
}