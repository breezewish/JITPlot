#pragma once

#include <list>
#include <sstream>

#include <windows.h>

#include <formula/types.h>
#include <formula/assembler.h>

namespace StupidPlot
{
    namespace Formula
    {
        enum class LIROperation
        {
            SET_BASE,           // mov ecx, addr
            PUSH_MEM_XMM,       // sub, movsd
            POP_XMM_MEM,        // movsd, add
            LOAD_XMM_MEM,       // movsd
            STORE_MEM_XMM,      // movsd
            ADD_XMM_XMM,        // addsd
            SUB_XMM_XMM,
            MUL_XMM_XMM,
            DIV_XMM_XMM,
            MOV_XMM_XMM,        // movapd
            SQRT_XMM_XMM,
            LOAD_ST0_MEM,
            STORE_ST0_MEM,
            SIN_ST0,
            COS_ST0,
            RET,
        };

        enum class LIROperandType
        {
            IMM,
            MEM,
            XMM,
            UNUSED,
        };

        struct LIROperand
        {
            LIROperandType type;

            union
            {
                unsigned int imm;
                MEM mem;
                XMM xmm;
            };

            LIROperand()
            {
                type = LIROperandType::UNUSED;
            }

            LIROperand(unsigned int v)
            {
                imm = v;
                type = LIROperandType::IMM;
            }

            LIROperand(MEM v)
            {
                mem = v;
                type = LIROperandType::MEM;
            }

            LIROperand(XMM v)
            {
                xmm = v;
                type = LIROperandType::XMM;
            }
        };

        std::wostream & operator<< (std::wostream & out, LIROperand const & operand);

        struct LIRInstruction
        {
            int operands;
            LIROperation operation;
            LIROperand operand1, operand2;

            LIRInstruction(LIROperation op)
            {
                operation = op;
                operands = 0;
            }

            LIRInstruction(LIROperation op, LIROperand opnd1)
            {
                operation = op;
                operand1 = opnd1;
                operands = 1;
            }

            LIRInstruction(LIROperation op, LIROperand opnd1, LIROperand opnd2)
            {
                operation = op;
                operand1 = opnd1;
                operand2 = opnd2;
                operands = 2;
            }

            inline void generate(PBYTE & buffer)
            {
                switch (operation)
                {
                case LIROperation::RET:
                    Assembler::RET(buffer, short(operand1.imm));
                    break;
                case LIROperation::SET_BASE:
                    Assembler::MOV_REG_IMM32(buffer, REG(GPREG::ECX), operand1.imm);
                    break;
                case LIROperation::PUSH_MEM_XMM:
                    Assembler::SUB_REG_IMM8(buffer, REG(GPREG::ESP), byte(sizeof(double)));
                    Assembler::MOVSD_MEM_XMM(buffer, MEMREF::ESP_REF(), operand1.xmm);
                    break;
                case LIROperation::POP_XMM_MEM:
                    Assembler::MOVSD_XMM_MEM(buffer, operand1.xmm, MEMREF::ESP_REF());
                    Assembler::ADD_REG_IMM8(buffer, REG(GPREG::ESP), byte(sizeof(double)));
                    break;
                case LIROperation::LOAD_XMM_MEM:
                    Assembler::MOVSD_XMM_MEM(buffer, operand1.xmm, MEMREF::ECX_REF(operand2.mem.offset * sizeof(double)));
                    break;
                case LIROperation::STORE_MEM_XMM:
                    Assembler::MOVSD_MEM_XMM(buffer, MEMREF::ECX_REF(operand1.mem.offset * sizeof(double)), operand2.xmm);
                    break;
                case LIROperation::ADD_XMM_XMM:
                    Assembler::ADDSD_XMM_XMM(buffer, operand1.xmm, operand2.xmm);
                    break;
                case LIROperation::SUB_XMM_XMM:
                    Assembler::SUBSD_XMM_XMM(buffer, operand1.xmm, operand2.xmm);
                    break;
                case LIROperation::MUL_XMM_XMM:
                    Assembler::MULSD_XMM_XMM(buffer, operand1.xmm, operand2.xmm);
                    break;
                case LIROperation::DIV_XMM_XMM:
                    Assembler::DIVSD_XMM_XMM(buffer, operand1.xmm, operand2.xmm);
                    break;
                case LIROperation::MOV_XMM_XMM:
                    Assembler::MOVAPD_XMM_XMM(buffer, operand1.xmm, operand2.xmm);
                    break;
                case LIROperation::SQRT_XMM_XMM:
                    Assembler::SQRTSD_XMM_XMM(buffer, operand1.xmm, operand1.xmm);
                    break;
                case LIROperation::LOAD_ST0_MEM:
                    // Not implemented
                    break;
                case LIROperation::STORE_ST0_MEM:
                    // Not implemented
                    break;
                case LIROperation::SIN_ST0:
                    // Not implemented
                    break;
                case LIROperation::COS_ST0:
                    // Not implemented
                    break;
                default:
                    break;
                }
            }
        };

        std::wostream & operator<< (std::wostream & out, LIRInstruction const & ins);

        typedef std::list<LIRInstruction> LIRInstructionSet;
    }
}