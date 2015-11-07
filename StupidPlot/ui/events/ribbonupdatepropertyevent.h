#pragma once

#include <UIRibbon.h>

#include <ui/events/event.h>

namespace StupidPlot
{
    namespace UI
    {
        namespace Events
        {
            class RibbonUpdatePropertyEvent : public Event
            {
            public:
                REFPROPERTYKEY      key;
                const PROPVARIANT * currentValue;
                PROPVARIANT *       newValue;

                RibbonUpdatePropertyEvent(
                    REFPROPERTYKEY _key,
                    const PROPVARIANT * _currentValue,
                    PROPVARIANT * _newValue)
                    : Event(EventType::RIBBON), key(_key), currentValue(_currentValue), newValue(_newValue)
                {
                }
            };
        }
    }
}