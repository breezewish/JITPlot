#pragma once

#include <string>
#include <map>
#include <formula/token.h>
#include <formula/lir.h>

namespace StupidPlot
{
    namespace Formula
    {
        struct OperatorTranslateItem
        {
            std::wstring name;
            int operands;

            OperatorTranslateItem(std::wstring _name, int _operands)
            {
                name = _name;
                operands = _operands;
            }
        };

        bool isOperatorTranslatable(OperatorType op);

        OperatorTranslateItem translateOperator(OperatorType op);

        typedef void(*LIRGF1)(int & constLength, std::map<double, int> & constMap, LIRInstructionSet & ins, XMM opnd1);
        typedef void(*LIRGF2)(int & constLength, std::map<double, int> & constMap, LIRInstructionSet & ins, XMM opnd1, XMM opnd2);

        struct LIRGenerateFunction
        {
            int operands;
            union
            {
                LIRGF1 genfunc1;
                LIRGF2 genfunc2;
            };

            LIRGenerateFunction(LIRGF1 func)
            {
                operands = 1;
                genfunc1 = func;
            }

            LIRGenerateFunction(LIRGF2 func)
            {
                operands = 2;
                genfunc2 = func;
            }
        };

        bool isFunctionCallResolvable(wstring str);

        LIRGenerateFunction resolveFunctionCall(wstring name);
    }
}