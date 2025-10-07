#include <unordered_map>
#include <stdexcept>
#include <cmath>
#include <iostream>
#include "evaluator.h"
#include "token.h"
#include "func.h"

namespace hsl {
    using Env = std::unordered_map<std::string, double>;

    double evalExpr(Expression* expr, Env& env) {
        if (auto num = dynamic_cast<NumberExpr*>(expr)) {
            return num->value;
        }
        else if (auto id = dynamic_cast<IdentExpr*>(expr)) {
            // 내장 상수의 경우(ex: e, pi)
            const auto& K = hsl::builtinConstants();
            if (auto it = K.find(id->name); it != K.end()) {
                return it->second;
            }
            // 환경 변수
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
        else if (auto call = dynamic_cast<FunctionCallExpr*>(expr)) {
            // 내장 함수 중 sum(sigma)와 product(pi)는 특수하게 작동 -> 따로 동적 할당 등이 필요하기 때문에 별도로 정의.
            if (call->name == "sum" || call->name == "product") {
                if (call->args.size() != 4)
                    throw std::runtime_error(call->name + "() expects 4 arguments: (i, start, end, expr)");

                auto* idExpr = dynamic_cast<IdentExpr*>(call->args[0]);
                if (!idExpr)
                    throw std::runtime_error(call->name + "(): first argument must be an identifier");

                std::string varName = idExpr->name;
                double start = evalExpr(call->args[1], env);
                double end   = evalExpr(call->args[2], env);

                // 초기값 설정
                double result = (call->name == "sum") ? 0.0 : 1.0;

                for (int i = static_cast<int>(start); i <= static_cast<int>(end); ++i) {
                    env[varName] = i;
                    double val = evalExpr(call->args[3], env);
                    if (call->name == "sum") result += val;
                    else result *= val;
                }

                env.erase(varName);
                return result;
            }

            //나머지 내장함수는 별도로 정의된 built-in으로 해결.
            const auto& F = hsl::builtinFunctions();
            auto it = F.find(call->name);
            if (it == F.end()) {
                throw std::runtime_error("Unknown function: " + call->name);
            }

            std::vector<double> argv;
            argv.reserve(call->args.size());
            for (auto* a : call->args) {
                argv.push_back(evalExpr(a, env));
            }
            return it->second(argv);
        }
        else if (auto idx = dynamic_cast<IndexExpr*>(expr)) {
            double index = evalExpr(idx->index, env);
            int i = static_cast<int>(index);
            std::string key = idx->name + "[" + std::to_string(i) + "]";

            auto it = env.find(key);
            if (it == env.end()) {
                throw std::runtime_error(
                        "Undefined variable access: '" + key +
                        "'.\nMake sure it is declared in [VAR] section (e.g., [VAR] "
                        + idx->name + "[" + std::to_string(i) + "], ... )");
            } // 만약 range와 관련된 변수들이 제대로 정의가 되지 않았다면(ex: sum(i, 1, 3, x[i])에서 x[1], x[2]만 정의한 경우, 이 경우는 error.

            return it->second;
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

            std::string name = v->name;
            size_t lb = name.find('[');
            size_t dots = name.find("..", lb);
            size_t rb = name.find(']', dots);

            if (dots != std::string::npos && rb != std::string::npos) {
                // ex: x[1..3]
                std::string base = name.substr(0, lb);
                int start = std::stoi(name.substr(lb + 1, dots - (lb + 1)));
                int end   = std::stoi(name.substr(dots + 2, rb - (dots + 2)));

                for (int i = start; i <= end; ++i) {
                    std::string expanded = base + "[" + std::to_string(i) + "]";
                    Variable var{expanded, {lower, upper}, v->isInt};
                    prob.variables.push_back(var);
                } // range로 정의된 건 x[1], x[2]꼴로 하나씩 정의
            } else {
                // 단일 변수
                Variable var{name, {lower, upper}, v->isInt};
                prob.variables.push_back(var);
            }
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
