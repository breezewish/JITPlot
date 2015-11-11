#pragma once

#include <windows.h>
#include <Uxtheme.h>

#include <memory>
#include <string>

#include <ui/events/notifyevent.h>
#include <ui/events/listviewendeditevent.h>
#include <ui/events/customdrawevent.h>
#include <ui/controls/control.h>
#include <ui/controls/win32control.h>

namespace StupidPlot
{
    namespace UI
    {
        namespace Controls
        {
            class ListView : public Win32Control
            {
            protected:
                static LRESULT onNotify(Control * _control, const EventPtr & _event)
                {
                    auto event = std::dynamic_pointer_cast<NotifyEvent>(_event);
                    auto listview = dynamic_cast<ListView *>(_control);

                    switch (event->nmh.code)
                    {
                    case LVN_ENDLABELEDIT:
                        return listview->dispatchEvent(EventName::EVENT_LISTVIEW_ENDEDIT, EventPtr(new ListViewEndEditEvent(event->lParam)));
                    case NM_CUSTOMDRAW:
                        return listview->dispatchEvent(EventName::EVENT_CUSTOMDRAW, EventPtr(new CustomDrawEvent(event->lParam)));
                    }

                    return LRESULT_DEFAULT;
                }

            public:
                ListView(HWND _hWnd, int _id) : Win32Control(_hWnd, _id)
                {
                    ListView_SetExtendedListViewStyle(hControl, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
                    SetWindowTheme(hControl, L"Explorer", NULL);

                    addEventHandler(EventName::EVENT_NOTIFY, onNotify);
                }

                size_t getColumns()
                {
                    auto header = ListView_GetHeader(hControl);
                    return static_cast<size_t>(Header_GetItemCount(header));
                }

                size_t getRows()
                {
                    return static_cast<size_t>(ListView_GetItemCount(hControl));
                }

                size_t insertColumn(wstring name, int width, int index = -1)
                {
                    LVCOLUMNW col;

                    if (index == -1) index = getColumns();

                    col.pszText = &name[0];
                    col.cx = width;
                    col.mask = LVCF_TEXT | LVCF_WIDTH;

                    ListView_InsertColumn(hControl, index, &col);

                    return index;
                }

                size_t insertRow(wstring value, int index = -1)
                {
                    LVITEMW row;

                    if (index == -1) index = getRows();

                    row.pszText = &value[0];
                    row.iItem = index;
                    row.iSubItem = 0;
                    row.mask = LVIF_TEXT;

                    ListView_InsertItem(hControl, &row);

                    return index;
                }

                void beginEdit(int row)
                {
                    SetFocus(hControl);
                    ListView_EditLabel(hControl, row);
                }

                void cancelEdit()
                {
                    ListView_EditLabel(hControl, -1);
                }

                void getRect(int index, RECT * rect, int code)
                {
                    ListView_GetItemRect(hControl, index, rect, code);
                }

                void setSelected(int index, bool selected)
                {
                    if (selected)
                    {
                        ListView_SetItemState(hControl, index, LVIS_SELECTED, LVIS_SELECTED);
                    }
                    else
                    {
                        ListView_SetItemState(hControl, index, 0, LVIS_SELECTED);
                    }
                }

                int getSelectedIndex()
                {
                    return ListView_GetNextItem(hControl, -1, LVNI_SELECTED);
                }

                void deleteRow(int index)
                {
                    ListView_DeleteItem(hControl, index);
                }
            };

            typedef std::shared_ptr<ListView> ListViewPtr;
        }
    }
}