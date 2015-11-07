#pragma once

#include <UIRibbon.h>

#include <ui/events/event.h>

namespace StupidPlot
{
    namespace UI
    {
        namespace Events
        {
            class RibbonExecuteEvent : public Event
            {
            public:
                UI_EXECUTIONVERB        verb;
                const PROPERTYKEY *     key;
                const PROPVARIANT *     value;
                IUISimplePropertySet *  properties;

                RibbonExecuteEvent(
                    UI_EXECUTIONVERB _verb,
                    const PROPERTYKEY * _key,
                    const PROPVARIANT * _value,
                    IUISimplePropertySet * _properties)
                    : Event(EventType::RIBBON), verb(_verb), key(_key), value(_value), properties(_properties)
                {
                }
            };
        }
    }
}