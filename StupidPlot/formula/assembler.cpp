#include <formula/assembler.h>

namespace StupidPlot
{
    namespace Formula
    {
        map<GPREG, BYTE> Assembler::RegisterNumber = {
            {GPREG::ECX, byte(0x1)},
            {GPREG::ESP, byte(0x4)}
        };
    }
}