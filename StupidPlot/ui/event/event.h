#pragma once

#include <windows.h>
#include <string>

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

            class Event
            {
            protected:
                std::wstring name;

            public:
                Event(std::wstring _name)
                {
                    name = _name;
                }

                virtual ~Event()
                {
                }

                std::wstring toString()
                {
                    return name;
                }
            };
        }
    }
}