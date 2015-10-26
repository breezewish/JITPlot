#pragma once

#include <windows.h>
#include <windowsx.h>
#include <string>

#include <ui/event/event.h>

namespace StupidPlot
{
    namespace UI
    {
        namespace Event
        {
            class MouseEvent : public Event
            {
            public:
                int x;
                int y;

                MouseEvent() : Event(L"MouseEvent")
                {
                }

                MouseEvent(WPARAM wParam, LPARAM lParam) : Event(L"MouseEvent")
                {
                    UNREFERENCED_PARAMETER(wParam);
                    x = GET_X_LPARAM(lParam);
                    y = GET_Y_LPARAM(lParam);
                }
            };
        }
    }
}