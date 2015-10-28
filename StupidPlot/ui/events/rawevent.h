#pragma once

#include <windows.h>
#include <string>

#include <ui/events/event.h>

namespace StupidPlot
{
    namespace UI
    {
        namespace Events
        {
            class RawEvent : public Event
            {
            public:
                UINT        uMsg;
                WPARAM      wParam;
                LPARAM      lParam;

                RawEvent(UINT _uMsg, WPARAM _wParam, LPARAM _lParam) : Event(EventType::RAW)
                {
                    uMsg = _uMsg;
                    wParam = _wParam;
                    lParam = _lParam;
                }
            };
        }
    }
}