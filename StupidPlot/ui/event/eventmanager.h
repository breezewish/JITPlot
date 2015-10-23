#pragma once

#include <windows.h>
#include <string>
#include <memory>

#include <ui/control/control.h>
#include <ui/container.h>

using std::shared_ptr;

namespace StupidPlot
{
    namespace UI
    {
        namespace Event
        {
            class EventManager
            {
            protected:
                Container * container = NULL;
            public:
                EventManager(Container * _container)
                {
                    container = _container;
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
                            control->dispatchEvent(EVENT_CLICK, shared_ptr<Event>(new MouseEvent()));
                            break;
                        }

                        break;
                    }
                    return false;
                }
            };
        }
    }
}