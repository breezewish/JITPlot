#pragma once

#include <vector>
#include <map>
#include <string>
#include <formula/jit/types.h>
#include <formula/jit/instructions.h>
#include <formula/jit/functiontable.h>
#include <formula/jit/assembler.h>

using std::vector;
using std::map;
using std::wstring;

namespace StupidPlot
{
    namespace Formula
    {
        namespace JIT
        {
            class InstructionGen
            {
            protected:
                int varsMax = 0;
                int constsMax = 0;

                int funcConstsMax = 0;
                map<double, int> functionConstantOffsets;

                int tempVars = 0;
                int tempVarsMax = 0;

                vector<Instruction> instructions;

            public:
                InstructionGen(int vars, int consts)
                {
                    varsMax = vars;
                    constsMax = consts;
                }

                ~InstructionGen()
                {
                }

                int getReservedFuncConstantSize()
                {
                    return 0;
                }

                void copyFuncConstant(double * pFuncVars)
                {
                    UNREFERENCED_PARAMETER(pFuncVars);
                    // TODO
                }

                int getReservedTempVarSize()
                {
                    return tempVarsMax;
                }

                MEM reserve_temp_mem()
                {
                    tempVars++;
                    if (tempVars > tempVarsMax) tempVarsMax = tempVars;
                    return MEM(MemoryPositionType::TEMP_VAR, tempVars - 1);
                }

                void release_temp_mem(int n = 1)
                {
                    tempVars -= n;
                }

                int resolve_mem(MEM mem)
                {
                    int baseOffset = 0;
                    switch (mem.type)
                    {
                    case MemoryPositionType::DYNAMIC_SYMBOL_VAR:
                        baseOffset = 0; break;
                    case MemoryPositionType::STATIC_CONSTANT:
                        baseOffset = varsMax; break;
                    case MemoryPositionType::FUNCTION_CONSTANT:
                        baseOffset = varsMax + constsMax; break;
                    case MemoryPositionType::TEMP_VAR:
                        baseOffset = varsMax + constsMax + funcConstsMax; break;
                    }
                    return baseOffset + mem.offsetInType;
                }

                void jit_sse_movsd_xmm_mem(XMM xmmDst, MEM memSrc)
                {
                    Instruction ins(InstructionName::INS_MOVSD_XMM_MEM);
                    ins.params.push_back(InstructionParameter(xmmDst));
                    ins.params.push_back(InstructionParameter(memSrc));
                    instructions.push_back(ins);
                }

                void jit_sse_movsd_xmm_xmm(XMM xmmDst, XMM xmmSrc)
                {
                    Instruction ins(InstructionName::INS_MOVSD_XMM_XMM);
                    ins.params.push_back(InstructionParameter(xmmDst));
                    ins.params.push_back(InstructionParameter(xmmSrc));
                    instructions.push_back(ins);
                }

                void jit_sse_movsd_mem_xmm(MEM memDst, XMM xmmSrc)
                {
                    Instruction ins(InstructionName::INS_MOVSD_MEM_XMM);
                    ins.params.push_back(InstructionParameter(memDst));
                    ins.params.push_back(InstructionParameter(xmmSrc));
                    instructions.push_back(ins);
                }

                void jit_sse_addsd_xmm_xmm(XMM xmm1, XMM xmm2)
                {
                    Instruction ins(InstructionName::INS_ADDSD_XMM_XMM);
                    ins.params.push_back(InstructionParameter(xmm1));
                    ins.params.push_back(InstructionParameter(xmm2));
                    instructions.push_back(ins);
                }

                void jit_sse_subsd_xmm_xmm(XMM xmm1, XMM xmm2)
                {
                    Instruction ins(InstructionName::INS_SUBSD_XMM_XMM);
                    ins.params.push_back(InstructionParameter(xmm1));
                    ins.params.push_back(InstructionParameter(xmm2));
                    instructions.push_back(ins);
                }

                void jit_sse_mulsd_xmm_xmm(XMM xmm1, XMM xmm2)
                {
                    Instruction ins(InstructionName::INS_MULSD_XMM_XMM);
                    ins.params.push_back(InstructionParameter(xmm1));
                    ins.params.push_back(InstructionParameter(xmm2));
                    instructions.push_back(ins);
                }

                void jit_sse_divsd_xmm_xmm(XMM xmm1, XMM xmm2)
                {
                    Instruction ins(InstructionName::INS_DIVSD_XMM_XMM);
                    ins.params.push_back(InstructionParameter(xmm1));
                    ins.params.push_back(InstructionParameter(xmm2));
                    instructions.push_back(ins);
                }

                void jit_sse_sqrtsd_xmm(XMM xmm1, XMM xmm2)
                {
                    Instruction ins(InstructionName::INS_SQRTSD_XMM_XMM);
                    ins.params.push_back(InstructionParameter(xmm1));
                    ins.params.push_back(InstructionParameter(xmm2));
                    instructions.push_back(ins);
                }

                void jit_x87_fld_st0_mem(MEM memSrc)
                {
                    Instruction ins(InstructionName::INS_FLD_ST0_MEM);
                    ins.params.push_back(InstructionParameter(memSrc));
                    instructions.push_back(ins);
                }

                void jit_x87_fstp_mem_st0(MEM memDst)
                {
                    Instruction ins(InstructionName::INS_FSTP_MEM_ST0);
                    ins.params.push_back(InstructionParameter(memDst));
                    instructions.push_back(ins);
                }

                void jit_x87_fsin()
                {
                    Instruction ins(InstructionName::INS_FSIN);
                    instructions.push_back(ins);
                }

                void jit_x87_fcos()
                {
                    Instruction ins(InstructionName::INS_FCOS);
                    instructions.push_back(ins);
                }

                void jit_x86_ret()
                {
                    Instruction ins(InstructionName::INS_RET);
                    instructions.push_back(ins);
                }

                void generate(
                    char * buffer,
                    int & codeLength,
                    const BufferPosition & bufferPos
                    )
                {
                    jit_sse_movsd_mem_xmm(MEM(MemoryPositionType::RETURN_VAR, 0), XMM(0));
                    jit_x86_ret();

                    codeLength = 0;
                    for (auto itr = instructions.begin(); itr != instructions.end(); itr++)
                    {
                        itr->resolveMemoryAddress(bufferPos);
                    }
                    for (auto ins : instructions)
                    {
                        Assembler::assemble(ins, buffer, codeLength);
                    }
                }
            };
        }
    }
}