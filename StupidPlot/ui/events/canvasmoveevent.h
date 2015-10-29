#pragma once

#include <ui/events/event.h>

namespace StupidPlot
{
    namespace UI
    {
        namespace Events
        {
            class CanvasMoveEvent : public Event
            {
            public:
                int dx;
                int dy;
                CanvasMoveEvent(int _dx, int _dy) : dx(_dx), dy(_dy)
                {
                }
            };
        }
    }
}