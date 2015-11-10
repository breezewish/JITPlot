#pragma once

#include <windows.h>

#include <string>
#include <memory>

#include <ui/controls/control.h>
#include <ui/controls/win32control.h>

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
                Textbox(HWND _hWindow, int _id) : Win32Control(_hWindow, _id)
                {
                }

                wstring getText()
                {
                    int len = GetWindowTextLengthW(hControl);
                    WCHAR * buffer = new WCHAR[len + 1];
                    SendMessageW(hControl, WM_GETTEXT, static_cast<WPARAM>(len + 1), reinterpret_cast<LPARAM>(buffer));
                    wstring ret(buffer);
                    delete buffer;
                    return ret;
                }

                Textbox * setText(wstring s)
                {
                    SetWindowTextW(hControl, s.c_str());
                    return this;
                }
            };

            typedef std::shared_ptr<Textbox> TextboxPtr;
        }
    }
}