#pragma once

#include <sstream>
#include <cmath>

#include <list>
#include <stack>
#include <memory>
#include <string>

#include <windows.h>

#include <formula/token.h>
#include <formula/lexer.h>
#include <formula/parser.h>
#include <formula/compiler.h>
#include <formula/jit/executable.h>

using std::wstring;
using std::map;
using std::vector;
using std::shared_ptr;

namespace StupidPlot
{
    namespace Formula
    {
        using namespace JIT;

        class Expression
        {
        protected:
            wstring            exp;
            ExecutablePtr      executable;

        public:
            Expression(
                wstring _exp,
                map<wstring, double> & _constVars
                )
            {
                exp = _exp;

                auto dynamicVars = vector<wstring>();
                dynamicVars.push_back(L"x");

                auto tokens = Lexer::lex(exp);
                tokens = Parser::getRPN(tokens);
                auto cr = Compiler::compileRPN(tokens, _constVars, dynamicVars);
                executable = ExecutablePtr(new Executable(cr));
            }

            double eval(double x)
            {
                executable->setVar(0, x);
                return executable->eval();
            }
        };

        typedef std::shared_ptr<Expression> ExpressionPtr;
    }
}