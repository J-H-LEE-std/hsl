#include <unordered_map>
#include <stdexcept>
#include <cmath>
#include <iostream>
#include "evaluator.h"
#include "token.h"

namespace hsl {
    using Env = std::unordered_map<std::string, double>;

    double evalExpr(Expression* expr, Env& env) {
        if (auto num = dynamic_cast<NumberExpr*>(expr)) {
            return num->value;
        }
        else if (auto id = dynamic_cast<IdentExpr*>(expr)) {
            if (env.find(id->name) == env.end())
                throw std::runtime_error("Undefined variable: " + id->name);
            return env[id->name];
        }
        else if (auto un = dynamic_cast<UnaryExpr*>(expr)) {
            double val = evalExpr(un->expr, env);
            switch (un->op) {
                case TokenType::MINUS: return -val;
                case TokenType::PLUS: return +val;
                default: throw std::runtime_error("Unsupported unary op");
            }
        } // 음수 양수.
        else if (auto bin = dynamic_cast<BinaryExpr*>(expr)) {
            double lhs = evalExpr(bin->left, env);
            double rhs = evalExpr(bin->right, env);
            switch (bin->op) {
                case TokenType::PLUS: return lhs + rhs;
                case TokenType::MINUS: return lhs - rhs;
                case TokenType::ASTERISK: return lhs * rhs;
                case TokenType::SLASH: return lhs / rhs;
                case TokenType::CARET: return std::pow(lhs, rhs); // 여기서는 볼 수 있듯 ^를 pow로 사용
                default: throw std::runtime_error("Unsupported binary op");
            }
        }
        throw std::runtime_error("Unknown expression node");
    }

    // 제약조건 평가 (true=만족, false=위반)
    bool evalConstraint(Constraint* c, Env& env) {
        double left = evalExpr(c->left, env);
        double right = evalExpr(c->right, env);
        switch (c->comparator) {
            case TokenType::LEQ: return left <= right;
            case TokenType::GEQ: return left >= right;
            case TokenType::EQ:  return std::fabs(left - right) < 1e-9; // 배정밀도 오차 보정
            case TokenType::NEQ: return std::fabs(left - right) >= 1e-9;
            case TokenType::LT:  return left < right;
            case TokenType::GT:  return left > right;
            default: throw std::runtime_error("Unsupported comparator");
        }
    }

    HSProblem buildHSProblem(Program* program) {
        HSProblem prob;

        for (auto* v : program->vars) {
            Env env;
            double lower = evalExpr(v->lower, env);
            double upper = evalExpr(v->upper, env);

            Variable var{v->name, {lower, upper}, v->isInt};
            prob.variables.push_back(var);
        } // 변수 정의 및 범위 할당이 실제로 이루어짐

        Objective* obj = program->obj;
        prob.maximize = obj->isMax;
        prob.objective = [=](const std::vector<double>& values) {
            Env env;
            for (size_t i = 0; i < prob.variables.size(); i++) {
                env[prob.variables[i].name] = values[i];
            }
            return evalExpr(obj->expr, env);
        }; // 목적 함수 해석

        auto constraints = program->constraints;
        prob.penalty = [=](const std::vector<double>& values) {
            Env env;
            for (size_t i = 0; i < prob.variables.size(); i++) {
                env[prob.variables[i].name] = values[i];
            }
            for (auto* c : constraints) {
                if (!evalConstraint(c, env)) {
                    // 제약조건 위반이 걸리면 패널티를 infinity로 줘서 무효화
                    return std::numeric_limits<double>::infinity();
                }
            }
            return 0.0; // 제약 모두 만족
        }; // 제약 조건들을 해석 후 실제로 이 조건들을 만족하는지 검사할 수 있게 해석


        return prob;
    }

}
