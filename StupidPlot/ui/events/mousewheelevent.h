#pragma once

#include <windows.h>
#include <windowsx.h>
#include <string>

#include <ui/events/event.h>
#include <ui/events/mouseevent.h>

namespace StupidPlot
{
    namespace UI
    {
        namespace Events
        {
            class MouseWheelEvent : public MouseEvent
            {
            public:
                int delta;

                MouseWheelEvent(HWND hWnd, WPARAM wParam, LPARAM lParam) : MouseEvent(wParam, lParam, MouseButton::NONE)
                {
                    delta = GET_WHEEL_DELTA_WPARAM(wParam);

                    // coordinates here is relative to desktop
                    POINT pt;
                    pt.x = x;
                    pt.y = y;
                    ScreenToClient(hWnd, &pt);
                    x = pt.x;
                    y = pt.y;
                }
            };
        }
    }
}