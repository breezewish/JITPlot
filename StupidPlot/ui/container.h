#pragma once

#include <vector>
#include <unordered_map>
#include <memory>

#include <windows.h>

#include <ui/controls/win32control.h>
#include <ui/controls/ribboncontrol.h>

using std::vector;
using std::unordered_map;

namespace StupidPlot
{
    namespace UI
    {
        using namespace Controls;

        const int WIN32_CONTROLS_ID_START = 1000;
        const int WIN32_MAX_CONTROLS = 200;

        const int RIBBON_CONTROLS_ID_START = 1;
        const int RIBBON_MAX_CONTROLS = 100;

        class Container
        {
        protected:
            // all controls in the container
            vector<Win32Control *>          win32Controls;
            vector<RibbonControl *>         ribbonControls;

            // hash map to find the control from control_id
            Win32Control                    * id2Win32Control[WIN32_MAX_CONTROLS];
            RibbonControl                   * id2RibbonControl[RIBBON_MAX_CONTROLS];

        public:
            Container()
            {
                for (int i = 0; i < WIN32_MAX_CONTROLS; ++i) id2Win32Control[i] = NULL;
                for (int i = 0; i < RIBBON_MAX_CONTROLS; ++i) id2RibbonControl[i] = NULL;
            }

            inline size_t transformWin32ControlId(size_t id)
            {
                return id - WIN32_CONTROLS_ID_START;
            }

            inline size_t transformRibbonControlId(size_t id)
            {
                return id - RIBBON_CONTROLS_ID_START;
            }

            Container * addWin32Control(Win32Control * control)
            {
                win32Controls.push_back(control);
                id2Win32Control[transformWin32ControlId(control->id)] = control;
                return this;
            }

            Container * addRibbonControl(RibbonControl * control)
            {
                ribbonControls.push_back(control);
                id2RibbonControl[transformRibbonControlId(control->id)] = control;
                return this;
            }

            inline Win32Control * getWin32ControlById(int _id)
            {
                auto id = transformWin32ControlId(_id);
                if (id < 0) return NULL;
                return id2Win32Control[id];
            }

            inline RibbonControl * getRibbonControlById(int _id)
            {
                auto id = transformRibbonControlId(_id);
                if (id < 0) return NULL;
                return id2RibbonControl[id];
            }
        };

        typedef std::shared_ptr<Container> ContainerPtr;
    }
}