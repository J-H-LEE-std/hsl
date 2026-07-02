/**
 * @file parser.h
 * @brief Header file for parser class defination.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-10-05
 */

#ifndef HSL_PARSER_
#define HSL_PARSER_

#include <optional>
#include "lexer.h"
#include "token.h"
#include "ast.h"

namespace hsl {
    class Parser {
    public:
        explicit Parser(Lexer& lexer);

        Program* parseProgram();

        // For access error log
        [[nodiscard]] const std::vector<std::string>& getErrors() const { return errors; }

    private:
        Lexer& lexer;
        Token curToken;
        Token peekToken;
        std::vector<std::string> errors;
        [[nodiscard]] bool curTokenIs(TokenType t) const;
        [[nodiscard]] bool peekTokenIs(TokenType t) const;
        bool expectPeek(TokenType t);
        [[nodiscard]] int tokenPrecedence(TokenType t) const;

        void nextToken();
        Objective* parseObjDecl();
        VarDecl* parseVarDecl();
        std::vector<VarDecl*> parseVarDeclList();
        ConstDecl* parseConstDecl();
        std::vector<ConstDecl*> parseConstDeclList();
        FuncDecl* parseFuncDecl();
        std::vector<FuncDecl*> parseFuncDeclList();
        Constraint* parseStDecl();
        std::vector<Constraint*> parseStList();
        void parseEndStmt();
        bool isValidAliasLHS(Expression* expr) const;
        std::optional<std::string> extractAliasName(Expression* expr) const;

        Expression* parseExpression(int precedence = 0);
        Expression* parseIdentifier();
        Expression* parseNumber();
        Expression* parseGroupedExpr();
        Expression* parseFunctionCall(std::string funcName);
    };

    // Define enum class for precedence climbing procedure
    enum class Precedence {
        LOWEST = 0,
        SUM = 10,   // +, -
        PRODUCT = 20,   // *, /
        POWER = 30,   // ^
        PREFIX = 40,   // -x, +x
    };
}

#endif
