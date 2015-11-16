#pragma once

#include <windows.h>
#include <CommCtrl.h>

#include <ui/events/event.h>

namespace StupidPlot
{
    namespace UI
    {
        namespace Events
        {
            enum class LabelEditType
            {
                BEGIN_EDIT,
                END_EDIT
            };

            class ListViewEditLabelEvent : public Event
            {
            public:
                NMLVDISPINFOW * displayInfo;
                LabelEditType type;

                ListViewEditLabelEvent(LabelEditType _type, LPARAM lParam)
                    : Event(EventType::LISTVIEW), type(_type)
                {
                    displayInfo = reinterpret_cast<NMLVDISPINFOW *>(lParam);
                }
            };
        }
    }
}