#pragma once

#include <memory>
#include <formula/jit/instructiongen.h>

using std::shared_ptr;

namespace StupidPlot
{
    namespace Formula
    {
        namespace JIT
        {
            static void add(shared_ptr<InstructionGen> insgen, XMM retXMM, XMM lXMM, XMM rXMM)
            {
                insgen->jit_sse_addsd_xmm_xmm(lXMM, rXMM);
                if (retXMM.offset != lXMM.offset) insgen->jit_sse_movsd_xmm_xmm(retXMM, lXMM);
            }

            static void subtract(shared_ptr<InstructionGen> insgen, XMM retXMM, XMM lXMM, XMM rXMM)
            {
                insgen->jit_sse_subsd_xmm_xmm(lXMM, rXMM);
                if (retXMM.offset != lXMM.offset) insgen->jit_sse_movsd_xmm_xmm(retXMM, lXMM);
            }

            static void multiply(shared_ptr<InstructionGen> insgen, XMM retXMM, XMM lXMM, XMM rXMM)
            {
                insgen->jit_sse_mulsd_xmm_xmm(lXMM, rXMM);
                if (retXMM.offset != lXMM.offset) insgen->jit_sse_movsd_xmm_xmm(retXMM, lXMM);
            }

            static void divide(shared_ptr<InstructionGen> insgen, XMM retXMM, XMM lXMM, XMM rXMM)
            {
                insgen->jit_sse_divsd_xmm_xmm(lXMM, rXMM);
                if (retXMM.offset != lXMM.offset) insgen->jit_sse_movsd_xmm_xmm(retXMM, lXMM);
            }

            static void sqrt(shared_ptr<InstructionGen> insgen, XMM retXMM, XMM vXMM)
            {
                insgen->jit_sse_sqrtsd_xmm(retXMM, vXMM);
            }

            static void sin(shared_ptr<InstructionGen> insgen, XMM retXMM, XMM vXMM)
            {
                MEM mem = insgen->reserve_temp_mem();
                insgen->jit_sse_movsd_mem_xmm(mem, vXMM);
                insgen->jit_x87_fld_st0_mem(mem);           // ST(0) = mem
                insgen->jit_x87_fsin();                     // ST(0) = sin(ST(0)
                insgen->jit_x87_fstp_mem_st0(mem);          // mem = ST(0)
                insgen->jit_sse_movsd_xmm_mem(retXMM, mem);
                insgen->release_temp_mem();
            }

            static void cos(shared_ptr<InstructionGen> insgen, XMM retXMM, XMM vXMM)
            {
                MEM mem = insgen->reserve_temp_mem();
                insgen->jit_sse_movsd_mem_xmm(mem, vXMM);
                insgen->jit_x87_fld_st0_mem(mem);           // ST(0) = mem
                insgen->jit_x87_fcos();                     // ST(0) = cos(ST(0)
                insgen->jit_x87_fstp_mem_st0(mem);          // mem = ST(0)
                insgen->jit_sse_movsd_xmm_mem(retXMM, mem);
                insgen->release_temp_mem();
            }

            static void negative(shared_ptr<InstructionGen> insgen, XMM retXMM, XMM vXMM)
            {
                UNREFERENCED_PARAMETER(insgen);
                UNREFERENCED_PARAMETER(retXMM);
                UNREFERENCED_PARAMETER(vXMM);
                // not implemented
            }

            static void ln(shared_ptr<InstructionGen> insgen, XMM retXMM, XMM vXMM)
            {
                UNREFERENCED_PARAMETER(insgen);
                UNREFERENCED_PARAMETER(retXMM);
                UNREFERENCED_PARAMETER(vXMM);
                // not implemented
            }

            static void lg(shared_ptr<InstructionGen> insgen, XMM retXMM, XMM vXMM)
            {
                UNREFERENCED_PARAMETER(insgen);
                UNREFERENCED_PARAMETER(retXMM);
                UNREFERENCED_PARAMETER(vXMM);
                // not implemented
            }
        }
    }
}