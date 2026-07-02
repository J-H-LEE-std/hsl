/**
 * @file lexer.h
 * @brief HS-L Lexer implementation.
 * Parser parse all token and make a base for AST.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-10-05
 */

#include <iostream>
#include "parser.h"

namespace hsl {
    /**
    * @brief Initializes the Parser and primes the first two tokens.
    * @param l Reference to the Lexer to be used for tokenization.
    */
    Parser::Parser(Lexer& l) : lexer(l) {
        nextToken();
        nextToken();
    }

    /**
    * @brief Advances the current and lookahead tokens.
    */
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

    /**
    * @brief Top-level parsing function that builds the complete Program AST.
    * @return A Program object containing all top-level sections.
    * @details Supports [OBJ], [VAR], [CONST], [FUNC], [ST], [END].
    */
    Program* Parser::parseProgram() {
        auto* program = new Program();
        program->obj = nullptr;

        while (!curTokenIs(TokenType::END) && !curTokenIs(TokenType::END_OF_FILE)) {
            switch (curToken.type) {
                case TokenType::OBJ: {
                    if (program->obj != nullptr) {
                        errors.emplace_back("Duplicate [OBJ] declaration at line " + std::to_string(curToken.line));
                    } else {
                        program->obj = parseObjDecl();
                    }
                    break;
                }
                case TokenType::VAR: {
                    auto* varDecl = parseVarDecl();
                    if (varDecl) {
                        program->vars.emplace_back(varDecl);
                    }
                    break;
                }
                case TokenType::DEFCONST: {
                    auto* constDecl = parseConstDecl();
                    if (constDecl) {
                        program->consts.emplace_back(constDecl);
                    }
                    break;
                }
                case TokenType::DEFFUNC: {
                    auto* funcDecl = parseFuncDecl();
                    if (funcDecl) {
                        program->funcs.emplace_back(funcDecl);
                    }
                    break;
                }
                case TokenType::ST: {
                    auto* stDecl = parseStDecl();
                    if (stDecl) {
                        program->constraints.emplace_back(stDecl);
                    }
                    break;
                }
                default:
                    errors.emplace_back("Unexpected top-level token '" + curToken.literal +
                                        "' at line " + std::to_string(curToken.line));
                    break;
            }

            if (curTokenIs(TokenType::END) || curTokenIs(TokenType::END_OF_FILE)) {
                break;
            }
            nextToken();
        }

        if (program->obj == nullptr) {
            errors.emplace_back("Missing required [OBJ] declaration");
        }
        parseEndStmt();

        return program;
    }

    /**
    * @brief Parses the [OBJ] section to define the optimization target.
    * @return An Objective pointer containing the goal (max/min) and the expression.
    * @note Syntax: obj_decl ::= "[OBJ]" ("max" | "min") expression ;
    */
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
    }

    /**
    * @brief Parses a single [VAR] declaration, including range-based array syntax.
    * @return A VarDecl pointer with bounds and type information.
    * @details Handles standalone variables (x) and range-expanded variables (x[1..3]).
    * @note Syntax: var_decl ::= "[VAR]" identifier [ index_of_range ] "," lower "," upper "," ("int" | "any") ;
    */
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

        /* Array-type variables such as x[1] are also supported.
         However, they must have already been defined in OBJ such as sum.
         Declaring arrays statically here is not allowed.
         */
        if (peekTokenIs(TokenType::LBRACKET)) {
            nextToken();
            nextToken();

            if (curTokenIs(TokenType::NUMBER_FLOAT)) {
                errors.emplace_back("Float Range is not supported.");
                return nullptr;
            } else if (!curTokenIs(TokenType::NUMBER_INT)){
                errors.emplace_back("Expected numeric index after '[' in variable name");
                return nullptr;
            } // NUMBER_FLOAT token in range is not supported.

            std::string startIdx = curToken.literal;
            std::string endIdx = "";

            // If dynamically defined formulas such as x[1], x[2], x[3], ... are all the same, define them as a range like [1..3].
            if (peekTokenIs(TokenType::RANGE)) {
                nextToken();
                nextToken();
                if (curTokenIs(TokenType::NUMBER_FLOAT)) {
                    errors.emplace_back("Float Range is not supported.");
                    return nullptr;
                } else if (!curTokenIs(TokenType::NUMBER_INT)){
                    errors.emplace_back("Expected number after '..' in range declaration");
                    return nullptr;
                }
                endIdx = curToken.literal;
            }

            if (!peekTokenIs(TokenType::RBRACKET)) {
                errors.emplace_back("Expected ']' after variable index");
                return nullptr;
            }
            nextToken();

            if (!endIdx.empty()) {
                name = name + "[" + startIdx + ".." + endIdx + "]";
            } else {
                name = name + "[" + startIdx + "]";
            }
        }

        if (!expectPeek(TokenType::COMMA)) return nullptr;

        /* Reading min range.
        It changed to simply parse the expression to support various number formats
        */
        nextToken();
        Expression *lowerExpr = parseExpression();
        if (!lowerExpr) {
            errors.emplace_back("Invalid expression as lower bound");
            return nullptr;
        }

        if (!expectPeek(TokenType::COMMA)) return nullptr;

        /* Reading max range.
        It changed to simply parse the expression to support various number formats
        */
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
    }

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

    ConstDecl* Parser::parseConstDecl() {
        if (!curTokenIs(TokenType::DEFCONST)) {
            errors.emplace_back("Expected [CONST]");
            return nullptr;
        }

        nextToken();
        Expression* lhs = parseExpression();
        if (!lhs) {
            errors.emplace_back("Invalid [CONST] left-hand side expression");
            return nullptr;
        }

        if (!expectPeek(TokenType::EQ)) {
            errors.emplace_back("Expected '=' after [CONST] left-hand side");
            return nullptr;
        }
        if (curToken.literal != "=") {
            errors.emplace_back("Expected '=' after [CONST] left-hand side");
            return nullptr;
        }

        nextToken();
        Expression* rhs = parseExpression();
        if (!rhs) {
            errors.emplace_back("Invalid [CONST] right-hand side expression");
            return nullptr;
        }

        if (!isValidAliasLHS(lhs)) {
            errors.emplace_back("[CONST] left-hand side must be a simple identifier");
            return nullptr;
        }

        auto name = extractAliasName(lhs);
        if (!name.has_value()) {
            errors.emplace_back("[CONST] left-hand side must be a simple identifier");
            return nullptr;
        }
        return new ConstDecl{*name, rhs};
    }

    std::vector<ConstDecl*> Parser::parseConstDeclList() {
        std::vector<ConstDecl*> consts;

        while (peekTokenIs(TokenType::DEFCONST)) {
            nextToken();
            auto* decl = parseConstDecl();
            if (decl) {
                consts.emplace_back(decl);
            } else {
                break;
            }
        }
        return consts;
    }

    FuncDecl* Parser::parseFuncDecl() {
        if (!curTokenIs(TokenType::DEFFUNC)) {
            errors.emplace_back("Expected [FUNC]");
            return nullptr;
        }

        nextToken();
        Expression* lhs = parseExpression();
        if (!lhs) {
            errors.emplace_back("Invalid [FUNC] left-hand side expression");
            return nullptr;
        }

        if (!expectPeek(TokenType::EQ)) {
            errors.emplace_back("Expected '=' after [FUNC] left-hand side");
            return nullptr;
        }
        if (curToken.literal != "=") {
            errors.emplace_back("Expected '=' after [FUNC] left-hand side");
            return nullptr;
        }

        nextToken();
        Expression* rhs = parseExpression();
        if (!rhs) {
            errors.emplace_back("Invalid [FUNC] right-hand side expression");
            return nullptr;
        }

        if (!isValidAliasLHS(lhs)) {
            errors.emplace_back("[FUNC] left-hand side must be a simple identifier");
            return nullptr;
        }

        auto name = extractAliasName(lhs);
        if (!name.has_value()) {
            errors.emplace_back("[FUNC] left-hand side must be a simple identifier");
            return nullptr;
        }
        return new FuncDecl{*name, rhs};
    }

    std::vector<FuncDecl*> Parser::parseFuncDeclList() {
        std::vector<FuncDecl*> funcs;

        while (peekTokenIs(TokenType::DEFFUNC)) {
            nextToken();
            auto* decl = parseFuncDecl();
            if (decl) {
                funcs.emplace_back(decl);
            } else {
                break;
            }
        }
        return funcs;
    }

    /**
    * @brief Parses a single [ST] constraint declaration.
    * @return A Constraint pointer containing the LHS expression, comparator, and RHS expression.
    * @note Syntax: st_decl ::= "[ST]" expression comparator expression ;
    */
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
    }

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
        if (!curTokenIs(TokenType::END)) {
            errors.emplace_back("Expected [END] at line " + std::to_string(curToken.line));
            return;
        }
        nextToken();
        if (!curTokenIs(TokenType::END_OF_FILE)) {
            errors.emplace_back("Expected none of code after [END] at line " + std::to_string(curToken.line));
        }
    }

    bool Parser::isValidAliasLHS(Expression* expr) const {
        return dynamic_cast<IdentExpr*>(expr) != nullptr;
    }

    std::optional<std::string> Parser::extractAliasName(Expression* expr) const {
        auto* ident = dynamic_cast<IdentExpr*>(expr);
        if (!ident) {
            return std::nullopt;
        }
        return ident->name;
    }

    /**
    * @brief Entry point for the Pratt Parser to handle expressions with operator precedence.
    * @param precedence The current operator precedence level.
    * @return An Expression pointer (Number, Ident, Binary, etc.).
    * @details Handles unary prefixes, grouped expressions, and infix operations using a precedence table.
    */
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
            nextToken(); // operator

            TokenType op = curToken.type;
            int prec = tokenPrecedence(op);
            int nextPrec = (op == TokenType::CARET) ? prec - 1 : prec;

            nextToken();
            Expression* right = parseExpression(nextPrec);

            left = new BinaryExpr{op, left, right};
        }

        return left;
    }

    /**
    * @brief Parses an identifier, branching to function calls or index accesses if needed.
    * @return An IdentExpr, FunctionCallExpr, or IndexExpr pointer.
    */
    Expression* Parser::parseIdentifier() {
        std::string name = curToken.literal;

        // function :== IDENT "(" expression ")"
        if (peekTokenIs(TokenType::LPAREN)) {
            expectPeek(TokenType::LPAREN); // move to '('
            return parseFunctionCall(name);
        }

        // index call - index =
        if (peekTokenIs(TokenType::LBRACKET)) {
            expectPeek(TokenType::LBRACKET);
            nextToken(); // Index start
            Expression* indexExpr = parseExpression();
            if (!expectPeek(TokenType::RBRACKET)) {
                errors.emplace_back("Expected ']' after index expression");
                return nullptr;
            }
            return new IndexExpr{name, indexExpr};
        }

        return new IdentExpr{name};
    }

    /**
    * @brief Parses numeric literals into NumberExpr nodes.
    */
    Expression* Parser::parseNumber() {
        bool isInt = (curToken.type == TokenType::NUMBER_INT);
        return new NumberExpr{std::stod(curToken.literal), isInt};
    }

    /**
    * @brief Parses expressions wrapped in parentheses.
    */
    Expression* Parser::parseGroupedExpr() {
        nextToken(); // Call next token after consume '('
        Expression* expr = parseExpression(0);
        if (!expectPeek(TokenType::RPAREN)) {
            errors.emplace_back("Expected ')' after expression");
            return nullptr;
        }
        return expr;
    }

    /**
    * @brief Returns the mathematical precedence level for a given token type.
    */
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

    /**
    * @brief Parses function calls with support for multiple comma-separated arguments.
    * @param funcName The name of the function being called.
    * @return A FunctionCallExpr pointer.
    */
    Expression* Parser::parseFunctionCall(std::string funcName) {
        std::vector<Expression*> args;

        // If the function has no input value, call ')' immediately.
        if (peekTokenIs(TokenType::RPAREN)) {
            nextToken();
            return new FunctionCallExpr{std::move(funcName), std::move(args)};
        }

        // If one input value:
        nextToken();
        args.push_back(parseExpression());

        // If there are two or more input values, separate them with ',' and continue calling the argument values thereafter.
        while (peekTokenIs(TokenType::COMMA)) {
            nextToken();
            nextToken();
            args.push_back(parseExpression());
        }

        // Error when no ')'.
        if (!expectPeek(TokenType::RPAREN)) {
            errors.emplace_back("Expected ')' after function arguments");
            return nullptr;
        }

        return new FunctionCallExpr{std::move(funcName), std::move(args)};
    }
}
