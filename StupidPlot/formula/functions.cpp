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
            { OperatorType::OP_SUB, OperatorTranslateItem(L"sub", 2) },
            { OperatorType::OP_MUL, OperatorTranslateItem(L"mul", 2) },
            { OperatorType::OP_DIV, OperatorTranslateItem(L"div", 2) },
            { OperatorType::OP_NEG, OperatorTranslateItem(L"neg", 1) },
            { OperatorType::OP_POW, OperatorTranslateItem(L"pow", 2) },
        };

        bool isOperatorTranslatable(OperatorType op)
        {
            return operatorMapping.find(op) != operatorMapping.end();
        }

        OperatorTranslateItem translateOperator(OperatorType op)
        {
            return operatorMapping.find(op)->second;
        }

        inline int storeAndGetOffset(int & constLength, map<double, int> & constMap, double v)
        {
            int offset;

            if (constMap.find(v) == constMap.end())
            {
                offset = constMap[v] = constLength;
                constLength++;
            }
            else
            {
                offset = constMap.find(v)->second;
            }
            return offset;
        }

        void genAdd(int & constLength, map<double, int> & constMap, LIRInstructionSet & ins, XMM opnd1, XMM opnd2)
        {
            UNREFERENCED_PARAMETER(constLength);
            UNREFERENCED_PARAMETER(constMap);

            ins.push_back(LIRInstruction(LIROperation::ADD_XMM_XMM, opnd1, opnd2));
        }

        void genSub(int & constLength, map<double, int> & constMap, LIRInstructionSet & ins, XMM opnd1, XMM opnd2)
        {
            UNREFERENCED_PARAMETER(constLength);
            UNREFERENCED_PARAMETER(constMap);

            ins.push_back(LIRInstruction(LIROperation::SUB_XMM_XMM, opnd1, opnd2));
        }

        void genMul(int & constLength, map<double, int> & constMap, LIRInstructionSet & ins, XMM opnd1, XMM opnd2)
        {
            UNREFERENCED_PARAMETER(constLength);
            UNREFERENCED_PARAMETER(constMap);

            ins.push_back(LIRInstruction(LIROperation::MUL_XMM_XMM, opnd1, opnd2));
        }

        void genDiv(int & constLength, map<double, int> & constMap, LIRInstructionSet & ins, XMM opnd1, XMM opnd2)
        {
            UNREFERENCED_PARAMETER(constLength);
            UNREFERENCED_PARAMETER(constMap);

            ins.push_back(LIRInstruction(LIROperation::DIV_XMM_XMM, opnd1, opnd2));
        }

        void genSqrt(int & constLength, map<double, int> & constMap, LIRInstructionSet & ins, XMM opnd1)
        {
            UNREFERENCED_PARAMETER(constLength);
            UNREFERENCED_PARAMETER(constMap);

            ins.push_back(LIRInstruction(LIROperation::SQRT_XMM, opnd1));
        }

        void genSin(int & constLength, map<double, int> & constMap, LIRInstructionSet & ins, XMM opnd1)
        {
            UNREFERENCED_PARAMETER(constLength);
            UNREFERENCED_PARAMETER(constMap);

            ins.push_back(LIRInstruction(LIROperation::MOV_ST0_XMM, LIROperand(opnd1)));
            ins.push_back(LIRInstruction(LIROperation::SIN_ST0));
            ins.push_back(LIRInstruction(LIROperation::MOV_XMM_ST0, LIROperand(opnd1)));
        }

        void genCos(int & constLength, map<double, int> & constMap, LIRInstructionSet & ins, XMM opnd1)
        {
            UNREFERENCED_PARAMETER(constLength);
            UNREFERENCED_PARAMETER(constMap);

            ins.push_back(LIRInstruction(LIROperation::MOV_ST0_XMM, LIROperand(opnd1)));
            ins.push_back(LIRInstruction(LIROperation::COS_ST0));
            ins.push_back(LIRInstruction(LIROperation::MOV_XMM_ST0, LIROperand(opnd1)));
        }

        void genNeg(int & constLength, map<double, int> & constMap, LIRInstructionSet & ins, XMM opnd1)
        {
            int offset = storeAndGetOffset(constLength, constMap, -0.0);
            ins.push_back(LIRInstruction(LIROperation::LOAD_XMM_MEM, LIROperand(XMM(7)), LIROperand(MEM(MemoryOffsetType::OFFSET_CONSTANT, offset))));
            ins.push_back(LIRInstruction(LIROperation::XOR_XMM_XMM, LIROperand(opnd1), LIROperand(XMM(7))));
        }

        void genAbs(int & constLength, map<double, int> & constMap, LIRInstructionSet & ins, XMM opnd1)
        {
            int offset = storeAndGetOffset(constLength, constMap, -0.0);
            ins.push_back(LIRInstruction(LIROperation::LOAD_XMM_MEM, LIROperand(XMM(7)), LIROperand(MEM(MemoryOffsetType::OFFSET_CONSTANT, offset))));
            ins.push_back(LIRInstruction(LIROperation::ANDNOT_XMM_XMM, LIROperand(XMM(7)), LIROperand(opnd1)));
            ins.push_back(LIRInstruction(LIROperation::MOV_XMM_XMM, LIROperand(opnd1), LIROperand(XMM(7))));
        }

        void genPower(int & constLength, map<double, int> & constMap, LIRInstructionSet & ins, XMM opnd1, XMM opnd2)
        {
            UNREFERENCED_PARAMETER(constLength);
            UNREFERENCED_PARAMETER(constMap);

            ins.push_back(LIRInstruction(LIROperation::MOV_ST0_XMM, LIROperand(opnd2)));
            ins.push_back(LIRInstruction(LIROperation::MOV_ST0_XMM, LIROperand(opnd1)));
            ins.push_back(LIRInstruction(LIROperation::POWER_ST0_ST1));
            ins.push_back(LIRInstruction(LIROperation::MOV_XMM_ST0, LIROperand(opnd1)));
        }

        const map<wstring, LIRGenerateFunction> functionMapping = {
            { L"add", LIRGenerateFunction(genAdd) },
            { L"sub", LIRGenerateFunction(genSub) },
            { L"mul", LIRGenerateFunction(genMul) },
            { L"div", LIRGenerateFunction(genDiv) },
            { L"sqrt", LIRGenerateFunction(genSqrt) },
            { L"sin", LIRGenerateFunction(genSin) },
            { L"cos", LIRGenerateFunction(genCos) },
            { L"neg", LIRGenerateFunction(genNeg) },
            { L"abs", LIRGenerateFunction(genAbs) },
            { L"pow", LIRGenerateFunction(genPower) },
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