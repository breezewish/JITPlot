#pragma once

#include <sstream>
#include <cmath>

#include <list>
#include <stack>
#include <memory>
#include <string>

#include <formula/token.h>
#include <formula/lexer.h>
#include <formula/parser.h>

using std::wstring;
using std::list;
using std::stack;
using std::shared_ptr;

namespace StupidPlot
{
    namespace Formula
    {
        class Expression
        {
        protected:
            list<shared_ptr<Token>>     tokens;
        public:
            Expression(wstring exp)
            {
                tokens = Lexer::lex(exp);
                tokens = Parser::getRPN(tokens);
            }

            double evaluate(double x)
            {
                stack<double> c;

                for (auto pToken : tokens)
                {
                    if (pToken->is(TokenType::OPERAND))
                    {
                        auto _token = std::dynamic_pointer_cast<OperandToken>(pToken);
                        if (_token->operandType == OperandType::OPERAND_CONST)
                        {
                            auto token = std::dynamic_pointer_cast<ConstantOperandToken>(_token);
                            c.push(token->value);
                        }
                        else
                        {
                            c.push(x);
                        }
                    }
                    else if (pToken->is(TokenType::OPERATOR))
                    {
                        double a, b;
                        auto token = std::dynamic_pointer_cast<OperatorToken>(pToken);
                        switch (token->op)
                        {
                        case OperatorType::OP_ADD:
                            a = c.top(); c.pop();
                            b = c.top(); c.pop();
                            c.push(a + b);
                            break;
                        case OperatorType::OP_SUB:
                            a = c.top(); c.pop();
                            b = c.top(); c.pop();
                            c.push(a - b);
                            break;
                        case OperatorType::OP_MUL:
                            a = c.top(); c.pop();
                            b = c.top(); c.pop();
                            c.push(a * b);
                            break;
                        case OperatorType::OP_DIV:
                            a = c.top(); c.pop();
                            b = c.top(); c.pop();
                            c.push(a / b);
                            break;
                        }
                    }
                    else if (pToken->is(TokenType::FUNC_CALL))
                    {
                        double v = c.top(); c.pop();
                        c.push(std::sin(v));
                    }
                }

                return c.top();
            }
        };
        /*
        class FormulaNode
        {
        public:
            virtual ~FormulaNode() = 0;
            virtual double eval() = 0;
        };

        class ConstantFormulaNode : public FormulaNode
        {
        protected:
            double val;
        public:
            ConstantFormulaNode(double _v) : FormulaNode()
            {
                val = _v;
            }
            virtual double eval()
            {
                return val;
            }
        };

        class UnaryOperatorFormulaNode : public FormulaNode
        {
        protected:
            FormulaNode * node;
        public:
            UnaryOperatorFormulaNode(FormulaNode * _node) : FormulaNode()
            {
                node = _node;
            }
            virtual ~UnaryOperatorFormulaNode()
            {
                delete node;
            }
            virtual double eval() = 0;
        };

        class BinaryOperatorFormulaNode : public FormulaNode
        {
        protected:
            FormulaNode * left;
            FormulaNode * right;
        public:
            BinaryOperatorFormulaNode(FormulaNode * _left, FormulaNode * _right) : FormulaNode()
            {
                left = _left;
                right = _right;
            }
            virtual ~BinaryOperatorFormulaNode()
            {
                delete left;
                delete right;
            }
            virtual double eval() = 0;
        };

        class NegateFormulaNode : public UnaryOperatorFormulaNode
        {
        public:
            NegateFormulaNode(FormulaNode * _node) : UnaryOperatorFormulaNode(_node)
            {
            }
            virtual double eval()
            {
                return -node->eval();
            }
        };

        class AddFormulaNode : public BinaryOperatorFormulaNode
        {
        public:
            AddFormulaNode(FormulaNode * _l, FormulaNode * _r) : BinaryOperatorFormulaNode(_l, _r)
            {
            }
            virtual double eval()
            {
                return left->eval() + right->eval();
            }
        };

        class SubtractFormulaNode : public BinaryOperatorFormulaNode
        {
        public:
            SubtractFormulaNode(FormulaNode * _l, FormulaNode * _r) : BinaryOperatorFormulaNode(_l, _r)
            {
            }
            virtual double eval()
            {
                return left->eval() - right->eval();
            }
        };

        class MultiplyFormulaNode : public BinaryOperatorFormulaNode
        {
        public:
            MultiplyFormulaNode(FormulaNode * _l, FormulaNode * _r) : BinaryOperatorFormulaNode(_l, _r)
            {
            }
            virtual double eval()
            {
                return left->eval() * right->eval();
            }
        };

        class DivideFormulaNode : public BinaryOperatorFormulaNode
        {
        public:
            DivideFormulaNode(FormulaNode * _l, FormulaNode * _r) : BinaryOperatorFormulaNode(_l, _r)
            {
            }
            virtual double eval()
            {
                return left->eval() / right->eval();
            }
        };

        class PowerFormulaNode : public BinaryOperatorFormulaNode
        {
        public:
            PowerFormulaNode(FormulaNode * _l, FormulaNode * _r) : BinaryOperatorFormulaNode(_l, _r)
            {
            }
            virtual double eval()
            {
                return std::pow(left->eval(), right->eval());
            }
        };
        */
    }
}