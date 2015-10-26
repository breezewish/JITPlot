#pragma once

#include <windows.h>
#include <string>

using std::wstring;

namespace StupidPlot
{
    namespace UI
    {
        namespace Event
        {
            static const int EVENT_REDRAW = 0;
            static const int EVENT_CLICK = 1;
            static const int EVENT_PAINT = 2;
            static const int EVENT_RESIZE = 3;
            static const int EVENT_MESSAGE = 4;
            static const int EVENT_MOUSEDOWN = 5;
            static const int EVENT_MOUSEMOVE = 6;
            static const int EVENT_MOUSEUP = 7;

            class Event
            {
            protected:
                wstring name;

            public:
                Event(wstring _name)
                {
                    name = _name;
                }

                virtual ~Event()
                {
                }

                wstring toString()
                {
                    return name;
                }
            };
        }
    }
}