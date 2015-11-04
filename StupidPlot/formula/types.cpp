#include <formula/types.h>

using std::wostream;

namespace StupidPlot
{
    namespace Formula
    {
        wostream & operator<< (wostream & out, MEM const & mem)
        {
            switch (mem.type)
            {
            case MemoryOffsetType::INVALID:             return out << L"[?]";
            case MemoryOffsetType::OFFSET_CONSTANT:     return out << L"[CONSTANT + 0x" << std::hex << mem.offset << std::dec << L" * 8]";
            case MemoryOffsetType::OFFSET_RETURN_VALUE: return out << L"[RET]";
            case MemoryOffsetType::OFFSET_VARIABLE:     return out << L"[VAR + 0x" << std::hex << mem.offset << std::dec << L" * 8]";
            case MemoryOffsetType::RESOLVED:            return out << L"[ECX + 0x" << std::hex << mem.offset << std::dec << L" * 8]";
            default:                                    return out << L"[?]";
            }
        }

        wostream & operator<< (wostream & out, XMM const & xmm)
        {
            return out << L"XMM" << xmm.reg;
        }

        wostream & operator<< (wostream & out, REG const & reg)
        {
            switch (reg.reg)
            {
            case GPREG::ECX:    return out << L"ECX";
            case GPREG::ESP:    return out << L"ESP";
            default:            return out << L"UNKNOWN_REG";
            }
        }

        wostream & operator<< (wostream & out, MEMREF const & memref)
        {
            return out << L"[" << memref.ref << L" + 0x" << std::hex << memref.offset << std::dec << L"]";
        }
    }
}