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
            enum class EventName : size_t
            {
                EVENT_MESSAGE,
                EVENT_NOTIFY,
                EVENT_BUFFER_REDRAW,
                EVENT_CLICK,
                EVENT_PAINT,
                EVENT_RESIZING,
                EVENT_RESIZE,
                EVENT_MOUSEDOWN,
                EVENT_MOUSEMOVE,
                EVENT_MOUSEUP,
                EVENT_MOUSEWHEEL,
                EVENT_SETCURSOR,
                EVENT_CANVAS_REBUILD,
                EVENT_CANVAS_BEGINMOVE,
                EVENT_CANVAS_ENDMOVE,
                EVENT_CANVAS_MOVE,
                EVENT_CANVAS_RESIZE,
                EVENT_GOT_FOCUS,
                EVENT_LOSING_FOCUS,
                EVENT_RIBBON_UPDATE_PROPERTY,
                EVENT_RIBBON_EXECUTE,
                EVENT_LISTVIEW_ENDEDIT,

                EVENT_LAST
            };

            enum class EventType
            {
                MOUSE,
                RAW,
                NOTIFY,
                PAINT,
                GENERAL,
                LISTVIEW,
                RIBBON,
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