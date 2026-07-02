/**
 * @file ast.h
 * @brief Header file defining the components of the AST.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-10-05
 */

#ifndef HSL_AST_
#define HSL_AST_

#include <string>
#include <vector>
#include "token.h"

namespace hsl{
     /* Define the structure for the entire program.
     There is one final Objective, and since multiple variables and Constraints can appear according to the language specification, manage them as vectors.
    */
    struct Program {
        struct Objective* obj;
        std::vector<struct VarDecl*> vars;
        std::vector<struct ConstDecl*> consts;
        std::vector<struct FuncDecl*> funcs;
        std::vector<struct Constraint*> constraints;
    };
    // TODO: Extend Objective structure to support Multi-object Oprimization.

    // Define Object function.
    struct Objective {
        bool isMax; // true = max, false = min
        struct Expression* expr;
    };

    // Define Variable and its range.
    struct VarDecl {
        std::string name;
        Expression* lower;
        Expression* upper;
        bool isInt; // true=int, false=any(double)
    };

    struct ConstDecl {
        std::string name;
        Expression* value;
    };

    struct FuncDecl {
        std::string name;
        Expression* value;
    };

    // Define Constraint.
    struct Constraint {
        struct Expression* left;
        TokenType comparator; // LEQ, GEQ, EQ, NEQ, LT, GT
        struct Expression* right;
    };

    struct Expression {
        virtual ~Expression() = default;
    };

    // Define number expression. Integer and real value are expressed as same structure.
    struct NumberExpr : Expression {
        double value;
        bool isInt; // true=int, false=double
        NumberExpr(double v, bool isInt_) : value(v), isInt(isInt_) {}
    };

    // Define identfier(name of variable, function, ...).
    struct IdentExpr : Expression {
        std::string name;
        explicit IdentExpr(std::string n) : name(std::move(n)) {}
    };

    // Define unray expression for negative value.
    struct UnaryExpr : Expression {
        TokenType op;
        Expression* expr;
        UnaryExpr(TokenType op_, Expression* e) : op(op_), expr(e) {}
    };

    /* Define structure of operators and operands.
     HS-L only treat only binary operators except unray expression.
     For unray expression, refer UnaryExpr.
    */
    struct BinaryExpr : Expression {
        TokenType op;
        Expression* left;
        Expression* right;
        BinaryExpr(TokenType op_, Expression* l, Expression* r)
                : op(op_), left(l), right(r) {}
    };

    // Define function call.
    struct FunctionCallExpr : Expression {
        std::string name;
        std::vector<Expression*> args;
        FunctionCallExpr(std::string n, std::vector<Expression*> a)
                : name(std::move(n)), args(std::move(a)) {}
    };

    // Define Index-base variable call.
    struct IndexExpr : Expression {
        std::string name;
        Expression* index;
        IndexExpr(std::string n, Expression* idx)
                : name(std::move(n)), index(idx) {}
    };
}

#endif
