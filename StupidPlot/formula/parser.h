#pragma once

#include <list>
#include <stack>
#include <memory>
#include <formula/exception.h>
#include <formula/token.h>

using std::shared_ptr;
using std::list;

namespace StupidPlot
{
    namespace Formula
    {
        class Parser
        {
        protected:
            shared_ptr<Token> currentToken = shared_ptr<Token>(new EmptyToken());
            shared_ptr<Token> currentStackToken = shared_ptr<Token>(new EmptyToken());

            list<shared_ptr<Token>> tokens;
            list<shared_ptr<Token>> output;
            std::stack<shared_ptr<Token>> stack;
            int commas;

            void popStack()
            {
                if (stack.empty()) throw OutOfStackException();
                getStackTop();
                stack.pop();
            }

            void getStackTop()
            {
                if (stack.empty())
                {
                    currentStackToken = shared_ptr<Token>(new EmptyToken());
                }
                else
                {
                    currentStackToken = stack.top();
                }
            }

            int getCurrentTokenPrecedence()
            {
                return currentToken->getPrecedence();
            }

            int getCurrentStackTokenPrecedence()
            {
                getStackTop();
                return currentStackToken->getPrecedence();
            }

            void handleBracketRight()
            {
                commas = 0;
                for (;;)
                {
                    getStackTop();
                    if (currentStackToken->is(TokenType::BR_LEFT))
                    {
                        popStack();
                        getStackTop();
                        if (currentStackToken->is(TokenType::FUNC_NAME))
                        {
                            handleFunctionEnd();
                        }
                        return;
                    }
                    else if (currentStackToken->is(COMMA))
                    {
                        commas++;
                    }
                    else
                    {
                        outputOperator();
                    }
                    if (stack.empty()) throw OutOfStackException();
                    popStack();
                }
            }

            void handleRemainingStack()
            {
                while (!stack.empty())
                {
                    popStack();
                    if (currentStackToken->is(TokenType::BR_LEFT))
                    {
                        throw MissingRightBracketException();
                    }
                    else if (currentStackToken->is(TokenType::COMMA))
                    {
                        throw UnexpectedCommaException();
                    }
                    else
                    {
                        outputOperator();
                    }
                }
            }

            void handleFunctionStart()
            {
                // do nothing
            }

            void handleFunctionEnd()
            {
                outputOperator();
                popStack();
            }

            void handleOperator()
            {
                int precToken = getCurrentTokenPrecedence();
                int precStackToken = getCurrentStackTokenPrecedence();

                while (precToken <= precStackToken)
                {
                    if (currentStackToken->is(TokenType::COMMA))
                    {
                        if (currentToken->is(TokenType::COMMA))
                        {
                            break;
                        }
                    }
                    else
                    {
                        outputOperator();
                    }
                    popStack();
                    precStackToken = getCurrentStackTokenPrecedence();
                }
                stack.push(currentToken);
            }

            void outputOperator()
            {
                if (!currentStackToken->is(TokenType::FUNC_NAME))
                {
                    output.push_back(currentStackToken);
                }
                else
                {
                    output.push_back(currentStackToken);
                    output.push_back(shared_ptr<Token>(new FunctionArgumentNumberToken(commas + 1)));
                    commas = 0;
                }
            }

            void mergeFunctionCall()
            {
                for (auto itr = output.begin(); itr != output.end();)
                {
                    auto _token = *itr;
                    if (_token->is(FUNC_NAME))
                    {
                        auto token = std::dynamic_pointer_cast<FunctionNameToken>(_token);
                        auto _nextToken = *(std::next(itr));
                        if (_nextToken->is(FUNC_ARG))
                        {
                            auto nextToken = std::dynamic_pointer_cast<FunctionArgumentNumberToken>(_nextToken);
                            output.insert(itr, shared_ptr<Token>(new FunctionCallToken(
                                token->name,
                                nextToken->n
                                )));
                            output.erase(std::next(itr));
                            itr = output.erase(itr);
                            continue;
                        }
                    }
                    itr++;
                }
            }

        public:
            Parser(list<shared_ptr<Token>> & _tokens)
            {
                tokens = _tokens;
            }

            list<shared_ptr<Token>> toRPN()
            {
                for (auto pToken : tokens)
                {
                    currentToken = pToken;

                    if (pToken->is(TokenType::OPERAND))
                    {
                        output.push_back(pToken);
                    }
                    else if (pToken->is(TokenType::BR_LEFT))
                    {
                        stack.push(pToken);
                    }
                    else if (pToken->is(TokenType::BR_RIGHT))
                    {
                        handleBracketRight();
                    }
                    else if (pToken->is(TokenType::FUNC_NAME))
                    {
                        handleFunctionStart();
                        handleOperator();
                    }
                    else
                    {
                        handleOperator();
                    }
                }

                handleRemainingStack();

                // convert FUNC_NAME, FUNC_ARGS => FUNC_CALL
                mergeFunctionCall();

                return output;
            }

            static list<shared_ptr<Token>> getRPN(list<shared_ptr<Token>> & inputTokens)
            {
                return Parser(inputTokens).toRPN();
            }
        };
    }
}
