#include <iostream>
#include "parser.h"

namespace hsl {
    Parser::Parser(Lexer& l) : lexer(l) {
        nextToken();
        nextToken();
    }

    void Parser::nextToken() {
        curToken = peekToken;
        peekToken = lexer.nextToken();
    }

    bool Parser::curTokenIs(TokenType t) const {
        return curToken.type == t;
    }

    bool Parser::peekTokenIs(TokenType t) const {
        return peekToken.type == t;
    }

    bool Parser::expectPeek(TokenType t) {
        if (peekTokenIs(t)) {
            nextToken();
            return true;
        } else {
            errors.push_back("expected next token to be " + std::to_string((int)t));
            return false;
        }
    }

    Program* Parser::parseProgram() {
        auto* program = new Program();

        program->obj = parseObjDecl();
        program->vars = parseVarDeclList();
        program->constraints = parseStList();
        parseEndStmt();

        return program;
    }

    Objective* Parser::parseObjDecl() {
        if (!curTokenIs(TokenType::OBJ)) {
            errors.push_back("Expected [OBJ] at line " + std::to_string(curToken.line));
            return nullptr;
        }

        nextToken();
        bool isMax = false;
        if (curTokenIs(TokenType::MAX)) {
            isMax = true;
        } else if (curTokenIs(TokenType::MIN)) {
            isMax = false;
        } else {
            errors.emplace_back("Expected 'max' or 'min' after [OBJ]");
            return nullptr;
        }

        nextToken();
        Expression* expr = parseExpression();

        return new Objective{isMax, expr};
    } // obj_decl ::= "[OBJ]" ("max" | "min") expression ;

    VarDecl* Parser::parseVarDecl() {
        if (!curTokenIs(TokenType::VAR)) {
            errors.emplace_back("Expected [VAR]");
            return nullptr;
        }

        nextToken();
        if (!curTokenIs(TokenType::IDENT)) {
            errors.emplace_back("Expected identifier after [VAR]");
            return nullptr;
        }
        std::string name = curToken.literal;

        if (!expectPeek(TokenType::COMMA)) return nullptr;

        // min 범위 읽기 -> 다양한 숫자 형식에 대응되기 위해 표현식을 그냥 parsing하는 것으로 변경
        nextToken();
        Expression *lowerExpr = parseExpression();
        if (!lowerExpr) {
            errors.emplace_back("Invalid expression as lower bound");
            return nullptr;
        }

        if (!expectPeek(TokenType::COMMA)) return nullptr;

        // max 범위 읽기 -> 다양한 숫자 형식에 대응되기 위해 표현식을 그냥 parsing하는 것으로 변경
        nextToken();
        Expression *upperExpr = parseExpression();
        if (!upperExpr) {
            errors.emplace_back("Invalid expression as upper bound");
            return nullptr;
        }

        if (!expectPeek(TokenType::COMMA)) return nullptr;

        // type
        nextToken();
        bool isInt = false;
        if (curTokenIs(TokenType::INT)) {
            isInt = true;
        } else if (curTokenIs(TokenType::ANY)) {
            isInt = false;
        } else {
            errors.emplace_back("Expected 'int' or 'any' as type");
            return nullptr;
        }

        return new VarDecl{name, lowerExpr, upperExpr, isInt};
    } // var_decl ::= "[VAR]" identifier "," number "," number "," type ;

    std::vector<VarDecl*> Parser::parseVarDeclList() {
        std::vector<VarDecl*> vars;

        while (peekTokenIs(TokenType::VAR)) {
            nextToken();
            auto* c = parseVarDecl();
            if (c) {
                vars.emplace_back(c);
            } else {
                break;
            }
        }

        return vars;
    }

    Constraint* Parser::parseStDecl() {
        if (!curTokenIs(TokenType::ST)) {
            errors.emplace_back("Expected [ST] at line " + std::to_string(curToken.line));
            return nullptr;
        }

        nextToken();
        Expression* left = parseExpression();

        nextToken();
        TokenType comp = curToken.type;
        if (!(comp == TokenType::LEQ || comp == TokenType::LT ||
              comp == TokenType::GEQ || comp == TokenType::GT ||
              comp == TokenType::EQ  || comp == TokenType::NEQ)) {
            errors.emplace_back("Expected comparator at line " + std::to_string(curToken.line));
            return nullptr;
        }

        nextToken();
        Expression* right = parseExpression();

        return new Constraint{left, comp, right};
    } // st_decl ::= "[ST]" expression comparator expression ;

    std::vector<Constraint*> Parser::parseStList() {
        std::vector<Constraint*> constraints;

        while (peekTokenIs(TokenType::ST)) {
            nextToken();
            auto* c = parseStDecl();
            if (c) {
                constraints.emplace_back(c);
            } else {
                break;
            }
        }

        return constraints;
    }

    void Parser::parseEndStmt() {
        nextToken();

        if (!curTokenIs(TokenType::END)) {
            errors.emplace_back("Expected [END] at line " + std::to_string(curToken.line));
            return;
        }
        nextToken();
        if (!curTokenIs(TokenType::END_OF_FILE)) {
            errors.emplace_back("Expected none of code after [END] at line " + std::to_string(curToken.line));
        }
    }


    Expression* Parser::parseExpression(int precedence) {
        Expression* left = nullptr;

        switch (curToken.type) {
            case TokenType::IDENT:
                left = parseIdentifier();
                break;
            case TokenType::NUMBER_INT:
            case TokenType::NUMBER_FLOAT:
                left = parseNumber();
                break;
            case TokenType::LPAREN:
                left = parseGroupedExpr();
                break;
            case TokenType::MINUS: {
                nextToken();
                Expression* right = parseExpression(static_cast<int>(Precedence::PREFIX));
                left = new UnaryExpr{TokenType::MINUS, right};
                break;
            }
            default:
                errors.emplace_back("No prefix parse function for token: " + curToken.literal);
                return nullptr;
        }

        while (!peekTokenIs(TokenType::END_OF_FILE) &&
               precedence < tokenPrecedence(peekToken.type)) {
            nextToken(); // 연산자

            TokenType op = curToken.type;
            int prec = tokenPrecedence(op);
            int nextPrec = (op == TokenType::CARET) ? prec - 1 : prec;

            nextToken();
            Expression* right = parseExpression(nextPrec);

            left = new BinaryExpr{op, left, right};
        }

        return left;
    }

    Expression* Parser::parseIdentifier() {
        std::string name = curToken.literal;

        // 함수 호출 - function :== IDENT "(" expression ")"
        if (peekTokenIs(TokenType::LPAREN)) {
            expectPeek(TokenType::LPAREN); // '(' 로 이동
            return parseFunctionCall(name);
        }

        return new IdentExpr{name};
    }

    Expression* Parser::parseNumber() {
        bool isInt = (curToken.type == TokenType::NUMBER_INT);
        return new NumberExpr{std::stod(curToken.literal), isInt};
    }

    Expression* Parser::parseGroupedExpr() {
        nextToken(); // '(' 소비 후 다음 토큰 호출
        Expression* expr = parseExpression(0);
        if (!expectPeek(TokenType::RPAREN)) {
            errors.emplace_back("Expected ')' after expression");
            return nullptr;
        }
        return expr;
    }

    int Parser::tokenPrecedence(TokenType t) const {
        switch (t) {
            case TokenType::PLUS:
            case TokenType::MINUS:
                return static_cast<int>(Precedence::SUM);
            case TokenType::ASTERISK:
            case TokenType::SLASH:
                return static_cast<int>(Precedence::PRODUCT);
            case TokenType::CARET:
                return static_cast<int>(Precedence::POWER);
            default:
                return static_cast<int>(Precedence::LOWEST);
        }
    }

    Expression* Parser::parseFunctionCall(std::string funcName) {
        std::vector<Expression*> args;

        // 함수 입력값이 없는 경우는 바로 ) 호출
        if (peekTokenIs(TokenType::RPAREN)) {
            nextToken();
            return new FunctionCallExpr{std::move(funcName), std::move(args)};
        }

        // 입력값이 1개인 경우
        nextToken();
        args.push_back(parseExpression());

        // 입력값이 2개 이상의 경우에는 ','로 구분하고, 그 이후로 계속해서 인자값을 호출한다.
        while (peekTokenIs(TokenType::COMMA)) {
            nextToken();
            nextToken();
            args.push_back(parseExpression());
        }

        // 닫는 괄호 없을 때에는 당연히 에러.
        if (!expectPeek(TokenType::RPAREN)) {
            errors.emplace_back("Expected ')' after function arguments");
            return nullptr;
        }

        return new FunctionCallExpr{std::move(funcName), std::move(args)};
    }

}
