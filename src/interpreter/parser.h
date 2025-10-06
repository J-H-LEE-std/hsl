#ifndef HSL_PARSER_
#define HSL_PARSER_

#include "lexer.h"
#include "token.h"
#include "ast.h"

namespace hsl {
    class Parser {
    public:
        explicit Parser(Lexer& lexer);

        Program* parseProgram();

        // 에러 로그 접근
        [[nodiscard]] const std::vector<std::string>& getErrors() const { return errors; }

    private:
        Lexer& lexer;
        Token curToken;
        Token peekToken;
        std::vector<std::string> errors;

        void nextToken();

        [[nodiscard]] bool curTokenIs(TokenType t) const;
        [[nodiscard]] bool peekTokenIs(TokenType t) const;
        bool expectPeek(TokenType t);

        Objective* parseObjDecl();
        VarDecl* parseVarDecl();
        std::vector<VarDecl*> parseVarDeclList();
        Constraint* parseStDecl();
        std::vector<Constraint*> parseStList();
        void parseEndStmt();

        Expression* parseExpression(int precedence = 0);
        Expression* parseIdentifier();
        Expression* parseNumber();
        Expression* parseGroupedExpr();
        Expression* parseFunctionCall(std::string funcName);

        [[nodiscard]] int tokenPrecedence(TokenType t) const;
    };

    enum class Precedence {
        LOWEST = 0,
        SUM = 10,   // +, -
        PRODUCT = 20,   // *, /
        POWER = 30,   // ^
        PREFIX = 40,   // -x, +x
    };


}

#endif
