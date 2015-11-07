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
            enum class MouseButton
            {
                NONE,
                LEFT,
                RIGHT,
            };

            class MouseEvent : public Event
            {
            public:
                int x;
                int y;
                MouseButton button;

                MouseEvent() : Event(EventType::MOUSE)
                {
                }

                MouseEvent(WPARAM wParam, LPARAM lParam, MouseButton _button) : Event(EventType::MOUSE)
                {
                    UNREFERENCED_PARAMETER(wParam);
                    x = GET_X_LPARAM(lParam);
                    y = GET_Y_LPARAM(lParam);
                    button = _button;
                }
            };
        }
    }
}