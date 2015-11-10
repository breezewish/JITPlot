#pragma once

#include <windows.h>
#include <gdiplus.h>

#include <memory>

#include <ui/controls/control.h>
#include <ui/controls/win32control.h>
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
            class BufferedCanvas : public Win32Control
            {
            protected:
                int vpInitialX, vpInitialY;

                static LRESULT onMessage(Control * _control, const EventPtr & _event)
                {
                    auto event = std::dynamic_pointer_cast<RawEvent>(_event);
                    UNREFERENCED_PARAMETER(_control);

                    if (event->uMsg == WM_NCHITTEST)
                    {
                        return HTCLIENT;
                    }

                    return LRESULT_DEFAULT;
                }

                static LRESULT onPaint(Control * _control, const EventPtr & _event)
                {
                    UNREFERENCED_PARAMETER(_event);
                    auto canvas = dynamic_cast<BufferedCanvas *>(_control);

                    PAINTSTRUCT ps;
                    HDC hdc = BeginPaint(canvas->hControl, &ps);
                    BitBlt(hdc, 0, 0, canvas->width, canvas->height, canvas->memDC, canvas->vpX, canvas->vpY, SRCCOPY);
                    EndPaint(canvas->hControl, &ps);

                    return LRESULT_DEFAULT;
                }

                static LRESULT onResize(Control * _control, const EventPtr & _event)
                {
                    UNREFERENCED_PARAMETER(_event);
                    auto canvas = dynamic_cast<BufferedCanvas *>(_control);

                    canvas->updateOrCreateBuffer();
                    canvas->dispatchEvent(EventName::EVENT_CANVAS_RESIZE, EventPtr(new Event()));
                    canvas->dispatchRedraw();

                    return LRESULT_DEFAULT;
                }

                static LRESULT onMouseDown(Control * _control, const EventPtr & _event)
                {
                    auto canvas = dynamic_cast<BufferedCanvas *>(_control);
                    auto event = std::dynamic_pointer_cast<MouseEvent>(_event);

                    if (!canvas->canMove) return LRESULT_DEFAULT;

                    SetCapture(canvas->hControl);

                    canvas->isMoving = true;
                    canvas->mouseInitialX = event->x;
                    canvas->mouseInitialY = event->y;
                    canvas->vpInitialX = canvas->vpX;
                    canvas->vpInitialY = canvas->vpY;
                    canvas->dispatchEvent(EventName::EVENT_CANVAS_BEGINMOVE, EventPtr(new Event()));

                    return LRESULT_DEFAULT;
                }

                static LRESULT onMouseUp(Control * _control, const EventPtr & _event)
                {
                    auto canvas = dynamic_cast<BufferedCanvas *>(_control);
                    auto event = std::dynamic_pointer_cast<MouseEvent>(_event);

                    ReleaseCapture();

                    canvas->isMoving = false;
                    if (!canvas->canMove) return LRESULT_DEFAULT;

                    canvas->dispatchEvent(EventName::EVENT_CANVAS_ENDMOVE, EventPtr(new Event()));
                    canvas->resetViewportPosition();
                    canvas->dispatchRedraw();

                    return LRESULT_DEFAULT;
                }

                static LRESULT onMouseMove(Control * _control, const EventPtr & _event)
                {
                    auto canvas = dynamic_cast<BufferedCanvas *>(_control);
                    auto event = std::dynamic_pointer_cast<MouseEvent>(_event);

                    if (!canvas->isMoving) return LRESULT_DEFAULT;
                    if (!canvas->canMove) return LRESULT_DEFAULT;

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

                    return LRESULT_DEFAULT;
                }

                void destroyBuffer()
                {
                    if (oldBitmap != NULL)
                    {
                        SelectObject(memDC, oldBitmap);
                        DeleteObject(bitmap);
                        bitmap = NULL;
                        oldBitmap = NULL;
                    }
                }

                void updateOrCreateBuffer()
                {
                    canvasW = static_cast<int>(enlargeFactor * width);
                    canvasH = static_cast<int>(enlargeFactor * height);

                    destroyBuffer();
                    bitmap = CreateCompatibleBitmap(hDC, canvasW, canvasH);
                    oldBitmap = SelectObject(memDC, bitmap);

                    dispatchEvent(EventName::EVENT_CANVAS_REBUILD, EventPtr(new CanvasRebuildEvent(canvasW, canvasH)));

                    resetViewportPosition();
                }

            public:

                HDC             memDC = NULL;
                HGDIOBJ         oldBitmap = NULL;
                HBITMAP         bitmap;

                int             vpX, vpY;
                int             canvasW, canvasH;
                bool            isMoving = false;
                int             mouseInitialX, mouseInitialY;

                double          enlargeFactor;

                bool            canMove = true;

                BufferedCanvas(HWND _hWindow, int _id, double enlarge = 2.0) : Win32Control(_hWindow, _id), enlargeFactor(enlarge)
                {
                    memDC = CreateCompatibleDC(hDC);
                    updateOrCreateBuffer();

                    addEventHandler(EventName::EVENT_MESSAGE, onMessage);
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
                    InvalidateRect(hControl, NULL, false);
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