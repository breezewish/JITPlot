#include <windows.h>
#include <formula/functions.h>

using std::wstring;
using std::map;

namespace StupidPlot
{
    namespace Formula
    {
        const map<OperatorType, OperatorTranslateItem> operatorMapping = {
            { OperatorType::OP_ADD, OperatorTranslateItem(L"add", 2) },
            { OperatorType::OP_SUB, OperatorTranslateItem(L"subtract", 2) },
            { OperatorType::OP_MUL, OperatorTranslateItem(L"multiply", 2) },
            { OperatorType::OP_DIV, OperatorTranslateItem(L"divide", 2) },
            { OperatorType::OP_NEG, OperatorTranslateItem(L"negative", 1) },
            { OperatorType::OP_POW, OperatorTranslateItem(L"power", 2) },
        };

        bool isOperatorTranslatable(OperatorType op)
        {
            return operatorMapping.find(op) != operatorMapping.end();
        }

        OperatorTranslateItem translateOperator(OperatorType op)
        {
            return operatorMapping.find(op)->second;
        }

        void genAdd(int & constLength, map<double, int> & constMap, LIRInstructionSet & ins, XMM opnd1, XMM opnd2)
        {
            UNREFERENCED_PARAMETER(constLength);
            UNREFERENCED_PARAMETER(constMap);

            ins.push_back(LIRInstruction(LIROperation::ADD_XMM_XMM, opnd1, opnd2));
        }

        void genSubtract(int & constLength, map<double, int> & constMap, LIRInstructionSet & ins, XMM opnd1, XMM opnd2)
        {
            UNREFERENCED_PARAMETER(constLength);
            UNREFERENCED_PARAMETER(constMap);

            ins.push_back(LIRInstruction(LIROperation::SUB_XMM_XMM, opnd1, opnd2));
        }

        void genMultiply(int & constLength, map<double, int> & constMap, LIRInstructionSet & ins, XMM opnd1, XMM opnd2)
        {
            UNREFERENCED_PARAMETER(constLength);
            UNREFERENCED_PARAMETER(constMap);

            ins.push_back(LIRInstruction(LIROperation::MUL_XMM_XMM, opnd1, opnd2));
        }

        void genDivide(int & constLength, map<double, int> & constMap, LIRInstructionSet & ins, XMM opnd1, XMM opnd2)
        {
            UNREFERENCED_PARAMETER(constLength);
            UNREFERENCED_PARAMETER(constMap);

            ins.push_back(LIRInstruction(LIROperation::DIV_XMM_XMM, opnd1, opnd2));
        }

        const map<wstring, LIRGenerateFunction> functionMapping = {
            { L"add", LIRGenerateFunction(genAdd) },
            { L"subtract", LIRGenerateFunction(genSubtract) },
            { L"multiply", LIRGenerateFunction(genMultiply) },
            { L"divide", LIRGenerateFunction(genDivide) },
        };

        bool isFunctionCallResolvable(wstring str)
        {
            return functionMapping.find(str) != functionMapping.end();
        }

        LIRGenerateFunction resolveFunctionCall(wstring str)
        {
            return functionMapping.find(str)->second;
        }
    }
}