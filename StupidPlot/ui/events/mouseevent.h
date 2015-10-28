#pragma once

#include <windows.h>
#include <windowsx.h>
#include <string>

#include <ui/events/event.h>

namespace StupidPlot
{
    namespace UI
    {
        namespace Events
        {
            class MouseEvent : public Event
            {
            public:
                int x;
                int y;

                MouseEvent() : Event(EventType::MOUSE)
                {
                }

                MouseEvent(WPARAM wParam, LPARAM lParam) : Event(EventType::MOUSE)
                {
                    UNREFERENCED_PARAMETER(wParam);
                    x = GET_X_LPARAM(lParam);
                    y = GET_Y_LPARAM(lParam);
                }
            };
        }
    }
}