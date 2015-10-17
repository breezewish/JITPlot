#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

namespace StupidPlot
{
    class Debug
    {
    public:
        static void output(LPCTSTR lpszFormat, ...)
        {
            va_list args;
            va_start(args, lpszFormat);
            int nBuf;
            WCHAR szBuffer[512];
            nBuf = _vsnwprintf_s(szBuffer, 511, lpszFormat, args);
            OutputDebugStringW(szBuffer);
            va_end(args);
        }
    };

}
