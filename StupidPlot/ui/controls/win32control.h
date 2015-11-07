#pragma once

#include <windows.h>

#include <ui/controls/control.h>
#include <ui/events/rawevent.h>
#include <ui/events/mouseevent.h>
#include <ui/events/mousewheelevent.h>

namespace StupidPlot
{
    namespace UI
    {
        using namespace Events;

        namespace Controls
        {
            class Win32Control : public Control
            {
            protected:
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

                    auto control = reinterpret_cast<Win32Control *>(dwRefData);
                    control->dispatchEvent(EventName::EVENT_MESSAGE, rawEvent);

                    switch (uMsg)
                    {
                    case WM_SETCURSOR:
                        control->dispatchEvent(EventName::EVENT_SETCURSOR, EventPtr(new Event()));
                        break;
                    case WM_KILLFOCUS:
                        control->dispatchEvent(EventName::EVENT_LOSING_FOCUS, EventPtr(new Event()));
                        break;
                    case WM_SETFOCUS:
                        control->dispatchEvent(EventName::EVENT_GOT_FOCUS, EventPtr(new Event()));
                        break;
                    case WM_MOUSEWHEEL:
                        control->dispatchEvent(EventName::EVENT_MOUSEWHEEL, EventPtr(new MouseWheelEvent(hWnd, wParam, lParam)));
                        break;
                    case WM_LBUTTONDOWN:
                        control->dispatchEvent(EventName::EVENT_MOUSEDOWN, EventPtr(new MouseEvent(wParam, lParam, Events::MouseButton::LEFT)));
                        break;
                    case WM_LBUTTONUP:
                        control->dispatchEvent(EventName::EVENT_MOUSEUP, EventPtr(new MouseEvent(wParam, lParam, Events::MouseButton::LEFT)));
                        break;
                    case WM_MOUSEMOVE:
                        // TODO
                        control->dispatchEvent(EventName::EVENT_MOUSEMOVE, EventPtr(new MouseEvent(wParam, lParam, Events::MouseButton::NONE)));
                        break;
                    case WM_RBUTTONDOWN:
                        control->dispatchEvent(EventName::EVENT_MOUSEDOWN, EventPtr(new MouseEvent(wParam, lParam, Events::MouseButton::RIGHT)));
                        break;
                    case WM_RBUTTONUP:
                        control->dispatchEvent(EventName::EVENT_MOUSEUP, EventPtr(new MouseEvent(wParam, lParam, Events::MouseButton::RIGHT)));
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
                HWND    hWnd;
                HDC     hDC;

                LONG    width;
                LONG    height;

                Win32Control(HWND _hWnd, int _id) : Control(_id)
                {
                    hWnd = GetDlgItem(_hWnd, id);
                    hDC = GetDC(hWnd);

                    updateSize();
                    initSubclass();
                }

                virtual ~Win32Control()
                {
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