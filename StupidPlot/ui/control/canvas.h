#pragma once

#include <gdiplus.h>

#include <ui/control/control.h>
#include <ui/event/event.h>
#include <plot/plotoptions.h>
#include <plot/plotdrawer.h>
#include <formula/expression.h>

#include <map>
#include <vector>
#include <cmath>
#include <string>

using std::map;
using std::vector;
using std::wstring;

using namespace Gdiplus;

namespace StupidPlot
{
    namespace UI
    {
        namespace Control
        {
            class Canvas : public Control
            {
            protected:
                Plot::PlotOptions   * options;
                Plot::PlotDrawer    * drawer;

            public:

                Canvas(HWND _hWnd, int _id) : Control(_hWnd, _id, true)
                {
                    options = new Plot::PlotOptions();
                    drawer = new Plot::PlotDrawer(options, getDC());

                    map<wstring, double> constVars;
                    constVars[L"PI"] = 3.1415;

                    vector<wstring> dynamicVars;
                    dynamicVars.push_back(L"x");

                    // TODO: update options from UI
                    options->formulaColors.push_back(Gdiplus::Color(255, 47, 197, 255));
                    options->formulaObjects.push_back(new Formula::Expression(L"sin(x+1)+1", constVars, dynamicVars));

                    addEventHandler(Event::EVENT_REDRAW, onRedraw);
                    redraw();
                }

                ~Canvas()
                {
                    delete options->formulaObjects[0];
                    delete options;
                    delete drawer;
                }

                static void onRedraw(Control * _control, Event::Event *_event)
                {
                    UNREFERENCED_PARAMETER(_event);

                    Canvas * canvas = dynamic_cast<Canvas *>(_control);
                    canvas->redraw();
                }

                void redraw()
                {
                    drawer->draw(width, height);
                }
            };
        }
    }
}