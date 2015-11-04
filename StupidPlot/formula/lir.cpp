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
            case LIROperation::SET_BASE:        out << "SET_BASE";      break;
            case LIROperation::PUSH_MEM_XMM:    out << "PUSH_MEM_XMM";  break;
            case LIROperation::POP_XMM_MEM:     out << "POP_XMM_MEM";   break;
            case LIROperation::LOAD_XMM_MEM:    out << "LOAD_XMM_MEM";  break;
            case LIROperation::STORE_MEM_XMM:   out << "STORE_MEM_XMM"; break;
            case LIROperation::ADD_XMM_XMM:     out << "ADD_XMM_XMM";   break;
            case LIROperation::SUB_XMM_XMM:     out << "SUB_XMM_XMM";   break;
            case LIROperation::MUL_XMM_XMM:     out << "MUL_XMM_XMM";   break;
            case LIROperation::DIV_XMM_XMM:     out << "DIV_XMM_XMM";   break;
            case LIROperation::MOV_XMM_XMM:     out << "MOV_XMM_XMM";   break;
            case LIROperation::SQRT_XMM_XMM:    out << "SQRT_XMM_XMM";  break;
            case LIROperation::MOV_XMM_ST0:     out << "MOV_XMM_ST0";   break;
            case LIROperation::MOV_ST0_XMM:     out << "MOV_ST0_XMM";   break;
            case LIROperation::SIN_ST0:         out << "SIN_ST0";       break;
            case LIROperation::COS_ST0:         out << "COS_ST0";       break;
            case LIROperation::RET:             out << "RET";           break;
            default:                            out << L"?";            break;
            }
            if (ins.operands >= 1) out << L" " << ins.operand1;
            if (ins.operands >= 2) out << L", " << ins.operand2;
            return out;
        }
    }
}