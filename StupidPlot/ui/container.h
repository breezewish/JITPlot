#pragma once

#include <windows.h>
#include <vector>
#include <unordered_map>

#include <ui/control/control.h>

using std::vector;
using std::unordered_map;

namespace StupidPlot
{
    namespace UI
    {
        class Container
        {
        protected:
            // all controls in the container
            vector<Control::Control *>              controls;

            // hash map to find the control from control_id
            unordered_map<int, Control::Control *>  id2control;

        public:
            Container * addControl(Control::Control * control)
            {
                controls.push_back(control);
                id2control[control->getId()] = control;
                return this;
            }

            Control::Control * getControlById(int id)
            {
                if (id2control.find(id) == id2control.end())
                {
                    return 0;
                }
                return id2control[id];
            }
        };
    }
}