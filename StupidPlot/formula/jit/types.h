#pragma once

#include <string>

using std::wstring;

namespace StupidPlot
{
    namespace Formula
    {
        namespace JIT
        {
            enum MemoryPositionType
            {
                DYNAMIC_SYMBOL_VAR,
                STATIC_CONSTANT,
                FUNCTION_CONSTANT,
                TEMP_VAR,
                INVALID,
            };

            struct MEM
            {
                MemoryPositionType type;
                int offsetInType;
                MEM()
                {
                    type = MemoryPositionType::INVALID;
                    offsetInType = 0;
                }
                MEM(MemoryPositionType _type, int _offset)
                {
                    type = _type;
                    offsetInType = _offset;
                }
                wstring toString()
                {
                    wstring offset;
                    if (type == MemoryPositionType::DYNAMIC_SYMBOL_VAR)
                    {
                        offset = L"P_VAR_SYMBOL";
                    }
                    else if (type == MemoryPositionType::FUNCTION_CONSTANT)
                    {
                        offset = L"P_CONST_FUNC";
                    }
                    else if (type == MemoryPositionType::STATIC_CONSTANT)
                    {
                        offset = L"P_CONST_STATIC";
                    }
                    else if (type == MemoryPositionType::TEMP_VAR)
                    {
                        offset = L"P_VAR_TEMP";
                    }
                    return wstring(L"[").append(offset).append(L" + ").append(std::to_wstring(offsetInType)).append(L"]");
                }
            };

            struct XMM
            {
                int offset;
                XMM()
                {
                    offset = 0;
                }
                XMM(int _offset)
                {
                    offset = _offset;
                }
                wstring toString()
                {
                    return wstring(L"XMM").append(std::to_wstring(offset));
                }
            };
        }
    }
}