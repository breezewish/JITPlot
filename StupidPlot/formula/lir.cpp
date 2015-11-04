#include <formula/lir.h>

using std::wostream;

namespace StupidPlot
{
    namespace Formula
    {
        wostream & operator<< (wostream & out, LIROperand const & operand)
        {
            switch (operand.type)
            {
            case LIROperandType::MEM:       return out << operand.mem;
            case LIROperandType::XMM:       return out << operand.xmm;
            case LIROperandType::IMM:       return out << L"0x" << std::hex << operand.imm << std::dec;
            default:                        return out << L"?";
            }
        }

        wostream & operator<< (wostream & out, LIRInstruction const & ins)
        {
            switch (ins.operation)
            {
            case LIROperation::SET_BASE:        out << L"SET_BASE";         break;
            case LIROperation::PUSH_MEM_XMM:    out << L"PUSH_MEM_XMM";     break;
            case LIROperation::POP_XMM_MEM:     out << L"POP_XMM_MEM";      break;
            case LIROperation::LOAD_XMM_MEM:    out << L"LOAD_XMM_MEM";     break;
            case LIROperation::STORE_MEM_XMM:   out << L"STORE_MEM_XMM";    break;
            case LIROperation::ADD_XMM_XMM:     out << L"ADD_XMM_XMM";      break;
            case LIROperation::SUB_XMM_XMM:     out << L"SUB_XMM_XMM";      break;
            case LIROperation::MUL_XMM_XMM:     out << L"MUL_XMM_XMM";      break;
            case LIROperation::DIV_XMM_XMM:     out << L"DIV_XMM_XMM";      break;
            case LIROperation::MOV_XMM_XMM:     out << L"MOV_XMM_XMM";      break;
            case LIROperation::SQRT_XMM:        out << L"SQRT_XMM";         break;
            case LIROperation::MOV_XMM_ST0:     out << L"MOV_XMM_ST0";      break;
            case LIROperation::MOV_ST0_XMM:     out << L"MOV_ST0_XMM";      break;
            case LIROperation::XOR_XMM_XMM:     out << L"XOR_XMM_XMM";      break;
            case LIROperation::ANDNOT_XMM_XMM:  out << L"ANDNOT_XMM_XMM";   break;
            case LIROperation::SIN_ST0:         out << L"SIN_ST0";          break;
            case LIROperation::COS_ST0:         out << L"COS_ST0";          break;
            case LIROperation::POWER_ST0_ST1:   out << L"POWER_ST0_ST1";    break;
            case LIROperation::RET:             out << L"RET";              break;
            default:                            out << L"?";                break;
            }
            if (ins.operands >= 1) out << L" " << ins.operand1;
            if (ins.operands >= 2) out << L", " << ins.operand2;
            return out;
        }
    }
}