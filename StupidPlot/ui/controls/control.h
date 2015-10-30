#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

#include <CommCtrl.h>
#include <windows.h>

#include <util.h>
#include <ui/events/event.h>
#include <ui/events/rawevent.h>
#include <ui/events/mouseevent.h>
#include <ui/events/mousewheelevent.h>

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

            typedef void(*EventCallback)(
                Control * control,
                const EventPtr & event
                );

            struct EventListeners
            {
                int n;
                EventCallback listeners[20];

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

                static LRESULT CALLBACK ctrlProc(
                    HWND hWnd,
                    UINT uMsg,
                    WPARAM wParam,
                    LPARAM lParam,
                    UINT_PTR uIdSubclass,
                    DWORD_PTR dwRefData
                    )
                {
                    UNREFERENCED_PARAMETER(uIdSubclass);

                    auto rawEvent = EventPtr(new RawEvent(uMsg, wParam, lParam));

                    auto control = reinterpret_cast<Control *>(dwRefData);
                    control->dispatchEvent(EventName::EVENT_MESSAGE, rawEvent);

                    switch (uMsg)
                    {
                    case WM_NCHITTEST:
                        return HTCLIENT;
                    case WM_MOUSEWHEEL:
                        control->dispatchEvent(EventName::EVENT_MOUSEWHEEL, EventPtr(new MouseWheelEvent(wParam, lParam, hWnd)));
                        break;
                    case WM_LBUTTONDOWN:
                        control->dispatchEvent(EventName::EVENT_MOUSEDOWN, EventPtr(new MouseEvent(wParam, lParam)));
                        break;
                    case WM_LBUTTONUP:
                        control->dispatchEvent(EventName::EVENT_MOUSEUP, EventPtr(new MouseEvent(wParam, lParam)));
                        break;
                    case WM_MOUSEMOVE:
                        control->dispatchEvent(EventName::EVENT_MOUSEMOVE, EventPtr(new MouseEvent(wParam, lParam)));
                        break;
                    case WM_SIZE:
                        control->updateSize();
                        control->dispatchEvent(EventName::EVENT_RESIZE, EventPtr(new Event()));
                        break;
                    case WM_PAINT:
                        control->dispatchEvent(EventName::EVENT_PAINT, EventPtr(new Event()));
                        break;
                    }

                    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
                }

                inline void initSubclass()
                {
                    SetWindowSubclass(hWnd, ctrlProc, 0, reinterpret_cast<DWORD_PTR>(this));
                }

                inline void updateSize()
                {
                    RECT rect;
                    GetWindowRect(hWnd, &rect);

                    width = rect.right - rect.left;
                    height = rect.bottom - rect.top;
                }

            public:

                int     id;
                HWND    hWnd;
                HDC     hDC;

                LONG    width;
                LONG    height;

                Control(HWND _hWnd, int _id)
                {
                    id = _id;
                    hWnd = GetDlgItem(_hWnd, id);
                    hDC = GetDC(hWnd);

                    updateSize();
                    initSubclass();
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

                inline Control * setEnabled(BOOL enable)
                {
                    EnableWindow(hWnd, enable);
                    return this;
                }

                inline BOOL isEnabled()
                {
                    return IsWindowEnabled(hWnd);
                }
            };
        }
    }
}