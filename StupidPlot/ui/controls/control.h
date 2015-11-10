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

            typedef LRESULT(*EventCallback)(
                Control * control,
                const EventPtr & event
                );

            extern LRESULT LRESULT_DEFAULT;

            struct EventListeners
            {
                vector<EventCallback> listeners;

                EventListeners()
                {
                }

                void addListener(EventCallback listener)
                {
                    listeners.push_back(listener);
                }

                inline LRESULT dispatch(Control * control, const EventPtr & ev)
                {
                    LRESULT retFinal = -1;
                    for (auto listener : listeners)
                    {
                        LRESULT ret = listener(control, ev);
                        if (ret != -1 && retFinal == -1) retFinal = ret;
                    }
                    return retFinal;
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

                inline LRESULT dispatchEvent(EventName eventName, const EventPtr & event)
                {
                    return handlers[static_cast<size_t>(eventName)].dispatch(this, event);
                }
            };
        }
    }
}