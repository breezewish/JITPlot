#pragma once

#include <vector>
#include <map>
#include <string>

#include <CommCtrl.h>
#include <windows.h>

#include <debug.h>
#include <ui/event/event.h>
#include <ui/event/rawevent.h>
#include <ui/event/redrawevent.h>
#include <ui/event/mouseevent.h>

using std::vector;
using std::map;
using std::string;

namespace StupidPlot
{
    namespace UI
    {
        namespace Control
        {
            class Control;

            typedef void(*CONTROLEVENTHANDLER)(
                Control * control,
                Event::Event * event
                );

            class Control
            {
            protected:
                int     id;
                HWND    hWnd;
                BOOL    enableDoubleBuffer;

                HDC     hDC;
                HDC     memDC;

                LONG    width;
                LONG    height;

                map<int, vector<CONTROLEVENTHANDLER>> handlers;

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

                    Control * control = reinterpret_cast<Control *>(dwRefData);
                    control->dispatchEvent(Event::EVENT_MESSAGE, new Event::RawEvent(uMsg, wParam, lParam));

                    switch (uMsg)
                    {
                    case WM_SIZE:
                    case WM_SIZING:
                        control->resize();
                        if (control->enableDoubleBuffer)
                        {
                            control->resizeDoubleBuffer();
                            InvalidateRect(control->hWnd, NULL, false);
                        }
                        control->dispatchEvent(Event::EVENT_REDRAW, new Event::RawEvent(uMsg, wParam, lParam));
                        break;
                    case WM_PAINT:
                        control->updateDoubleBuffer();
                        control->dispatchEvent(Event::EVENT_PAINT, new Event::RawEvent(uMsg, wParam, lParam));
                        break;
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

                void initDoubleBuffer()
                {
                    if (!enableDoubleBuffer)
                    {
                        return;
                    }

                    memDC = CreateCompatibleDC(hDC);
                    resizeDoubleBuffer();
                }

                void destroyDoubleBuffer()
                {
                    if (!enableDoubleBuffer)
                    {
                        return;
                    }

                    DeleteDC(memDC);
                    // TODO: shall we delete bitmap?
                }

                void resizeDoubleBuffer()
                {
                    if (!enableDoubleBuffer)
                    {
                        return;
                    }

                    // create new bitmap
                    HBITMAP newBitmap = CreateCompatibleBitmap(hDC, width, height);

                    // select new bitmap into memDC
                    HGDIOBJ oldBitmap = SelectObject(memDC, newBitmap);
                    DeleteObject(oldBitmap);

                    dispatchEvent(Event::EVENT_REDRAW, new Event::RedrawEvent());
                }

                void updateDoubleBuffer()
                {
                    if (!enableDoubleBuffer)
                    {
                        return;
                    }
                    BitBlt(hDC, 0, 0, width, height, memDC, 0, 0, SRCCOPY);
                }

            public:
                Control(HWND _hWnd, int _id, BOOL _enableDoubleBuffer = false)
                {
                    id = _id;
                    hWnd = GetDlgItem(_hWnd, id);

                    hDC = GetDC(hWnd);
                    enableDoubleBuffer = _enableDoubleBuffer;

                    resize();
                    initDoubleBuffer();
                    initSubclass();
                }

                virtual ~Control()
                {
                    destroyDoubleBuffer();
                }

                HDC getDC()
                {
                    if (enableDoubleBuffer)
                    {
                        return memDC;
                    }
                    return hDC;
                }

                HWND getHWND()
                {
                    return hWnd;
                }

                int getId()
                {
                    return id;
                }

                Control * addEventHandler(int eventName, CONTROLEVENTHANDLER handler)
                {
                    handlers[eventName].push_back(handler);
                    return this;
                }

                Control * dispatchEvent(int eventName, Event::Event * event)
                {
                    if (handlers.find(eventName) == handlers.end())
                    {
                        return this;
                    }
                    for (CONTROLEVENTHANDLER handler : handlers[eventName])
                    {
                        handler(this, event);
                    }

                    delete event;
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