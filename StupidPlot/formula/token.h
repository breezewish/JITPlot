#pragma once

#include <string>
#include <memory>

using std::shared_ptr;
using std::string;
using std::wstring;

namespace StupidPlot
{
    namespace Formula
    {
        enum TokenType
        {
            BR_LEFT,
            BR_RIGHT,
            OPERATOR,
            OPERAND,
            COMMA,
            FUNC_NAME,
            FUNC_ARG,
            FUNC_CALL,
            EMPTY,
        };

        enum OperatorType
        {
            OP_ADD,
            OP_SUB,
            OP_MUL,
            OP_DIV,
            OP_NEG,
            OP_POW,
        };

        enum OperandType
        {
            OPERAND_CONST,
            OPERAND_VAR,
        };

        class Token
        {
        public:

            static bool isTokenOperand(shared_ptr<Token> token, OperandType type);

            static bool isTokenOperator(shared_ptr<Token> token, OperatorType op);

            TokenType type;

            Token(TokenType _type)
            {
                type = _type;
            }

            ~Token()
            {
            }

            bool is(TokenType _type)
            {
                return type == _type;
            }

            virtual int getPrecedence()
            {
                return 0;
            }

            virtual wstring getTokenName() = 0;

            virtual wstring getTokenValue() = 0;

            virtual wstring toString()
            {
                return getTokenName() + L":" + getTokenValue();
            }
        };

        class EmptyToken : public Token
        {
        public:
            EmptyToken() : Token(TokenType::EMPTY)
            {
            }
            virtual wstring getTokenName()
            {
                return L"EMPTY";
            }
            virtual wstring getTokenValue()
            {
                return L"?";
            }
        };

        class FunctionNameToken : public Token
        {
        public:
            wstring name;
            FunctionNameToken(wstring _name) : Token(TokenType::FUNC_NAME)
            {
                name = _name;
            }
            virtual int getPrecedence()
            {
                return 200;
            }
            virtual wstring getTokenName()
            {
                return L"FUNC_NAME";
            }
            virtual wstring getTokenValue()
            {
                return name;
            }
        };

        class FunctionArgumentNumberToken : public Token
        {
        public:
            int n;
            FunctionArgumentNumberToken(int _n) : Token(TokenType::FUNC_ARG)
            {
                n = _n;
            }
            virtual wstring getTokenName()
            {
                return L"FUNC_ARG";
            }
            virtual wstring getTokenValue()
            {
                return std::to_wstring(n);
            }
        };

        class FunctionCallToken : public Token
        {
        public:
            wstring name;
            int n;
            FunctionCallToken(wstring _name, int _n) : Token(TokenType::FUNC_CALL)
            {
                name = _name;
                n = _n;
            }
            virtual wstring getTokenName()
            {
                return L"FUNC_CALL";
            }
            virtual wstring getTokenValue()
            {
                return name.append(L",n=").append(std::to_wstring(n));
            }
        };

        class LeftBracketToken : public Token
        {
        public:
            LeftBracketToken() : Token(TokenType::BR_LEFT)
            {
            }
            virtual int getPrecedence()
            {
                return 10;
            }
            virtual wstring getTokenName()
            {
                return L"BR_LEFT";
            }
            virtual wstring getTokenValue()
            {
                return L"(";
            }
        };

        class RightBracketToken : public Token
        {
        public:
            RightBracketToken() : Token(TokenType::BR_RIGHT)
            {
            }
            virtual wstring getTokenName()
            {
                return L"BR_RIGHT";
            }
            virtual wstring getTokenValue()
            {
                return L")";
            }
        };

        class OperatorToken : public Token
        {
        public:
            OperatorType op;
            OperatorToken(OperatorType _op) : Token(TokenType::OPERATOR)
            {
                op = _op;
            }
            virtual int getPrecedence()
            {
                if (op == OperatorType::OP_SUB || op == OperatorType::OP_ADD) return 50;
                if (op == OperatorType::OP_NEG) return 60;
                if (op == OperatorType::OP_MUL || op == OperatorType::OP_DIV) return 70;
                if (op == OperatorType::OP_POW) return 80;
                return 0;
            }
            virtual wstring getTokenName()
            {
                return L"OPERATOR";
            }
            virtual wstring getTokenValue()
            {
                switch (op)
                {
                case OperatorType::OP_ADD:
                    return L"+";
                case OperatorType::OP_DIV:
                    return L"/";
                case OperatorType::OP_MUL:
                    return L"*";
                case OperatorType::OP_NEG:
                    return L"~";
                case OperatorType::OP_POW:
                    return L"^";
                case OperatorType::OP_SUB:
                    return L"-";
                default:
                    return L"?";
                }
            }
        };

        class OperandToken : public Token
        {
        public:
            OperandType operandType;

            OperandToken(OperandType _operandType) : Token(TokenType::OPERAND)
            {
                operandType = _operandType;
            }
            virtual wstring getTokenName()
            {
                return L"OPERAND";
            }
        };

        class ConstantOperandToken : public OperandToken
        {
        public:
            double value;
            ConstantOperandToken(double _v) : OperandToken(OperandType::OPERAND_CONST)
            {
                value = _v;
            }
            virtual wstring getTokenValue()
            {
                return std::to_wstring(value);
            }
        };

        class VariableOperandToken : public OperandToken
        {
        public:
            wstring name;
            VariableOperandToken(wstring _name) : OperandToken(OperandType::OPERAND_VAR)
            {
                name = _name;
            }
            virtual wstring getTokenValue()
            {
                return name;
            }
        };

        class CommaToken : public Token
        {
        public:
            CommaToken() : Token(TokenType::COMMA)
            {
            }
            virtual int getPrecedence()
            {
                return 20;
            }
            virtual wstring getTokenName()
            {
                return L"COMMA";
            }
            virtual wstring getTokenValue()
            {
                return L",";
            }
        };
    }
}
