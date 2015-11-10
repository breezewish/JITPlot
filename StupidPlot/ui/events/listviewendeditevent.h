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
            class ListViewEndEditEvent : public Event
            {
            public:
                NMLVDISPINFO displayInfo;

                ListViewEndEditEvent(LPARAM lParam) : Event(EventType::LISTVIEW)
                {
                    displayInfo = *(reinterpret_cast<LPNMLVDISPINFOW>(lParam));
                }
            };
        }
    }
}