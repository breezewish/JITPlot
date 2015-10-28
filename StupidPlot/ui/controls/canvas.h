#pragma once

#include <gdiplus.h>

#include <ui/controls/control.h>
#include <ui/events/event.h>
#include <plot/plotoptions.h>
#include <plot/plotdrawer.h>
#include <formula/expdrawer.h>

#include <map>
#include <vector>
#include <string>
#include <memory>

using std::map;
using std::vector;
using std::wstring;
using std::shared_ptr;

using namespace Gdiplus;

namespace StupidPlot
{
    using Formula::ExpDrawer;
    using Formula::ExpDrawerPtr;

    namespace UI
    {
        using namespace Events;

        namespace Controls
        {
            class Canvas : public Control
            {
            protected:
                double              sl, st, sr, sb;
                int                 sx, sy;
                bool                moving;

                Plot::PlotOptionsPtr options;
                Plot::PlotDrawerPtr  drawer;

            public:

                Canvas(HWND _hWnd, int _id) : Control(_hWnd, _id)
                {
                    options = Plot::PlotOptionsPtr(new Plot::PlotOptions());
                    drawer = Plot::PlotDrawerPtr(new Plot::PlotDrawer(options, hDC));

                    // TODO: update options from UI
                    map<wstring, double> constants;
                    constants[L"PI"] = 3.1415927;

                    options->formulaColors.push_back(Gdiplus::Color(255, 47, 197, 255));
                    options->formulaObjects.push_back(ExpDrawerPtr(new ExpDrawer(L"1/x", constants)));

                    addEventHandler(EventName::EVENT_REDRAWBUFFER, onRedraw);
                    addEventHandler(EventName::EVENT_MOUSEDOWN, onMouseDown);
                    addEventHandler(EventName::EVENT_MOUSEUP, onMouseUp);
                    addEventHandler(EventName::EVENT_MOUSEMOVE, onMouseMove);

                    redraw();
                }

                static void onRedraw(Control * _control, const EventPtr & _event)
                {
                    UNREFERENCED_PARAMETER(_event);

                    Canvas * canvas = dynamic_cast<Canvas *>(_control);
                    canvas->redraw();
                }

                static void onMouseDown(Control * _control, const EventPtr & _event)
                {
                    auto canvas = dynamic_cast<Canvas *>(_control);
                    auto event = std::dynamic_pointer_cast<MouseEvent>(_event);

                    SetCapture(_control->hWnd);

                    canvas->moving = true;
                    canvas->sl = canvas->options->left;
                    canvas->st = canvas->options->top;
                    canvas->sr = canvas->options->right;
                    canvas->sb = canvas->options->bottom;
                    canvas->sx = event->x;
                    canvas->sy = event->y;
                }

                static void onMouseUp(Control * _control, const EventPtr & _event)
                {
                    auto canvas = dynamic_cast<Canvas *>(_control);
                    auto event = std::dynamic_pointer_cast<MouseEvent>(_event);

                    canvas->moving = false;

                    ReleaseCapture();
                }

                static void onMouseMove(Control * _control, const EventPtr & _event)
                {
                    auto canvas = dynamic_cast<Canvas *>(_control);
                    auto event = std::dynamic_pointer_cast<MouseEvent>(_event);

                    if (!canvas->moving) return;

                    int dx = event->x - canvas->sx;
                    int dy = event->y - canvas->sy;
                    double dmx = canvas->drawer->translateCanvasW(dx);
                    double dmy = canvas->drawer->translateCanvasH(dy);

                    canvas->options->left = canvas->sl - dmx;
                    canvas->options->right = canvas->sr - dmx;
                    canvas->options->top = canvas->st - dmy;
                    canvas->options->bottom = canvas->sb - dmy;
                    canvas->redraw();
                }

                void redraw()
                {
                    drawer->draw(width, height);
                    //updateDoubleBuffer();
                }
            };
        }
    }
}