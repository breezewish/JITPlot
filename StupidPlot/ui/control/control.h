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

        namespace Control
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
                        control->dispatchEvent(EventName::EVENT_RESIZING, EventPtr(new Event()));
                        /*
                    case WM_SIZE:
                    case WM_SIZING:
                        control->resize();
                        if (control->enableDoubleBuffer)
                        {
                            control->resizeDoubleBuffer();
                            InvalidateRect(control->hWnd, NULL, false);
                        }
                        control->dispatchEvent(Event::EVENT_REDRAW, Event::EventPtr(new Event::RawEvent(uMsg, wParam, lParam)));
                        break;
                    case WM_PAINT:
                        control->updateDoubleBuffer();
                        control->dispatchEvent(Event::EVENT_PAINT, Event::EventPtr(new Event::RawEvent(uMsg, wParam, lParam)));
                        break;
                        */
                    }

                    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
                }

                void initSubclass()
                {
                    SetWindowSubclass(hWnd, ctrlProc, 0, reinterpret_cast<DWORD_PTR>(this));
                }

                void resize()
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

                    resize();
                    initSubclass();
                }

                virtual ~Control()
                {
                }

                Control * addEventHandler(EventName eventName, CONTROLEVENTHANDLER handler)
                {
                    handlers[eventName].push_back(handler);
                    return this;
                }

                Control * dispatchEvent(EventName eventName, const EventPtr & event)
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

                Control * setEnabled(BOOL enable)
                {
                    EnableWindow(hWnd, enable);
                    return this;
                }

                BOOL isEnabled()
                {
                    return IsWindowEnabled(hWnd);
                }
            };
        }
    }
}