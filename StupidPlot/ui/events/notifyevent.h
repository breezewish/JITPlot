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
            class NotifyEvent : public Event
            {
            public:
                NMHDR   nmh;
                WPARAM  wParam;
                LPARAM  lParam;

                NotifyEvent(WPARAM _wParam, LPARAM _lParam) : Event(EventType::NOTIFY)
                {
                    wParam = _wParam;
                    lParam = _lParam;
                    nmh = *(reinterpret_cast<LPNMHDR>(lParam));
                }
            };
        }
    }
}