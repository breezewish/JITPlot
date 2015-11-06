#pragma once

#include <ui/events/event.h>

namespace StupidPlot
{
    namespace UI
    {
        namespace Events
        {
            class CanvasRebuildEvent : public Event
            {
            public:
                int canvasWidth;
                int canvasHeight;

                CanvasRebuildEvent(int _cw, int _ch) : canvasWidth(_cw), canvasHeight(_ch)
                {
                }
            };
        }
    }
}