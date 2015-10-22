#include <formula/token.h>

bool StupidPlot::Formula::Token::isTokenOperand(shared_ptr<Token> token, OperandType type)
{
    if (!token->is(TokenType::OPERAND)) return false;
    auto operandToken = std::dynamic_pointer_cast<OperandToken>(token);
    return operandToken->operandType == type;
}

bool StupidPlot::Formula::Token::isTokenOperator(shared_ptr<Token> token, OperatorType op)
{
    if (!token->is(TokenType::OPERATOR)) return false;
    auto operatorToken = std::dynamic_pointer_cast<OperatorToken>(token);
    return operatorToken->op == op;
}
