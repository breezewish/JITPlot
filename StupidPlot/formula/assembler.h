#pragma once

#include <map>

#include <windows.h>

#include <formula/types.h>

using std::map;

namespace StupidPlot
{
    namespace Formula
    {
        class Assembler
        {
        public:
            static map<GPREG, BYTE> RegisterNumber;

            // see: http://www.c-jump.com/CIS77/CPU/x86/lecture.html
            inline static void _MOD_RM(PBYTE & buffer, XMM dest, MEMREF src)
            {
                int mod, reg, rm;
                bool SIB = false;

                reg = dest.reg;

                if (src.offset == 0)
                {
                    mod = 0x0;
                }
                else if (-128 <= src.offset && src.offset <= 127)
                {
                    mod = 0x1;
                }
                else
                {
                    mod = 0x2;
                }

                if (src.ref.reg == GPREG::ECX)
                {
                    // for EAX,ECX,EDX,EBX,EBP,ESI,EDI, we needn't SIB mode
                    rm = RegisterNumber[src.ref.reg];
                }
                else
                {
                    // SIB mode
                    rm = 0x4;
                    SIB = true;
                }

                *buffer = byte((mod << 6) | (reg << 3) | rm); buffer += 1;

                if (SIB)
                {
                    // scale=b00=*1
                    // index=b100=null
                    // base=reg
                    *buffer = byte((0x4 << 3) | RegisterNumber[src.ref.reg]); buffer += 1;
                }

                if (src.offset == 0)
                {
                    // do nothing
                }
                else if (-128 <= src.offset && src.offset <= 127)
                {
                    *buffer = byte(src.offset); buffer += 1;
                }
                else
                {
                    *reinterpret_cast<int *>(buffer) = src.offset; buffer += 4;
                }
            }

            static void MOV_REG_IMM32(PBYTE & buffer, REG opnd1, int opnd2)
            {
                *buffer = byte(0xB8 | RegisterNumber[opnd1.reg]); buffer += 1;
                *reinterpret_cast<int *>(buffer) = opnd2; buffer += 4;
            }

            static void ADD_REG_IMM8(PBYTE & buffer, REG opnd1, byte opnd2)
            {
                *buffer = byte(0x83); buffer += 1;
                *buffer = byte(0xC0 | RegisterNumber[opnd1.reg]); buffer += 1;
                *buffer = opnd2; buffer += 1;
            }

            static void SUB_REG_IMM8(PBYTE & buffer, REG opnd1, byte opnd2)
            {
                *buffer = byte(0x83); buffer += 1;
                *buffer = byte(0xE8 | RegisterNumber[opnd1.reg]); buffer += 1;
                *buffer = opnd2; buffer += 1;
            }

            static void MOVSD_MEM_XMM(PBYTE & buffer, MEMREF opnd1, XMM opnd2)
            {
                *buffer = byte(0xF2); buffer += 1;
                *buffer = byte(0x0F); buffer += 1;
                *buffer = byte(0x11); buffer += 1;
                _MOD_RM(buffer, opnd2, opnd1);
            }

            static void MOVSD_XMM_MEM(PBYTE & buffer, XMM opnd1, MEMREF opnd2)
            {
                *buffer = byte(0xF2); buffer += 1;
                *buffer = byte(0x0F); buffer += 1;
                *buffer = byte(0x10); buffer += 1;
                _MOD_RM(buffer, opnd1, opnd2);
            }

            static void ADDSD_XMM_XMM(PBYTE & buffer, XMM opnd1, XMM opnd2)
            {
                *buffer = byte(0xF2); buffer += 1;
                *buffer = byte(0x0F); buffer += 1;
                *buffer = byte(0x58); buffer += 1;
                *buffer = byte(0xC0 | (opnd1.reg << 3) | opnd2.reg); buffer += 1;
            }

            static void SUBSD_XMM_XMM(PBYTE & buffer, XMM opnd1, XMM opnd2)
            {
                *buffer = byte(0xF2); buffer += 1;
                *buffer = byte(0x0F); buffer += 1;
                *buffer = byte(0x5C); buffer += 1;
                *buffer = byte(0xC0 | (opnd1.reg << 3) | opnd2.reg); buffer += 1;
            }

            static void MULSD_XMM_XMM(PBYTE & buffer, XMM opnd1, XMM opnd2)
            {
                *buffer = byte(0xF2); buffer += 1;
                *buffer = byte(0x0F); buffer += 1;
                *buffer = byte(0x59); buffer += 1;
                *buffer = byte(0xC0 | (opnd1.reg << 3) | opnd2.reg); buffer += 1;
            }

            static void DIVSD_XMM_XMM(PBYTE & buffer, XMM opnd1, XMM opnd2)
            {
                *buffer = byte(0xF2); buffer += 1;
                *buffer = byte(0x0F); buffer += 1;
                *buffer = byte(0x5E); buffer += 1;
                *buffer = byte(0xC0 | (opnd1.reg << 3) | opnd2.reg); buffer += 1;
            }

            static void MOVAPD_XMM_XMM(PBYTE & buffer, XMM opnd1, XMM opnd2)
            {
                *buffer = byte(0x66); buffer += 1;
                *buffer = byte(0x0F); buffer += 1;
                *buffer = byte(0x28); buffer += 1;
                *buffer = byte(0xC0 | (opnd1.reg << 3) | opnd2.reg); buffer += 1;
            }

            static void SQRTSD_XMM_XMM(PBYTE & buffer, XMM opnd1, XMM opnd2)
            {
                *buffer = byte(0xF2); buffer += 1;
                *buffer = byte(0x0F); buffer += 1;
                *buffer = byte(0x51); buffer += 1;
                *buffer = byte(0xC0 | (opnd1.reg << 3) | opnd2.reg); buffer += 1;
            }

            static void RET(PBYTE & buffer, unsigned short n)
            {
                if (n == 0)
                {
                    *buffer = byte(0xC3); buffer += 1;
                }
                else
                {
                    *buffer = byte(0xC2); buffer += 1;
                    *reinterpret_cast<short *>(buffer) = n; buffer += 2;
                }
            }
        };
    }
}