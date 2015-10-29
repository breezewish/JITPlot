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

            typedef void(*CONTROLEVENTHANDLER)(
                Control * control,
                const EventPtr & event
                );

            class Control
            {
            protected:
                unordered_map<EventName, vector<CONTROLEVENTHANDLER>> handlers;

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

                inline Control * addEventHandler(EventName eventName, CONTROLEVENTHANDLER handler)
                {
                    handlers[eventName].push_back(handler);
                    return this;
                }

                inline Control * dispatchEvent(EventName eventName, const EventPtr & event)
                {
                    auto registeredHandlers = handlers.find(eventName);
                    if (registeredHandlers == handlers.end())
                    {
                        return this;
                    }
                    for (CONTROLEVENTHANDLER handler : registeredHandlers->second)
                    {
                        handler(this, event);
                    }

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