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
            class ListViewDblClickEvent : public Event
            {
            public:
                NMLVDISPINFOW * displayInfo;

                ListViewEndEditEvent(LPARAM lParam) : Event(EventType::LISTVIEW)
                {
                    displayInfo = reinterpret_cast<NMLVDISPINFOW *>(lParam);
                }
            };
        }
    }
}