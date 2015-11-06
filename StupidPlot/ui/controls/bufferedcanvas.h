#pragma once

#include <windows.h>
#include <gdiplus.h>

#include <memory>

#include <ui/controls/control.h>
#include <ui/events/event.h>
#include <ui/events/canvasmoveevent.h>
#include <ui/events/canvasrebuildevent.h>

using namespace Gdiplus;

namespace StupidPlot
{
    namespace UI
    {
        namespace Controls
        {
            class BufferedCanvas : public Control
            {
            protected:
                int vpInitialX, vpInitialY;

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
                    UNREFERENCED_PARAMETER(dwRefData);

                    switch (uMsg)
                    {
                    case WM_NCHITTEST:
                        return HTCLIENT;
                    }

                    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
                }

                static void onPaint(Control * _control, const EventPtr & _event)
                {
                    UNREFERENCED_PARAMETER(_event);
                    auto canvas = dynamic_cast<BufferedCanvas *>(_control);

                    PAINTSTRUCT ps;
                    HDC hdc = BeginPaint(canvas->hWnd, &ps);
                    BitBlt(hdc, 0, 0, canvas->width, canvas->height, canvas->memDC, canvas->vpX, canvas->vpY, SRCCOPY);
                    EndPaint(canvas->hWnd, &ps);
                }

                static void onResize(Control * _control, const EventPtr & _event)
                {
                    UNREFERENCED_PARAMETER(_event);
                    auto canvas = dynamic_cast<BufferedCanvas *>(_control);

                    canvas->updateOrCreateBuffer();
                    canvas->dispatchEvent(EventName::EVENT_CANVAS_RESIZE, EventPtr(new Event()));
                    canvas->dispatchRedraw();
                }

                static void onMouseDown(Control * _control, const EventPtr & _event)
                {
                    auto canvas = dynamic_cast<BufferedCanvas *>(_control);
                    auto event = std::dynamic_pointer_cast<MouseEvent>(_event);

                    if (!canvas->canMove) return;

                    SetCapture(_control->hWnd);

                    canvas->isMoving = true;
                    canvas->mouseInitialX = event->x;
                    canvas->mouseInitialY = event->y;
                    canvas->vpInitialX = canvas->vpX;
                    canvas->vpInitialY = canvas->vpY;
                    canvas->dispatchEvent(EventName::EVENT_CANVAS_BEGINMOVE, EventPtr(new Event()));
                }

                static void onMouseUp(Control * _control, const EventPtr & _event)
                {
                    auto canvas = dynamic_cast<BufferedCanvas *>(_control);
                    auto event = std::dynamic_pointer_cast<MouseEvent>(_event);

                    ReleaseCapture();

                    canvas->isMoving = false;
                    if (!canvas->canMove) return;

                    canvas->dispatchEvent(EventName::EVENT_CANVAS_ENDMOVE, EventPtr(new Event()));
                    canvas->resetViewportPosition();
                    canvas->dispatchRedraw();
                }

                static void onMouseMove(Control * _control, const EventPtr & _event)
                {
                    auto canvas = dynamic_cast<BufferedCanvas *>(_control);
                    auto event = std::dynamic_pointer_cast<MouseEvent>(_event);

                    if (!canvas->isMoving) return;
                    if (!canvas->canMove) return;

                    int dx = event->x - canvas->mouseInitialX;
                    int dy = event->y - canvas->mouseInitialY;

                    canvas->vpX = canvas->vpInitialX - dx;
                    canvas->vpY = canvas->vpInitialY - dy;
                    canvas->refresh();

                    canvas->dispatchEvent(EventName::EVENT_CANVAS_MOVE, EventPtr(new CanvasMoveEvent(dx, dy)));

                    if (
                        canvas->vpX <= 0
                        || canvas->vpY <= 0
                        || canvas->vpX >= canvas->canvasW - canvas->width
                        || canvas->vpY >= canvas->canvasH - canvas->height)
                    {
                        onMouseUp(_control, _event);
                        onMouseDown(_control, _event);
                    }
                }

                void destroyBuffer()
                {
                    if (oldBitmap != NULL)
                    {
                        HGDIOBJ buf = SelectObject(memDC, oldBitmap);
                        DeleteObject(buf);
                        oldBitmap = NULL;
                    }
                }

                void updateOrCreateBuffer()
                {
                    canvasW = static_cast<int>(enlargeFactor * width);
                    canvasH = static_cast<int>(enlargeFactor * height);

                    destroyBuffer();
                    HBITMAP buf = CreateCompatibleBitmap(hDC, canvasW, canvasH);
                    oldBitmap = SelectObject(memDC, buf);

                    dispatchEvent(EventName::EVENT_CANVAS_REBUILD, EventPtr(new CanvasRebuildEvent(canvasW, canvasH)));

                    resetViewportPosition();
                }

                inline void initSubclass()
                {
                    SetWindowSubclass(hWnd, ctrlProc, 0, reinterpret_cast<DWORD_PTR>(this));
                }

            public:

                HDC             memDC = NULL;
                HGDIOBJ         oldBitmap = NULL;

                int             vpX, vpY;
                int             canvasW, canvasH;
                bool            isMoving = false;
                int             mouseInitialX, mouseInitialY;

                double          enlargeFactor;

                bool            canMove = true;

                BufferedCanvas(HWND _hWnd, int _id, double enlarge = 2.0) : Control(_hWnd, _id), enlargeFactor(enlarge)
                {
                    memDC = CreateCompatibleDC(hDC);
                    updateOrCreateBuffer();

                    initSubclass();

                    addEventHandler(EventName::EVENT_PAINT, onPaint);
                    addEventHandler(EventName::EVENT_MOUSEDOWN, onMouseDown);
                    addEventHandler(EventName::EVENT_MOUSEUP, onMouseUp);
                    addEventHandler(EventName::EVENT_MOUSEMOVE, onMouseMove);
                    addEventHandler(EventName::EVENT_RESIZE, onResize);
                }

                ~BufferedCanvas()
                {
                    destroyBuffer();
                    DeleteDC(memDC);
                }

                void refresh()
                {
                    InvalidateRect(hWnd, NULL, false);
                }

                void dispatchRedraw()
                {
                    dispatchEvent(EventName::EVENT_BUFFER_REDRAW, EventPtr(new Event()));
                    refresh();
                }

                void resetViewportPosition()
                {
                    vpX = (canvasW - width) >> 1;
                    vpY = (canvasH - height) >> 1;
                }
            };

            typedef std::shared_ptr<BufferedCanvas> BufferedCanvasPtr;
        }
    }
}