#pragma once

#include <windows.h>
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
                MouseEvent() : Event(L"MouseEvent")
                {
                }
            };
        }
    }
}