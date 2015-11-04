#pragma once

#include <sstream>
#include <string>

namespace StupidPlot
{
    namespace Formula
    {
        struct MemoryOffsets
        {
            int pVariable;
            int pConstants;
            int pTempVariable;
            int pReturnValue;
            int pEnd;
        };

        enum class MemoryOffsetType
        {
            OFFSET_VARIABLE,
            OFFSET_CONSTANT,
            OFFSET_TEMP_VAR,
            OFFSET_RETURN_VALUE,
            INVALID
        };

        struct MEM
        {
            MemoryOffsetType type;
            int offset;

            MEM()
            {
                type = MemoryOffsetType::INVALID;
                offset = 0;
            }

            MEM(MemoryOffsetType _type, int _offset)
            {
                type = _type;
                offset = _offset;
            }

            inline int resolve(const MemoryOffsets & offsets) const
            {
                switch (type)
                {
                case MemoryOffsetType::OFFSET_VARIABLE:
                    return offsets.pVariable + offset;
                case MemoryOffsetType::OFFSET_CONSTANT:
                    return offsets.pConstants + offset;
                case MemoryOffsetType::OFFSET_TEMP_VAR:
                    return offsets.pTempVariable + offset;
                case MemoryOffsetType::OFFSET_RETURN_VALUE:
                    return offsets.pReturnValue + offset;
                }
                throw std::logic_error("Unresolvable memory offsets");
            }
        };

        std::wostream & operator<< (std::wostream & out, MEM const & mem);

        struct XMM
        {
            int reg;

            XMM()
            {
                reg = -1;
            }

            XMM(int _reg)
            {
                reg = _reg;
            }
        };

        std::wostream & operator<< (std::wostream & out, XMM const & xmm);

        // we have only used these two registers
        enum class GPREG
        {
            UNKNOWN,
            ECX,
            ESP,
        };

        struct REG
        {
            GPREG reg;

            REG()
            {
                reg = GPREG::UNKNOWN;
            }

            REG(GPREG _reg)
            {
                reg = _reg;
            }
        };

        std::wostream & operator<< (std::wostream & out, REG const & reg);

        struct MEMREF
        {
            REG ref;
            int offset;

            MEMREF(REG _ref, int _offset)
            {
                ref = _ref;
                offset = _offset;
            }

            inline static MEMREF ECX_REF(int ofs = 0)
            {
                return MEMREF(REG(GPREG::ECX), ofs);
            }

            inline static MEMREF ESP_REF(int ofs = 0)
            {
                return MEMREF(REG(GPREG::ESP), ofs);
            }
        };

        std::wostream & operator<< (std::wostream & out, MEMREF const & memref);
    }
}