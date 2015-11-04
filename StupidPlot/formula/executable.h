#pragma once

#include <map>
#include <memory.h>

#include <windows.h>

namespace StupidPlot
{
    namespace Formula
    {
        class Executable
        {
        public:
            double      * pBuffer;
            double      * pVariable;
            double      * pConstants;
            double      * pReturnValue;

            PBYTE       pCode;
            int         poolSize;

            Executable(const MemoryOffsets & offsets, const std::map<double, int> & constMap)
            {
                int elements = offsets.pEnd;

                pBuffer = new double[elements];
                pVariable = pBuffer + offsets.pVariable;
                pConstants = pBuffer + offsets.pConstants;
                pReturnValue = pBuffer + offsets.pReturnValue;

                // copy constants
                for (auto pair : constMap)
                {
                    pConstants[pair.second] = pair.first;
                }

                // allocate executable page
                SYSTEM_INFO siSysInfo;
                GetSystemInfo(&siSysInfo);
                poolSize = siSysInfo.dwPageSize;
                pCode = static_cast<PBYTE>(VirtualAlloc(NULL, poolSize, MEM_COMMIT, PAGE_EXECUTE_READ));
            }

            ~Executable()
            {
                delete pBuffer;
                VirtualFree(pCode, 0, MEM_RELEASE);
            }

            inline void beginWriteCode()
            {
                DWORD dwOldProtect;
                VirtualProtect(pCode, poolSize, PAGE_READWRITE, &dwOldProtect);
            }

            inline void endWriteCode()
            {
                DWORD dwOldProtect;
                VirtualProtect(pCode, poolSize, PAGE_EXECUTE_READ, &dwOldProtect);
            }

            inline double eval(double x)
            {
                *pVariable = x;
                reinterpret_cast<void(*)()>(pCode)();
                return *pReturnValue;
            }
        };

        typedef std::shared_ptr<Executable> ExecutablePtr;
    }
}