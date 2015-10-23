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
#include <formula/jit/types.h>
#include <formula/jit/instructions.h>
#include <formula/jit/instructiongen.h>

using std::wstring;
using std::list;
using std::stack;
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
            wstring                     exp;
            list<shared_ptr<Token>>     tokens;
            CompiledResult              cr;

            int                         n_dynamicVars;
            int                         n_constVars;
            int                         n_funcVars;
            int                         n_tempVars;
            int                         n_all;

            double                      * pBuffer;
            double                      * pDynamicVars;
            double                      * pConstVars;
            double                      * pFuncVars;
            double                      * pTempVars;
            double                      * pRetVar;

            void                        * pCode;

        public:
            Expression(
                wstring _exp,
                map<wstring, double> _constVars,
                vector<wstring> _dynamicVars
                )
            {
                exp = _exp;

                tokens = Lexer::lex(exp);
                tokens = Parser::getRPN(tokens);
                cr = Compiler::compileRPN(tokens, _constVars, _dynamicVars);

                // allocate memory for
                /*
                Debug::Debug() << exp >> Debug::writeln;
                for (Instruction ins : insset)
                {
                Debug::Debug() << ins.toString() >> Debug::writeln;
                }*/

                n_dynamicVars = cr.dynamicVarOffsets.size();
                n_constVars = cr.constantOffsets.size();
                n_funcVars = cr.insgen->getReservedFuncConstantSize();
                n_tempVars = cr.insgen->getReservedTempVarSize();
                n_all = n_dynamicVars + n_constVars + n_funcVars + n_tempVars + 1;

                // allocate continuous memory
                pBuffer = new double(n_all);
                pDynamicVars = pBuffer + 0;
                pConstVars = pDynamicVars + n_dynamicVars;
                pFuncVars = pConstVars + n_constVars;
                pTempVars = pFuncVars + n_funcVars;
                pRetVar = pTempVars + n_tempVars;

                // fill 0.0
                for (int i = 0; i < n_all; ++i)
                {
                    pBuffer[i] = 0.0;
                }

                // copy static constants
                for (auto pair : cr.constantOffsets)
                {
                    pConstVars[pair.second] = pair.first;
                }

                // copy function constants
                cr.insgen->copyFuncConstant(pFuncVars);

                // generate machine code
                vector<byte> machineCode = cr.insgen->generate(pDynamicVars, pConstVars, pFuncVars, pTempVars, pRetVar);

                // write our code and then disable write permission
                pCode = VirtualAlloc(NULL, machineCode.size(), MEM_COMMIT, PAGE_READWRITE);
                CopyMemory(pCode, &machineCode[0], machineCode.size());
                VirtualProtect(pCode, machineCode.size(), PAGE_EXECUTE_READ, NULL);
            }

            ~Expression()
            {
                delete pBuffer;
                pBuffer = NULL;
                pDynamicVars = NULL;
                pFuncVars = NULL;
                pTempVars = NULL;
                pRetVar = NULL;

                VirtualFree(pCode, 0, MEM_RELEASE);
            }

            void setVar(wstring name, double val)
            {
                if (cr.dynamicVarOffsets.find(name) == cr.dynamicVarOffsets.end()) return;
                pDynamicVars[cr.dynamicVarOffsets[name]] = val;
            }

            double eval()
            {
                reinterpret_cast<void(*)()>(pCode)();
                return *pRetVar;
            }

            double evaluate(double x)
            {
                stack<double> c;

                for (auto pToken : tokens)
                {
                    if (pToken->is(TokenType::OPERAND))
                    {
                        auto _token = std::dynamic_pointer_cast<OperandToken>(pToken);
                        if (_token->operandType == OperandType::OPERAND_CONST)
                        {
                            auto token = std::dynamic_pointer_cast<ConstantOperandToken>(_token);
                            c.push(token->value);
                        }
                        else
                        {
                            c.push(x);
                        }
                    }
                    else if (pToken->is(TokenType::OPERATOR))
                    {
                        double a, b;
                        auto token = std::dynamic_pointer_cast<OperatorToken>(pToken);
                        switch (token->op)
                        {
                        case OperatorType::OP_ADD:
                            a = c.top(); c.pop();
                            b = c.top(); c.pop();
                            c.push(a + b);
                            break;
                        case OperatorType::OP_SUB:
                            a = c.top(); c.pop();
                            b = c.top(); c.pop();
                            c.push(a - b);
                            break;
                        case OperatorType::OP_MUL:
                            a = c.top(); c.pop();
                            b = c.top(); c.pop();
                            c.push(a * b);
                            break;
                        case OperatorType::OP_DIV:
                            a = c.top(); c.pop();
                            b = c.top(); c.pop();
                            c.push(a / b);
                            break;
                        }
                    }
                    else if (pToken->is(TokenType::FUNC_CALL))
                    {
                        double v = c.top(); c.pop();
                        c.push(std::sin(v));
                    }
                }

                return c.top();
            }
        };
    }
}