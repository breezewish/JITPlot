#pragma once

#include <windows.h>
#include <CommCtrl.h>

#include <ui/events/event.h>

namespace StupidPlot
{
    namespace UI
    {
        namespace Events
        {
            class CustomDrawEvent : public Event
            {
            public:
                NMLVCUSTOMDRAW * customDraw;

                CustomDrawEvent(LPARAM lParam) : Event(EventType::CUSTOMDRAW)
                {
                    customDraw = reinterpret_cast<NMLVCUSTOMDRAW *>(lParam);
                }
            };
        }
    }
}