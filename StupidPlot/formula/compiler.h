#pragma once

#include <list>
#include <map>
#include <vector>
#include <stack>
#include <string>
#include <memory>
#include <formula/token.h>
#include <formula/exception.h>
#include <formula/jit/types.h>
#include <formula/jit/instructions.h>
#include <formula/jit/functiontable.h>
#include <formula/jit/functions.h>
#include <formula/jit/instructiongen.h>

using std::list;
using std::shared_ptr;
using std::wstring;
using std::map;
using std::vector;
using std::stack;
using std::dynamic_pointer_cast;

namespace StupidPlot
{
    namespace Formula
    {
        using namespace JIT;

        struct CompiledResult
        {
            shared_ptr<InstructionGen>  insgen;
            map<wstring, int>           dynamicVarOffsets;
            map<double, int>            constantOffsets;
        };

        class Compiler
        {
        public:
            static CompiledResult compileRPN(
                list<shared_ptr<Token>> tokens,     // tokens
                map<wstring, double> & constVars,   // constant variables (name => value)
                vector<wstring> dynamicVars         // runtime variables, like 'x' (name)
                )
            {
                auto opTable = JIT::getOperatorTranslateTable();
                auto funcTable = JIT::getFunctionCallTable();

                int usedDynamicVars = 0;
                map<wstring, int> dynamicVarOffsets;

                int usedConstants = 0;
                map<double, int> constantOffsets;

                // 1. try to resolve symbols (variable and functions)
                for (auto itr = tokens.begin(); itr != tokens.end(); itr++)
                {
                    auto pToken = *itr;

                    if (Token::isTokenOperand(pToken, OperandType::OPERAND_VAR))
                    {
                        auto token = dynamic_pointer_cast<VariableOperandToken>(pToken);
                        wstring name = token->name;

                        // find var in constVars and replace
                        if (constVars.find(name) != constVars.end())
                        {
                            *itr = shared_ptr<Token>(new ConstantOperandToken(constVars[name]));
                            continue;
                        }

                        // find var in dynamicVars
                        if (std::find(dynamicVars.begin(), dynamicVars.end(), name) != dynamicVars.end())
                        {
                            // we needn't two spaces for the same variable
                            if (dynamicVarOffsets.find(name) == dynamicVarOffsets.end())
                            {
                                dynamicVarOffsets[name] = usedDynamicVars;
                                usedDynamicVars++;
                            }
                            continue;
                        }

                        // not found
                        throw UnknownSymbolException(name);
                    }

                    if (pToken->is(TokenType::FUNC_CALL))
                    {
                        auto token = dynamic_pointer_cast<FunctionCallToken>(pToken);

                        // find function
                        if (funcTable.find(token->name) == funcTable.end())
                        {
                            throw UnknownSymbolException(token->name);
                        }

                        // check parameter number
                        if (funcTable[token->name].parameterCount != token->n)
                        {
                            throw ParameterNumberMismatchException(token->name, token->n, funcTable[token->name].parameterCount);
                        }
                    }
                }

                // 2. convert operators to functions
                for (auto itr = tokens.begin(); itr != tokens.end(); itr++)
                {
                    auto pToken = *itr;

                    if (pToken->is(TokenType::OPERATOR))
                    {
                        auto token = dynamic_pointer_cast<OperatorToken>(pToken);
                        if (opTable.find(token->op) == opTable.end())
                        {
                            throw OperatorNotImplementedException(token->getTokenValue());
                        }

                        auto func = opTable[token->op];

                        *itr = shared_ptr<Token>(new FunctionCallToken(func.funcName, func.parameterCount));
                    }
                }

                // 3. analyze how much memory should we used to store constants
                // notice that we cannot directly move constants into registers
                for (auto pToken : tokens)
                {
                    if (pToken->is(TokenType::OPERAND))
                    {
                        auto opToken = dynamic_pointer_cast<OperandToken>(pToken);
                        if (opToken->operandType == OperandType::OPERAND_CONST)
                        {
                            auto token = dynamic_pointer_cast<ConstantOperandToken>(pToken);
                            if (constantOffsets.find(token->value) == constantOffsets.end())
                            {
                                constantOffsets[token->value] = usedConstants;
                                usedConstants++;
                            }
                        }
                    }
                }

                auto insgen = shared_ptr<JIT::InstructionGen>(new JIT::InstructionGen(usedDynamicVars, usedConstants));

                // 4. generate instructions
                // for function calls, we always store parameters in an XMM register
                // and notify the function to put result into an XMM register
                int stackTop = -1;      // xmm stack

                for (auto pToken : tokens)
                {
                    if (pToken->is(TokenType::OPERAND))
                    {
                        auto opToken = dynamic_pointer_cast<OperandToken>(pToken);
                        if (opToken->operandType == OperandType::OPERAND_CONST)
                        {
                            // constant
                            auto token = dynamic_pointer_cast<ConstantOperandToken>(pToken);
                            insgen->jit_sse_movsd_xmm_mem(
                                ++stackTop,                                                             // dst xmm reg
                                MEM(MemoryPositionType::STATIC_CONSTANT, constantOffsets[token->value]) // src memory
                                );
                        }
                        else
                        {
                            auto token = dynamic_pointer_cast<VariableOperandToken>(pToken);
                            insgen->jit_sse_movsd_xmm_mem(
                                ++stackTop,                                                                     // dst xmm reg
                                MEM(MemoryPositionType::DYNAMIC_SYMBOL_VAR, dynamicVarOffsets[token->name])     // src memory
                                );
                        }
                    }
                    else if (pToken->is(TokenType::FUNC_CALL))
                    {
                        auto token = dynamic_pointer_cast<FunctionCallToken>(pToken);
                        // calculate the reg num of each parameter
                        // notice: we push parameters from left to right in RPN
                        // for example, func(1,2,3) becomes [1],[2],[3],[func_call(3)]
                        // and we scan RPN from left to right.

                        vector<XMM> pXMM;
                        for (int i = 0; i < token->n; ++i)
                        {
                            pXMM.push_back(XMM(stackTop - token->n + i + 1));
                        }

                        stackTop -= token->n;
                        XMM retXMM(++stackTop);

                        FunctionCallItem func = funcTable[token->name];
                        if (token->n == 1)
                        {
                            func.address.p1(insgen, retXMM, pXMM[0]);
                        }
                        else if (token->n == 2)
                        {
                            func.address.p2(insgen, retXMM, pXMM[0], pXMM[1]);
                        }
                        else if (token->n == 3)
                        {
                            func.address.p3(insgen, retXMM, pXMM[0], pXMM[1], pXMM[2]);
                        }
                    }
                }

                CompiledResult ret;
                ret.insgen = insgen;
                ret.constantOffsets = constantOffsets;
                ret.dynamicVarOffsets = dynamicVarOffsets;
                return ret;
            };
        };
    }
}