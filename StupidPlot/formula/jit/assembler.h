#pragma once

#include <formula/exception.h>
#include <formula/jit/instructions.h>

namespace StupidPlot
{
    namespace Formula
    {
        namespace JIT
        {
            class Assembler
            {
            protected:
                static void ASSERT_OPERAND(const Instruction & ins, size_t n)
                {
                    if (ins.params.size() != n)
                    {
                        throw runtime_error("Operand number mismatch");
                    }
                }

                static void ASSERT_OPERAND_XMM(const Instruction & ins, int i)
                {
                    if (ins.params[i].type != InstructionParameterType::INS_PARAM_XMM)
                    {
                        throw runtime_error("Expect operand to be XMM");
                    }
                    if (ins.params[i].value.xmm.offset > 7)
                    {
                        throw runtime_error("Only support XMM0-XMM7");
                    }
                }

                static void ASSERT_OPERAND_MEM(const Instruction & ins, int i)
                {
                    if (ins.params[i].type != InstructionParameterType::INS_PARAM_MEM)
                    {
                        throw runtime_error("Expect operand to be MEM");
                    }
                    if (ins.params[i].value.mem.type != MemoryPositionType::RESOLVED)
                    {
                        throw runtime_error("Only support resolved memory address");
                    }
                }

                static void _ASM_MOV_ECX_IMM(char * buffer, int & offset, DWORD imm)
                {
                    buffer[offset++] = byte(0xB9);
                    *(reinterpret_cast<int *>(buffer + offset)) = imm;
                    offset += 4;
                }

                static void ASM_MOVSD_MEM_XMM(const Instruction & ins, char * buffer, int & offset)
                {
                    ASSERT_OPERAND(ins, 2);
                    ASSERT_OPERAND_MEM(ins, 0);
                    ASSERT_OPERAND_XMM(ins, 1);

                    // trick: we first move mem address into ECX, then use [ECX] as memory address
                    // MOV ECX, addr
                    _ASM_MOV_ECX_IMM(buffer, offset, ins.params[0].value.mem.offsetInType);

                    // MOVSD [ECX], XMM
                    buffer[offset++] = byte(0xF2);
                    buffer[offset++] = byte(0x0F);
                    buffer[offset++] = byte(0x11);
                    buffer[offset++] = byte(0x01 | (ins.params[1].value.xmm.offset) << 3);
                }

                static void ASM_MOVSD_XMM_MEM(const Instruction & ins, char * buffer, int & offset)
                {
                    ASSERT_OPERAND(ins, 2);
                    ASSERT_OPERAND_XMM(ins, 0);
                    ASSERT_OPERAND_MEM(ins, 1);

                    // MOV ECX, addr
                    _ASM_MOV_ECX_IMM(buffer, offset, ins.params[1].value.mem.offsetInType);

                    // MOVSD XMM, [ECX]
                    buffer[offset++] = byte(0xF2);
                    buffer[offset++] = byte(0x0F);
                    buffer[offset++] = byte(0x10);
                    buffer[offset++] = byte(0x01 | (ins.params[0].value.xmm.offset) << 3);
                }

                static void ASM_MOVSD_XMM_XMM(const Instruction & ins, char * buffer, int & offset)
                {
                    ASSERT_OPERAND(ins, 2);
                    ASSERT_OPERAND_XMM(ins, 0);
                    ASSERT_OPERAND_XMM(ins, 1);

                    buffer[offset++] = byte(0xF2);
                    buffer[offset++] = byte(0x0F);
                    buffer[offset++] = byte(0x10);
                    buffer[offset++] = byte(0xC0 | (ins.params[0].value.xmm.offset) << 3 | (ins.params[1].value.xmm.offset));
                }

                static void ASM_ADDSD_XMM_XMM(const Instruction & ins, char * buffer, int & offset)
                {
                    ASSERT_OPERAND(ins, 2);
                    ASSERT_OPERAND_XMM(ins, 0);
                    ASSERT_OPERAND_XMM(ins, 1);

                    buffer[offset++] = byte(0xF2);
                    buffer[offset++] = byte(0x0F);
                    buffer[offset++] = byte(0x58);
                    buffer[offset++] = byte(0xC0 | (ins.params[0].value.xmm.offset) << 3 | (ins.params[1].value.xmm.offset));
                }

                static void ASM_SUBSD_XMM_XMM(const Instruction & ins, char * buffer, int & offset)
                {
                    ASSERT_OPERAND(ins, 2);
                    ASSERT_OPERAND_XMM(ins, 0);
                    ASSERT_OPERAND_XMM(ins, 1);

                    buffer[offset++] = byte(0xF2);
                    buffer[offset++] = byte(0x0F);
                    buffer[offset++] = byte(0x5C);
                    buffer[offset++] = byte(0xC0 | (ins.params[0].value.xmm.offset) << 3 | (ins.params[1].value.xmm.offset));
                }

                static void ASM_MULSD_XMM_XMM(const Instruction & ins, char * buffer, int & offset)
                {
                    ASSERT_OPERAND(ins, 2);
                    ASSERT_OPERAND_XMM(ins, 0);
                    ASSERT_OPERAND_XMM(ins, 1);

                    buffer[offset++] = byte(0xF2);
                    buffer[offset++] = byte(0x0F);
                    buffer[offset++] = byte(0x59);
                    buffer[offset++] = byte(0xC0 | (ins.params[0].value.xmm.offset) << 3 | (ins.params[1].value.xmm.offset));
                }

                static void ASM_DIVSD_XMM_XMM(const Instruction & ins, char * buffer, int & offset)
                {
                    ASSERT_OPERAND(ins, 2);
                    ASSERT_OPERAND_XMM(ins, 0);
                    ASSERT_OPERAND_XMM(ins, 1);

                    buffer[offset++] = byte(0xF2);
                    buffer[offset++] = byte(0x0F);
                    buffer[offset++] = byte(0x5E);
                    buffer[offset++] = byte(0xC0 | (ins.params[0].value.xmm.offset) << 3 | (ins.params[1].value.xmm.offset));
                }

                static void ASM_SQRTSD_XMM_XMM(const Instruction & ins, char * buffer, int & offset)
                {
                    ASSERT_OPERAND(ins, 2);
                    ASSERT_OPERAND_XMM(ins, 0);
                    ASSERT_OPERAND_XMM(ins, 1);

                    buffer[offset++] = byte(0xF2);
                    buffer[offset++] = byte(0x0F);
                    buffer[offset++] = byte(0x51);
                    buffer[offset++] = byte(0xC0 | (ins.params[0].value.xmm.offset) << 3 | (ins.params[1].value.xmm.offset));
                }

                static void ASM_FSIN(const Instruction & ins, char * buffer, int & offset)
                {
                    ASSERT_OPERAND(ins, 0);

                    buffer[offset++] = byte(0xD9);
                    buffer[offset++] = byte(0xFE);
                }

                static void ASM_FCOS(const Instruction & ins, char * buffer, int & offset)
                {
                    ASSERT_OPERAND(ins, 0);

                    buffer[offset++] = byte(0xD9);
                    buffer[offset++] = byte(0xFF);
                }

                static void ASM_FLD_ST0_MEM(const Instruction & ins, char * buffer, int & offset)
                {
                    ASSERT_OPERAND(ins, 1);
                    ASSERT_OPERAND_MEM(ins, 0);

                    // MOV ECX, addr
                    _ASM_MOV_ECX_IMM(buffer, offset, ins.params[0].value.mem.offsetInType);

                    // FLD QWORD PTR [ECX]
                    buffer[offset++] = byte(0xDD);
                    buffer[offset++] = byte(0x01);
                }

                static void ASM_FST_MEM_ST0(const Instruction & ins, char * buffer, int & offset)
                {
                    ASSERT_OPERAND(ins, 1);
                    ASSERT_OPERAND_MEM(ins, 0);

                    // MOV ECX, addr
                    _ASM_MOV_ECX_IMM(buffer, offset, ins.params[0].value.mem.offsetInType);

                    // FST QWORD PTR [ECX]
                    buffer[offset++] = byte(0xDD);
                    buffer[offset++] = byte(0x11);
                }

                static void ASM_RET(const Instruction & ins, char * buffer, int & offset)
                {
                    ASSERT_OPERAND(ins, 0);

                    buffer[offset++] = byte(0xC3);
                }

            public:
                static void assemble(const Instruction & ins, char * buffer, int & offset)
                {
                    switch (ins.name)
                    {
                    case INS_MOVSD_MEM_XMM:  ASM_MOVSD_MEM_XMM(ins, buffer, offset); break;
                    case INS_MOVSD_XMM_MEM:  ASM_MOVSD_XMM_MEM(ins, buffer, offset); break;
                    case INS_MOVSD_XMM_XMM:  ASM_MOVSD_XMM_XMM(ins, buffer, offset); break;
                    case INS_ADDSD_XMM_XMM:  ASM_ADDSD_XMM_XMM(ins, buffer, offset); break;
                    case INS_SUBSD_XMM_XMM:  ASM_SUBSD_XMM_XMM(ins, buffer, offset); break;
                    case INS_MULSD_XMM_XMM:  ASM_MULSD_XMM_XMM(ins, buffer, offset); break;
                    case INS_DIVSD_XMM_XMM:  ASM_DIVSD_XMM_XMM(ins, buffer, offset); break;
                    case INS_SQRTSD_XMM_XMM: ASM_SQRTSD_XMM_XMM(ins, buffer, offset); break;
                    case INS_FLD_ST0_MEM:    ASM_FLD_ST0_MEM(ins, buffer, offset); break;
                    case INS_FST_MEM_ST0:    ASM_FST_MEM_ST0(ins, buffer, offset); break;
                    case INS_FSIN:           ASM_FSIN(ins, buffer, offset); break;
                    case INS_FCOS:           ASM_FCOS(ins, buffer, offset); break;
                    case INS_RET:            ASM_RET(ins, buffer, offset); break;
                    default:                 throw runtime_error("Unrecognized instruction"); break;
                    }
                }
            };
        }
    }
}