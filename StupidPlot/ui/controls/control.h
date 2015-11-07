#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

#include <CommCtrl.h>
#include <windows.h>

#include <util.h>
#include <ui/events/event.h>

using std::vector;
using std::unordered_map;
using std::shared_ptr;

namespace StupidPlot
{
    namespace UI
    {
        using namespace Events;

        namespace Controls
        {
            class Control;

            typedef std::shared_ptr<Control> ControlPtr;

            typedef void(*EventCallback)(
                Control * control,
                const EventPtr & event
                );

            struct EventListeners
            {
                int n;
                EventCallback listeners[10];

                EventListeners()
                {
                    n = 0;
                }

                void addListener(EventCallback listener)
                {
                    listeners[n] = listener;
                    n++;
                }

                inline void dispatch(Control * control, const EventPtr & ev)
                {
                    for (int i = 0; i < n; ++i)
                    {
                        listeners[i](control, ev);
                    }
                }
            };

            class Control
            {
            protected:
                EventListeners handlers[static_cast<size_t>(EventName::EVENT_LAST)];

            public:
                int id;

                Control(int _id)
                {
                    id = _id;
                }

                virtual ~Control()
                {
                }

                inline Control * addEventHandler(EventName eventName, EventCallback handler)
                {
                    handlers[static_cast<size_t>(eventName)].addListener(handler);
                    return this;
                }

                inline Control * dispatchEvent(EventName eventName, const EventPtr & event)
                {
                    handlers[static_cast<size_t>(eventName)].dispatch(this, event);
                    return this;
                }
            };
        }
    }
}