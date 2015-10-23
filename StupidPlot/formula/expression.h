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

                // reserved size
                n_dynamicVars = cr.dynamicVarOffsets.size();
                n_constVars = cr.constantOffsets.size();
                n_funcVars = cr.insgen->getReservedFuncConstantSize();
                n_tempVars = cr.insgen->getReservedTempVarSize();
                n_all = n_dynamicVars + n_constVars + n_funcVars + n_tempVars + 1;

                // allocate continuous memory
                pBuffer = new double[n_all];
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
                BufferPosition bpos;
                bpos.pConstVars = pConstVars;
                bpos.pDynamicVars = pDynamicVars;
                bpos.pFuncVars = pFuncVars;
                bpos.pRetVar = pRetVar;
                bpos.pTempVars = pTempVars;

                char * codeBuf = new char[4 * 1024];   // allocate 4K
                int codeLen;
                cr.insgen->generate(codeBuf, codeLen, bpos);

                // write our code and then disable write permission
                pCode = VirtualAlloc(NULL, codeLen, MEM_COMMIT, PAGE_READWRITE);
                CopyMemory(pCode, codeBuf, codeLen);
                DWORD dwOldProtect;
                VirtualProtect(pCode, codeLen, PAGE_EXECUTE_READ, &dwOldProtect);
                Debug() << GetLastError() >> Debug::writeln;

                delete[] codeBuf;
            }

            ~Expression()
            {
                delete[] pBuffer;
                pBuffer = NULL;
                pDynamicVars = NULL;
                pFuncVars = NULL;
                pTempVars = NULL;
                pRetVar = NULL;

                VirtualFree(pCode, 0, MEM_RELEASE);
            }

            int getDynamicVarOffset(wstring name)
            {
                return cr.dynamicVarOffsets[name];
            }

            inline void setVar(int offset, double val)
            {
                pDynamicVars[offset] = val;
            }

            double eval()
            {
                reinterpret_cast<void(*)()>(pCode)();
                return *pRetVar;
            }
        };
    }
}