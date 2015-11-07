#pragma once

#include <windows.h>

#include <string>
#include <memory>

#include <ui/controls/control.h>

using std::wstring;

namespace StupidPlot
{
    namespace UI
    {
        namespace Controls
        {
            class Textbox : public Win32Control
            {
            public:
                Textbox(HWND _hWnd, int _id) : Win32Control(_hWnd, _id)
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
                    SetWindowTextW(hWnd, s.c_str());
                    return this;
                }
            };

            typedef std::shared_ptr<Textbox> TextboxPtr;
        }
    }
}