#pragma once

#include <memory>
#include <string>
#include <map>
#include <vector>

#include <windows.h>

#include <formula/token.h>
#include <formula/lexer.h>
#include <formula/parser.h>
#include <formula/compiler.h>
#include <formula/jit/types.h>
#include <formula/jit/instructions.h>
#include <formula/jit/instructiongen.h>

using std::wstring;
using std::map;
using std::vector;

namespace StupidPlot
{
    namespace Formula
    {
        namespace JIT
        {
            class Executable
            {
            protected:
                const CompiledResultPtr     & cr;

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
                Executable(const CompiledResultPtr & _cr) : cr(_cr)
                {
                    // reserved size
                    n_dynamicVars = _cr->dynamicVarOffsets.size();
                    n_constVars = _cr->constantOffsets.size();
                    n_funcVars = _cr->insgen->getReservedFuncConstantSize();
                    n_tempVars = _cr->insgen->getReservedTempVarSize();
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
                    for (auto pair : _cr->constantOffsets)
                    {
                        pConstVars[pair.second] = pair.first;
                    }

                    // copy function constants
                    _cr->insgen->copyFuncConstant(pFuncVars);

                    // generate machine code
                    BufferPosition bpos;
                    bpos.pConstVars = pConstVars;
                    bpos.pDynamicVars = pDynamicVars;
                    bpos.pFuncVars = pFuncVars;
                    bpos.pRetVar = pRetVar;
                    bpos.pTempVars = pTempVars;

                    char * codeBuf = new char[4 * 1024];   // allocate 4K
                    int codeLen;
                    _cr->insgen->generate(codeBuf, codeLen, bpos);

                    // write our code and then disable write permission
                    pCode = VirtualAlloc(NULL, codeLen, MEM_COMMIT, PAGE_READWRITE);
                    CopyMemory(pCode, codeBuf, codeLen);
                    DWORD dwOldProtect;
                    VirtualProtect(pCode, codeLen, PAGE_EXECUTE_READ, &dwOldProtect);

                    delete[] codeBuf;
                }

                ~Executable()
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
                    return cr->dynamicVarOffsets[name];
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

            typedef std::shared_ptr<Executable> ExecutablePtr;
        }
    }
}