#pragma once

#include <string>
#include <sstream>
#include <iomanip>

#include <windows.h>

using std::string;
using std::wstring;
using std::wstringstream;

namespace StupidPlot
{
    template< typename T >
    struct array_deleter
    {
        void operator ()(T const * p)
        {
            delete[] p;
        }
    };

    class Util
    {
    public:
        template <typename T>
        static wstring to_string_with_precision(const T a_value, const int n = 6)
        {
            std::wostringstream out;
            out << std::setprecision(n) << a_value;
            return out.str();
        }

        static string utf8_encode(const wstring &wstr)
        {
            if (wstr.empty()) return string();
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], static_cast<int>(wstr.size()), NULL, 0, NULL, NULL);
            string strTo(size_needed, 0);
            WideCharToMultiByte(CP_UTF8, 0, &wstr[0], static_cast<int>(wstr.size()), &strTo[0], size_needed, NULL, NULL);
            return strTo;
        }

        static wstring utf8_decode(const string &str)
        {
            if (str.empty()) return wstring();
            int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), NULL, 0);
            wstring wstrTo(size_needed, 0);
            MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), &wstrTo[0], size_needed);
            return wstrTo;
        }
    };

    class Formatter
    {
    private:
        std::wstringstream _stream;
        Formatter(const Formatter &);
        Formatter & operator = (Formatter &);
    public:
        Formatter()
        {
        }

        ~Formatter()
        {
        }

        template <typename T>
        Formatter & operator << (const T & value)
        {
            _stream << value;
            return *this;
        }

        wstring str() const
        {
            return _stream.str();
        }

        operator wstring() const
        {
            return _stream.str();
        }

        enum FormatterOutput
        {
            to_string
        };

        wstring operator >> (FormatterOutput)
        {
            return _stream.str();
        }
    };

    class Debug
    {
    private:
        std::wstringstream _stream;
        Debug(const Debug &);
        Debug & operator = (Debug &);
    public:
        Debug()
        {
        }

        ~Debug()
        {
        }

        template <typename T>
        Debug & operator << (const T & value)
        {
            _stream << value;
            return *this;
        }

        enum DebugOutput
        {
            writeln
        };

        void operator >> (DebugOutput)
        {
            wstring str = _stream.str();
            printf(L"%s\n", str.c_str());
        }

        static void printf(LPCTSTR lpszFormat, ...)
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
