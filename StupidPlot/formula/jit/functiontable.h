#pragma once

#include <functional>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <formula/token.h>
#include <formula/jit/types.h>
#include <formula/jit/instructiongen.h>
#include <formula/jit/functions.h>

using std::vector;
using std::map;
using std::wstring;
using std::shared_ptr;

namespace StupidPlot
{
    namespace Formula
    {
        namespace JIT
        {
            struct OperatorTranslateItem
            {
                OperatorType op;
                wstring funcName;
                int parameterCount;

                OperatorTranslateItem()
                {
                }

                OperatorTranslateItem(OperatorType _op, wstring _name, int _n)
                {
                    op = _op;
                    funcName = _name;
                    parameterCount = _n;
                }
            };

            enum FunctionCallType
            {
                FROM_XMM_TO_XMM
            };

            struct FunctionCallItem
            {
                FunctionCallType type;
                wstring funcName;
                int parameterCount;
                union FunctionCallAddress
                {
                    void(*p1)(shared_ptr<InstructionGen>, XMM, XMM);
                    void(*p2)(shared_ptr<InstructionGen>, XMM, XMM, XMM);
                    void(*p3)(shared_ptr<InstructionGen>, XMM, XMM, XMM, XMM);
                } address;

                FunctionCallItem()
                {
                }

                FunctionCallItem(FunctionCallType _type, wstring _name, int _n)
                {
                    type = _type;
                    funcName = _name;
                    parameterCount = _n;
                }

                // insgen, ret, param1
                FunctionCallItem(wstring _name, void(*func)(shared_ptr<InstructionGen>, XMM, XMM))
                    : FunctionCallItem(FunctionCallType::FROM_XMM_TO_XMM, _name, 1)
                {
                    address.p1 = func;
                }

                // insgen, ret, param1, param2
                FunctionCallItem(wstring _name, void(*func)(shared_ptr<InstructionGen>, XMM, XMM, XMM))
                    : FunctionCallItem(FunctionCallType::FROM_XMM_TO_XMM, _name, 2)
                {
                    address.p2 = func;
                }

                // insgen, ret, param1, param2, param3
                FunctionCallItem(wstring _name, void(*func)(shared_ptr<InstructionGen>, XMM, XMM, XMM, XMM))
                    : FunctionCallItem(FunctionCallType::FROM_XMM_TO_XMM, _name, 3)
                {
                    address.p3 = func;
                }
            };

            static map<OperatorType, OperatorTranslateItem> getOperatorTranslateTable()
            {
                map<OperatorType, OperatorTranslateItem> table = {
                    { OperatorType::OP_ADD, OperatorTranslateItem(OperatorType::OP_ADD, L"add", 2) },
                    { OperatorType::OP_SUB, OperatorTranslateItem(OperatorType::OP_SUB, L"sub", 2) },
                    { OperatorType::OP_MUL, OperatorTranslateItem(OperatorType::OP_MUL, L"mul", 2) },
                    { OperatorType::OP_DIV, OperatorTranslateItem(OperatorType::OP_DIV, L"div", 2) },
                    { OperatorType::OP_NEG, OperatorTranslateItem(OperatorType::OP_NEG, L"neg", 2) },
                };
                return table;
            }

            static map<wstring, FunctionCallItem> getFunctionCallTable()
            {
                map<wstring, FunctionCallItem> table = {
                    { L"add", FunctionCallItem(L"add", add) },
                    { L"sub", FunctionCallItem(L"sub", subtract) },
                    { L"mul", FunctionCallItem(L"mul", multiply) },
                    { L"div", FunctionCallItem(L"div", divide) },
                    { L"neg", FunctionCallItem(L"neg", negative) },
                    { L"sqrt", FunctionCallItem(L"sqrt", sqrt) },
                    { L"sin", FunctionCallItem(L"sin", sin) },
                    { L"cos", FunctionCallItem(L"cos", cos) },
                    { L"ln", FunctionCallItem(L"ln", ln) },
                    { L"lg", FunctionCallItem(L"lg", lg) },
                };
                return table;
            }
        }
    }
}