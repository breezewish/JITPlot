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

                MouseWheelEvent(WPARAM wParam, LPARAM lParam) : MouseEvent(wParam, lParam)
                {
                    delta = GET_WHEEL_DELTA_WPARAM(wParam);
                }
            };
        }
    }
}