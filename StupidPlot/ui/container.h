#pragma once

#include <vector>
#include <unordered_map>
#include <memory>

#include <windows.h>

#include <ui/controls/control.h>

using std::vector;
using std::unordered_map;

namespace StupidPlot
{
    namespace UI
    {
        using namespace Controls;

        const int CONTROLS_ID_START = 1000;
        const int MAX_CONTROLS = 50;

        class Container
        {
        protected:
            // all controls in the container
            vector<Control *>              controls;

            // hash map to find the control from control_id
            Control                        * id2control[MAX_CONTROLS];

        public:
            Container()
            {
                for (int i = 0; i < MAX_CONTROLS; ++i)
                {
                    id2control[i] = NULL;
                }
            }

            inline size_t transformControlId(size_t id)
            {
                return id - CONTROLS_ID_START;
            }

            Container * addControl(Control * control)
            {
                controls.push_back(control);
                id2control[transformControlId(control->id)] = control;
                return this;
            }

            inline Control * getControlById(int id)
            {
                return id2control[transformControlId(id)];
            }
        };

        typedef std::shared_ptr<Container> ContainerPtr;
    }
}