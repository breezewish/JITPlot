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
            class RedrawEvent : public Event
            {
            public:
                RedrawEvent() : Event(L"RedrawEvent")
                {
                }
            };
        }
    }
}