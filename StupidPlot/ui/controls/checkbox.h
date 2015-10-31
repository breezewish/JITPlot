#pragma once

#include <windows.h>

#include <memory>

#include <ui/controls/control.h>

namespace StupidPlot
{
    namespace UI
    {
        namespace Controls
        {
            class Checkbox : public Control
            {
            public:
                Checkbox(HWND _hWnd, int _id) : Control(_hWnd, _id)
                {
                }

                bool isChecked()
                {
                    LRESULT checkStatus = SendMessageW(hWnd, BM_GETCHECK, 0, 0);
                    return (checkStatus == BST_CHECKED);
                }

                Checkbox * setChecked(bool checked)
                {
                    WPARAM wParam = checked ? BST_CHECKED : BST_UNCHECKED;
                    SendMessageW(hWnd, BM_SETCHECK, wParam, 0);
                    return this;
                }
            };

            typedef std::shared_ptr<Checkbox> CheckboxPtr;
        }
    }
}