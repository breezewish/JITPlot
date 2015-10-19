#pragma once

#include <windows.h>

#include <debug.h>

#include <ui/control/control.h>
#include <ui/event/event.h>
#include <ui/event/rawevent.h>

namespace StupidPlot
{
    namespace UI
    {
        namespace Control
        {
            class Checkbox : public Control
            {
            public:
                Checkbox(HWND _hWnd, int _id) : Control(_hWnd, _id)
                {
                }

                BOOL isChecked()
                {
                    LRESULT checkStatus = SendMessageW(getHWND(), BM_GETCHECK, 0, 0);
                    return (checkStatus == BST_CHECKED);
                }

                void setChecked(BOOL checked)
                {
                    WPARAM wParam = checked ? BST_CHECKED : BST_UNCHECKED;
                    SendMessageW(getHWND(), BM_SETCHECK, wParam, 0);
                }
            };
        }
    }
}