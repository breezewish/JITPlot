#pragma once

#include <sstream>
#include <cmath>

#include <list>
#include <stack>
#include <memory>
#include <string>
#include <cmath>
#include <cfloat>

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
            wstring             exp;
            ExecutablePtr       executable;

            double              evalInternal = 0;
            double              evalXMin = 0, evalXMax = 0;
            int                 evalN = 0;
            vector<double>      evalCache;

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

            inline double eval(double x)
            {
                executable->setVar(0, x);
                return executable->eval();
            }

            vector<double> & eval(double xMin, double xMax, int n)
            {
                double newInterval = (xMax - xMin) / n;

                // intervals are the same
                if (std::fabs(newInterval - evalInternal) < DBL_EPSILON && n == evalN)
                {
                    if (xMin == evalXMin)
                    {
                        // do nothing
                    }
                    else if (xMin < evalXMin)
                    {
                        // move right
                        int offset = static_cast<int>(std::round((evalXMin - xMin) / newInterval));
                        memcpy(&evalCache[offset], &evalCache[0], (n - offset) * sizeof(double));
                        // eval
                        double x = xMin;
                        for (int i = 0; i < offset; ++i)
                        {
                            evalCache[i] = eval(x);
                            x += newInterval;
                        }
                    }
                    else
                    {
                        // move left
                        int offset = static_cast<int>(std::round((xMax - evalXMax) / newInterval));
                        memcpy(&evalCache[0], &evalCache[offset], (n - offset) * sizeof(double));
                        // eval
                        double x = evalXMax + newInterval;
                        for (int i = n - offset; i < n; ++i)
                        {
                            evalCache[i] = eval(x);
                            x += newInterval;
                        }
                    }
                    evalXMin = xMin;
                    evalXMax = xMax;
                }
                else
                {
                    evalCache.resize(n);
                    double x = xMin;
                    for (int i = 0; i < n; ++i)
                    {
                        evalCache[i] = eval(x);
                        x += newInterval;
                    }
                    evalXMin = xMin;
                    evalXMax = xMax;
                    evalInternal = newInterval;
                    evalN = n;
                }

                return evalCache;
            }
        };

        typedef std::shared_ptr<Expression> ExpressionPtr;
    }
}