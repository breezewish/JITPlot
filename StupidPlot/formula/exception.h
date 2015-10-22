#pragma once

#include <stdexcept>
#include <string>

#include <util.h>

using std::runtime_error;
using std::wstring;

namespace StupidPlot
{
    namespace Formula
    {
        class Exception : public runtime_error
        {
        public:
            Exception(wstring msg)
                : runtime_error(Util::utf8_encode(msg))
            {
            }
        };

        // Lexer exceptions

        class UnexpectedSymbolException : public Exception
        {
        public:
            UnexpectedSymbolException(wstring symbol)
                : Exception(L"Unexpected symbol: " + symbol)
            {
            }
        };

        // Parser exceptions

        class OutOfStackException : public Exception
        {
        public:
            OutOfStackException()
                : Exception(L"Out of stack")
            {
            }
        };

        class UnknownOperatorException : public Exception
        {
        public:
            UnknownOperatorException(wstring op)
                : Exception(L"Unknown operator: " + op)
            {
            }
        };

        class MissingLeftBracketException : public Exception
        {
        public:
            MissingLeftBracketException()
                : Exception(L"Missing left bracket")
            {
            }
        };

        class MissingRightBracketException : public Exception
        {
        public:
            MissingRightBracketException()
                : Exception(L"Missing right bracket")
            {
            }
        };

        class UnexpectedCommaException : public Exception
        {
        public:
            UnexpectedCommaException()
                : Exception(L"Unexpected comma")
            {
            }
        };

        // Compiler exceptions

        class UnknownSymbolException : public Exception
        {
        public:
            UnknownSymbolException(wstring symbol)
                : Exception(L"Cannot resolve symbol: " + symbol)
            {
            }
        };

        class ParameterNumberMismatchException : public Exception
        {
        public:
            ParameterNumberMismatchException(wstring name, int received, int expect)
                : Exception(L"Function parameter number for " + name + L" mismatch, expect " + std::to_wstring(expect) + L", received " + std::to_wstring(received))
            {
            }
        };

        class OperatorNotImplementedException : public Exception
        {
        public:
            OperatorNotImplementedException(wstring op)
                : Exception(L"Operator " + op + L" is not implemented")
            {
            }
        };
    }
}