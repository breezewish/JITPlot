#pragma once

#include <string>
#include <memory>

#include <windows.h>

using std::wstring;

namespace StupidPlot
{
    namespace UI
    {
        namespace Events
        {
            enum class EventName
            {
                EVENT_REDRAWBUFFER,
                EVENT_CLICK,
                EVENT_PAINT,
                EVENT_RESIZING,
                EVENT_RESIZE,
                EVENT_MESSAGE,
                EVENT_MOUSEDOWN,
                EVENT_MOUSEMOVE,
                EVENT_MOUSEUP,
            };

            enum class EventType
            {
                MOUSE,
                RAW,
                PAINT,
                GENERAL,
            };

            class Event
            {
            protected:
                EventType name;
            public:
                Event()
                {
                    name = EventType::GENERAL;
                }

                Event(EventType _name) : name(_name)
                {
                }

                virtual ~Event()
                {
                }

                EventType getName()
                {
                    return name;
                }
            };

            typedef std::shared_ptr<Event> EventPtr;
        }
    }
}