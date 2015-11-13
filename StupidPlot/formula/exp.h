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
#include <formula/executable.h>

using std::wstring;
using std::map;
using std::vector;
using std::shared_ptr;

namespace StupidPlot
{
    namespace Formula
    {
        //using namespace JIT;

        class Expression
        {
        protected:
            wstring             exp;
            ExecutablePtr       executable;

            double              evalInternal = 0;
            double              evalXMin = 0, evalXMax = 0;
            int                 evalN = 0;
            vector<double>      evalCacheX, evalCacheY;

        public:
            Expression(
                wstring _exp,
                map<wstring, double> & _constVars
                )
            {
                exp = _exp;

                auto tokens = Lexer::lex(exp);
                tokens = Parser::getRPN(tokens);
                executable = Compiler::compileRPN(tokens, _constVars);
            }

            inline double eval(double x)
            {
                return executable->eval(x);
            }

            void eval(
                double xMin,
                double xMax,
                int n,
                const Plot::DoubleArr & x,
                const Plot::DoubleArr & y
                )
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
                        memcpy(&evalCacheX[offset], &evalCacheX[0], (n - offset) * sizeof(double));
                        memcpy(&evalCacheY[offset], &evalCacheY[0], (n - offset) * sizeof(double));
                        // eval
                        double _x = xMin;
                        for (int i = 0; i < offset; ++i)
                        {
                            evalCacheX[i] = _x;
                            evalCacheY[i] = eval(_x);
                            _x += newInterval;
                        }
                    }
                    else
                    {
                        // move left
                        int offset = static_cast<int>(std::round((xMax - evalXMax) / newInterval));
                        memcpy(&evalCacheX[0], &evalCacheX[offset], (n - offset) * sizeof(double));
                        memcpy(&evalCacheY[0], &evalCacheY[offset], (n - offset) * sizeof(double));
                        // eval
                        double _x = evalXMax + newInterval;
                        for (int i = n - offset; i < n; ++i)
                        {
                            evalCacheX[i] = _x;
                            evalCacheY[i] = eval(_x);
                            _x += newInterval;
                        }
                    }
                    evalXMin = xMin;
                    evalXMax = xMax;
                }
                else
                {
                    evalCacheX.resize(n);
                    evalCacheY.resize(n);
                    double _x = xMin;
                    for (int i = 0; i < n; ++i)
                    {
                        evalCacheX[i] = _x;
                        evalCacheY[i] = eval(_x);
                        _x += newInterval;
                    }
                    evalXMin = xMin;
                    evalXMax = xMax;
                    evalInternal = newInterval;
                    evalN = n;
                }

                memcpy(x.get(), evalCacheX.data(), n * sizeof(double));
                memcpy(y.get(), evalCacheY.data(), n * sizeof(double));
            }
        };

        typedef std::shared_ptr<Expression> ExpressionPtr;
    }
}