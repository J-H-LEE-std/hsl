#ifndef HSL_AST_
#define HSL_AST_

#include <string>
#include <vector>
#include "token.h"

namespace hsl{
    struct Program {
        struct Objective* obj;
        std::vector<struct VarDecl*> vars;
        std::vector<struct Constraint*> constraints;
    }; //프로그램 전체에 대한 구조를 정의. 최종 Objective는 1개, 변수와 Constraint는 언어 명세에 의하면 여러 개 나올 수 있으니 vector로 관리.

    struct Objective {
        bool isMax; // true = max, false = min
        struct Expression* expr;
    }; //목적함수.

    struct VarDecl {
        std::string name;
        Expression* lower;
        Expression* upper;
        bool isInt; // true=int, false=any(double)
    }; //변수 정의.

    struct Constraint {
        struct Expression* left;
        TokenType comparator;         // LEQ, GEQ, EQ, NEQ, LT, GT
        struct Expression* right;
    }; //제약조건 정의.

    struct Expression {
        virtual ~Expression() = default;
    };

    struct NumberExpr : Expression {
        double value;
        bool isInt;
        NumberExpr(double v, bool isInt_) : value(v), isInt(isInt_) {}
    };

    struct IdentExpr : Expression {
        std::string name;
        explicit IdentExpr(std::string n) : name(std::move(n)) {}
    };

    struct UnaryExpr : Expression {
        TokenType op;
        Expression* expr;
        UnaryExpr(TokenType op_, Expression* e) : op(op_), expr(e) {}
    };

    struct BinaryExpr : Expression {
        TokenType op;
        Expression* left;
        Expression* right;
        BinaryExpr(TokenType op_, Expression* l, Expression* r)
                : op(op_), left(l), right(r) {}
    };

    struct FunctionCallExpr : Expression {
        std::string name;
        std::vector<Expression*> args;
        FunctionCallExpr(std::string n, std::vector<Expression*> a)
                : name(std::move(n)), args(std::move(a)) {}
    }; // 함수 처리에 대한 정의.

}

#endif
