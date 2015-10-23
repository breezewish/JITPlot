#pragma once

#include <string>

using std::wstring;

namespace StupidPlot
{
    namespace Formula
    {
        namespace JIT
        {
            struct BufferPosition
            {
                double * pDynamicVars;
                double * pConstVars;
                double * pFuncVars;
                double * pTempVars;
                double * pRetVar;
            };

            enum MemoryPositionType
            {
                DYNAMIC_SYMBOL_VAR,
                STATIC_CONSTANT,
                FUNCTION_CONSTANT,
                TEMP_VAR,
                RETURN_VAR,
                INVALID,
                RESOLVED,
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
                    switch (type)
                    {
                    case MemoryPositionType::DYNAMIC_SYMBOL_VAR:
                        offset = L"P_VAR_SYMBOL";
                        break;
                    case MemoryPositionType::FUNCTION_CONSTANT:
                        offset = L"P_CONST_FUNC";
                        break;
                    case MemoryPositionType::STATIC_CONSTANT:
                        offset = L"P_CONST_STATIC";
                        break;
                    case MemoryPositionType::TEMP_VAR:
                        offset = L"P_VAR_TEMP";
                        break;
                    case MemoryPositionType::RETURN_VAR:
                        offset = L"P_VAR_RET";
                        break;
                    case MemoryPositionType::RESOLVED:
                        return std::to_wstring(offsetInType);
                    default:
                        return L"(invalid)";
                    }

                    return wstring(L"[").append(offset).append(L" + ").append(std::to_wstring(offsetInType)).append(L"]");
                }

                void resolve(const BufferPosition & bufferPos)
                {
                    double * offset;
                    switch (type)
                    {
                    case MemoryPositionType::DYNAMIC_SYMBOL_VAR:
                        offset = bufferPos.pDynamicVars + offsetInType;
                        break;
                    case MemoryPositionType::FUNCTION_CONSTANT:
                        offset = bufferPos.pFuncVars + offsetInType;
                        break;
                    case MemoryPositionType::STATIC_CONSTANT:
                        offset = bufferPos.pConstVars + offsetInType;
                        break;
                    case MemoryPositionType::TEMP_VAR:
                        offset = bufferPos.pTempVars + offsetInType;
                        break;
                    case MemoryPositionType::RETURN_VAR:
                        offset = bufferPos.pRetVar + offsetInType;
                        break;
                    case MemoryPositionType::RESOLVED:
                        return;
                    default:
                        throw runtime_error("Invalid memory position");
                    }
                    type = MemoryPositionType::RESOLVED;
                    offsetInType = reinterpret_cast<int>(offset);
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