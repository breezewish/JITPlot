#pragma once

#include <sstream>
#include <string>
#include <list>
#include <cctype>
#include <memory>
#include <functional>

#include <formula/exception.h>
#include <formula/token.h>

namespace StupidPlot
{
    namespace Formula
    {
        using std::shared_ptr;
        using std::list;
        using std::wstring;
        using std::wistringstream;
        using std::function;

        class Lexer
        {
        protected:
            // Extract characters from stream while character matches some rules
            static wstring extractMatch(
                wistringstream & stream,
                function<int(wchar_t)> tester,
                wstring initial = L""
                )
            {
                wstring val = initial;
                val.reserve(40);

                wchar_t c = 0;
                while (stream.get(c))
                {
                    if (!tester(c)) break;
                    val.append(1, c);
                }

                if (c != 0 && !tester(c))
                {
                    stream.putback(c);
                }

                val.shrink_to_fit();
                return val;
            }

        public:
            static list<shared_ptr<Token>> lex(const wstring & input)
            {
                list<shared_ptr<Token>> tokens;

                // 1. basic lex
                wistringstream in(input);
                wchar_t c;

                while (in.get(c))
                {
                    if (c == L' ' || c == L'\t' || c == L'\r' || c == L'\n') continue;

                    // number starting with "."
                    if (c == L'.')
                    {
                        wstring number = extractMatch(in, std::isdigit, L".");
                        tokens.push_back(shared_ptr<Token>(new ConstantOperandToken(std::stod(number))));
                        continue;
                    }

                    // number which may contain fraction part
                    if (std::isdigit(c))
                    {
                        wstring number = extractMatch(in, std::isdigit, wstring(1, c));
                        wchar_t dot = in.peek();
                        // contains fraction
                        if (dot != EOF && dot == L'.')
                        {
                            in.get(c);
                            wstring fract = extractMatch(in, std::isdigit);
                            number += L"." + fract;
                        }
                        tokens.push_back(shared_ptr<Token>(new ConstantOperandToken(std::stod(number))));
                        continue;
                    }

                    // identifier
                    if (std::isalpha(c))
                    {
                        wstring name = extractMatch(in, std::isalnum, wstring(1, c));
                        tokens.push_back(shared_ptr<Token>(new VariableOperandToken(name)));
                        continue;
                    }

                    // bracket
                    if (c == L'(')
                    {
                        tokens.push_back(shared_ptr<Token>(new LeftBracketToken()));
                        continue;
                    }

                    if (c == L')')
                    {
                        tokens.push_back(shared_ptr<Token>(new RightBracketToken()));
                        continue;
                    }

                    // operator
                    if (c == L'+')
                    {
                        tokens.push_back(shared_ptr<Token>(new OperatorToken(OperatorType::OP_ADD)));
                        continue;
                    }

                    if (c == L'-')
                    {
                        tokens.push_back(shared_ptr<Token>(new OperatorToken(OperatorType::OP_SUB)));
                        continue;
                    }

                    if (c == L'*')
                    {
                        tokens.push_back(shared_ptr<Token>(new OperatorToken(OperatorType::OP_MUL)));
                        continue;
                    }

                    if (c == L'/')
                    {
                        tokens.push_back(shared_ptr<Token>(new OperatorToken(OperatorType::OP_DIV)));
                        continue;
                    }

                    // comma
                    if (c == L',')
                    {
                        tokens.push_back(shared_ptr<Token>(new CommaToken()));
                        continue;
                    }

                    throw UnexpectedSymbolException(wstring(1, c));
                }

                // 2. recognize function call "abc("
                for (auto itr = tokens.begin(); itr != tokens.end(); ++itr)
                {
                    auto pToken = *itr;
                    if (
                        Token::isTokenOperand(pToken, OperandType::OPERAND_VAR)
                        && (std::next(itr) != tokens.end()))
                    {
                        auto nextToken = *(std::next(itr));
                        if (nextToken->is(TokenType::BR_LEFT))
                        {
                            *itr = shared_ptr<Token>(new FunctionNameToken(pToken->getTokenValue()));
                        }
                    }
                }

                // 3. recognize -x
                for (auto itr = tokens.begin(); itr != tokens.end(); ++itr)
                {
                    if (Token::isTokenOperator(*itr, OperatorType::OP_SUB))
                    {
                        // test previous token for the following cases
                        // "-x", "(-x", ",-x"
                        auto prev = std::prev(itr);
                        if (
                            prev == tokens.end()
                            || (*prev)->is(TokenType::BR_LEFT)
                            || (*prev)->is(TokenType::COMMA)
                            )
                        {
                            // for sub, change to neg
                            *itr = shared_ptr<Token>(new OperatorToken(OP_NEG));
                        }
                    }
                }

                // 4. recognize 2x -> 2*x, 2sin -> 2*sin
                for (auto itr = tokens.begin(); itr != tokens.end();)
                {
                    if (
                        itr != tokens.begin()
                        && Token::isTokenOperand(*(std::prev(itr)), OperandType::OPERAND_CONST)
                        )
                    {
                        auto pToken = *itr;
                        if (
                            pToken->is(TokenType::FUNC_NAME)
                            || Token::isTokenOperand(pToken, OperandType::OPERAND_VAR)
                            )
                        {
                            itr = tokens.insert(itr, shared_ptr<Token>(new OperatorToken(OP_MUL)));
                            continue;
                        }
                    }

                    itr++;
                }

                return tokens;
            }
        };
    }
}