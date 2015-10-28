#pragma once

#include <windows.h>
#include <string>
#include <memory>

#include <ui/events/event.h>
#include <ui/control/control.h>
#include <ui/container.h>

using std::shared_ptr;

namespace StupidPlot
{
    namespace UI
    {
        namespace Events
        {
            class EventManager
            {
            protected:
                const ContainerPtr & container = NULL;
            public:
                EventManager(const ContainerPtr & _container) : container(_container)
                {
                }

                // Window messages
                BOOL handle(UINT uMsg, WPARAM wParam, LPARAM lParam)
                {
                    UNREFERENCED_PARAMETER(lParam);

                    switch (uMsg)
                    {
                    case WM_COMMAND:
                        // get the source control
                        int id = LOWORD(wParam);
                        Control::Control * control = container->getControlById(id);
                        if (!control) break;

                        // is mouse click?
                        WORD wmEvent = HIWORD(wParam);
                        switch (wmEvent)
                        {
                        case BN_CLICKED:
                            control->dispatchEvent(EventName::EVENT_CLICK, EventPtr(new MouseEvent()));
                            break;
                        }

                        break;
                    }
                    return false;
                }
            };

            typedef std::shared_ptr<EventManager> EventManagerPtr;
        }
    }
}