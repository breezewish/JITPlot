#pragma once

#include <list>
#include <map>
#include <vector>
#include <stack>
#include <string>
#include <memory>
#include <cmath>
#include <formula/token.h>
#include <formula/exception.h>
#include <formula/types.h>
#include <formula/lir.h>
#include <formula/functions.h>
#include <formula/executable.h>

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
        class Compiler
        {
        public:
            static ExecutablePtr compileRPN(
                list<shared_ptr<Token>> tokens,            // tokens
                const map<wstring, double> & constVars     // constant variables (name => value)
                )
            {
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
                            *itr = shared_ptr<Token>(new ConstantOperandToken(constVars.find(name)->second));
                            continue;
                        }

                        if (name == L"x")
                        {
                            continue;
                        }

                        throw UnknownSymbolException(name);
                    }

                    if (pToken->is(TokenType::FUNC_CALL))
                    {
                        auto token = dynamic_pointer_cast<FunctionCallToken>(pToken);

                        // find function
                        if (!isFunctionCallResolvable(token->name))
                        {
                            throw UnknownSymbolException(token->name);
                        }

                        // check parameter number
                        auto f = resolveFunctionCall(token->name);
                        if (f.operands != token->n)
                        {
                            throw ParameterNumberMismatchException(token->name, token->n, f.operands);
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
                        if (!isOperatorTranslatable(token->op))
                        {
                            throw OperatorNotImplementedException(token->getTokenValue());
                        }

                        auto func = translateOperator(token->op);
                        *itr = shared_ptr<Token>(new FunctionCallToken(func.name, func.operands));
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

                // 4. generate LIR instructions
                // for function calls, we always store parameters in an XMM register
                // and notify the function to put result into an XMM register

                LIRInstructionSet lirins;
                int depth = 0;  // xmm0 = x

                for (auto pToken : tokens)
                {
                    if (pToken->is(TokenType::OPERAND))
                    {
                        auto opToken = dynamic_pointer_cast<OperandToken>(pToken);
                        auto dest = XMM(min(depth + 1, 6));

                        if (opToken->operandType == OperandType::OPERAND_CONST)
                        {
                            // constant
                            auto token = dynamic_pointer_cast<ConstantOperandToken>(pToken);
                            auto src = MEM(MemoryOffsetType::OFFSET_CONSTANT, constantOffsets[token->value]);
                            lirins.push_back(LIRInstruction(LIROperation::LOAD_XMM_MEM, LIROperand(dest), LIROperand(src)));
                        }
                        else
                        {
                            // variable x
                            auto token = dynamic_pointer_cast<VariableOperandToken>(pToken);
                            lirins.push_back(LIRInstruction(LIROperation::MOV_XMM_XMM, LIROperand(dest), LIROperand(XMM(0))));
                        }

                        if (depth + 1 >= 6)
                        {
                            lirins.push_back(LIRInstruction(LIROperation::PUSH_MEM_XMM, LIROperand(XMM(6))));
                        }

                        depth++;
                    }
                    else if (pToken->is(TokenType::FUNC_CALL))
                    {
                        auto token = dynamic_pointer_cast<FunctionCallToken>(pToken);
                        auto func = resolveFunctionCall(token->name);

                        XMM opnd1, opnd2;

                        if (token->n >= 1 && depth >= 6)
                        {
                            lirins.push_back(LIRInstruction(LIROperation::POP_XMM_MEM, LIROperand(XMM(6))));
                        }

                        if (token->n >= 2 && depth >= 7)
                        {
                            lirins.push_back(LIRInstruction(LIROperation::POP_XMM_MEM, LIROperand(XMM(7))));
                        }

                        if (token->n == 1)
                        {
                            opnd1 = XMM(min(depth, 6));
                            func.genfunc1(usedConstants, constantOffsets, lirins, opnd1);
                            if (depth >= 6)
                            {
                                lirins.push_back(LIRInstruction(LIROperation::PUSH_MEM_XMM, LIROperand(XMM(6))));
                            }
                        }
                        else if (token->n == 2)
                        {
                            opnd1 = XMM(min(depth - 1, 6));
                            opnd2 = XMM(min(depth, 7));
                            func.genfunc2(usedConstants, constantOffsets, lirins, opnd1, opnd2);
                            if (depth - 1 >= 6)
                            {
                                lirins.push_back(LIRInstruction(LIROperation::PUSH_MEM_XMM, LIROperand(XMM(6))));
                            }
                            depth--;
                        }
                    }
                }

                // 5. allocate memory for storing constants
                auto exec = ExecutablePtr(new Executable(usedConstants, constantOffsets));

                // pre-body instruction: load address and x
                lirins.push_front(LIRInstruction(LIROperation::SET_BASE, LIROperand(reinterpret_cast<unsigned int>(exec->pBuffer))));
                lirins.insert(std::next(lirins.begin()), LIRInstruction(LIROperation::LOAD_XMM_MEM, LIROperand(XMM(0)), LIROperand(MEM(MemoryOffsetType::OFFSET_VARIABLE, 0))));

                // post-body instruction: store result and return
                lirins.push_back(LIRInstruction(LIROperation::STORE_MEM_XMM, LIROperand(MEM(MemoryOffsetType::OFFSET_RETURN_VALUE, 0)), LIROperand(XMM(0))));
                lirins.push_back(LIRInstruction(LIROperation::RET, LIROperand(0)));

                // 6. resolve memory positions
                MemoryOffsets offsets;
                offsets.pVariable = 0;
                offsets.pConstants = offsets.pVariable + 1;
                offsets.pReturnValue = offsets.pConstants + usedConstants;
                for (auto it = lirins.begin(); it != lirins.end(); ++it)
                {
                    if (it->operands >= 1 && it->operand1.type == LIROperandType::MEM)
                    {
                        it->operand1.mem.resolve(offsets);
                    }
                    if (it->operands >= 2 && it->operand2.type == LIROperandType::MEM)
                    {
                        it->operand2.mem.resolve(offsets);
                    }
                }

                // 7. generate machine code from LIR
                auto pt = exec->pCode;
                exec->beginWriteCode();
                for (auto ins : lirins) ins.generate(pt);
                exec->endWriteCode();

                /*
                auto ret = CompiledResultPtr(new CompiledResult());
                ret->insgen = insgen;
                ret->constantOffsets = constantOffsets;
                ret->dynamicVarOffsets = variableOffsets;*/

                for (auto instruction : lirins)
                {
                    Debug::Debug() << instruction >> Debug::writeln;
                }

                return exec;
            };
        };
    }
}