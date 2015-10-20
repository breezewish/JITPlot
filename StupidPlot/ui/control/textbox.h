#pragma once

#include <string>
#include <windows.h>

#include <ui/control/control.h>

using std::wstring;

namespace StupidPlot
{
    namespace UI
    {
        namespace Control
        {
            class Textbox : public Control
            {
            public:
                Textbox(HWND _hWnd, int _id) : Control(_hWnd, _id)
                {
                }

                wstring getText()
                {
                    int len = GetWindowTextLengthW(hWnd);
                    WCHAR * buffer = new WCHAR[len + 1];
                    SendMessageW(hWnd, WM_GETTEXT, static_cast<WPARAM>(len + 1), reinterpret_cast<LPARAM>(buffer));
                    wstring ret(buffer);
                    delete buffer;
                    return ret;
                }

                Textbox * setText(wstring s)
                {
                    return this;
                }
            };
        }
    }
}