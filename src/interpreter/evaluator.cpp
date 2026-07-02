/**
 * @file evaluator.cpp
 * @brief An implementation that actually implements constants and variables.
 * Objects evaluated through the Evaluator are then passed into the HS engine.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-10-05
 */

#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include <cmath>
#include <iostream>
#include <sstream>
#include <limits>
#include "evaluator.h"
#include "token.h"
#include "func.h"

namespace hsl {
    using Env = std::unordered_map<std::string, double>;
    using AliasTable = std::unordered_map<std::string, Expression*>;

    struct EvalContext {
        const AliasTable& constAliases;
        const AliasTable& funcAliases;
        std::unordered_map<std::string, double> aliasCache;
        std::vector<std::string> aliasStack;
        std::unordered_set<std::string> aliasInProgress;
    };

    static std::string buildAliasCycleMessage(const std::vector<std::string>& stack, const std::string& name) {
        std::ostringstream oss;
        oss << "Circular alias reference detected: ";

        bool started = false;
        for (const auto& n : stack) {
            if (n == name) {
                started = true;
            }
            if (started) {
                oss << n << " -> ";
            }
        }
        oss << name;
        return oss.str();
    }

    static double evalExpr(Expression* expr, Env& env, EvalContext& ctx);

    static double evalAlias(const std::string& name, Expression* expr, Env& env, EvalContext& ctx) {
        auto cached = ctx.aliasCache.find(name);
        if (cached != ctx.aliasCache.end()) {
            return cached->second;
        }
        if (ctx.aliasInProgress.find(name) != ctx.aliasInProgress.end()) {
            throw std::runtime_error(buildAliasCycleMessage(ctx.aliasStack, name));
        }

        ctx.aliasInProgress.insert(name);
        ctx.aliasStack.push_back(name);
        double value = evalExpr(expr, env, ctx);
        ctx.aliasStack.pop_back();
        ctx.aliasInProgress.erase(name);
        ctx.aliasCache[name] = value;
        return value;
    }

    /**
    * @brief Recursively evaluates an AST expression node to a double value.
    * @param expr The expression node to evaluate (e.g., Number, Ident, Binary).
    * @param env The environment mapping variable names to their current values.
    * @return The calculated result of the expression as a double.
    * @throws std::runtime_error If an undefined variable/function is accessed or an unsupported operator is encountered.
    */
    static double evalExpr(Expression* expr, Env& env, EvalContext& ctx) {
        if (auto num = dynamic_cast<NumberExpr*>(expr)) {
            return num->value;
        }
        else if (auto id = dynamic_cast<IdentExpr*>(expr)) {
            // Runtime variables and loop variables have the highest priority.
            if (auto envIt = env.find(id->name); envIt != env.end()) {
                return envIt->second;
            }

            // User-defined constants.
            if (auto constIt = ctx.constAliases.find(id->name); constIt != ctx.constAliases.end()) {
                return evalAlias(id->name, constIt->second, env, ctx);
            }

            // User-defined expression aliases.
            if (auto funcIt = ctx.funcAliases.find(id->name); funcIt != ctx.funcAliases.end()) {
                return evalAlias(id->name, funcIt->second, env, ctx);
            }

            // Built-in constants (pi, e, inf).
            const auto& K = hsl::builtinConstants();
            if (auto it = K.find(id->name); it != K.end()) {
                return it->second;
            }

            throw std::runtime_error("Undefined variable or alias: " + id->name);
        }
        else if (auto un = dynamic_cast<UnaryExpr*>(expr)) {
            double val = evalExpr(un->expr, env, ctx);
            switch (un->op) {
                case TokenType::MINUS: return -val;
                case TokenType::PLUS: return +val;
                default: throw std::runtime_error("Unsupported unary op");
            }
        } // If unray operator(for negative value) is defined:
        else if (auto bin = dynamic_cast<BinaryExpr*>(expr)) {
            double lhs = evalExpr(bin->left, env, ctx);
            double rhs = evalExpr(bin->right, env, ctx);
            switch (bin->op) {
                case TokenType::PLUS: return lhs + rhs;
                case TokenType::MINUS: return lhs - rhs;
                case TokenType::ASTERISK: return lhs * rhs;
                case TokenType::SLASH: return lhs / rhs;
                case TokenType::CARET: return std::pow(lhs, rhs); // Caret symbol is defined as power operation.
                default: throw std::runtime_error("Unsupported binary op");
            }
        }
        else if (auto call = dynamic_cast<FunctionCallExpr*>(expr)) {
            /* sum(sigma) and product(pi) in built-in function work unlike others.
             They require dynamic allocation saperately, so they need to define individually.
             */
            if (call->name == "sum" || call->name == "product") {
                if (call->args.size() != 4)
                    throw std::runtime_error(call->name + "() expects 4 arguments: (i, start, end, expr)");

                auto* idExpr = dynamic_cast<IdentExpr*>(call->args[0]);
                if (!idExpr)
                    throw std::runtime_error(call->name + "(): first argument must be an identifier");

                std::string varName = idExpr->name;
                double start = evalExpr(call->args[1], env, ctx);
                double end   = evalExpr(call->args[2], env, ctx);

                // Define initial value (0 for sum, 1 for product).
                double result = (call->name == "sum") ? 0.0 : 1.0;

                for (int i = static_cast<int>(start); i <= static_cast<int>(end); ++i) {
                    env[varName] = i;
                    double val = evalExpr(call->args[3], env, ctx);
                    if (call->name == "sum") result += val;
                    else result *= val;
                }

                env.erase(varName);
                return result;
            }

            // The other built-in function is defined binding of C++20's functions.
            const auto& F = hsl::builtinFunctions();
            auto it = F.find(call->name);
            if (it == F.end()) {
                throw std::runtime_error("Unknown function: " + call->name);
            }

            std::vector<double> argv;
            argv.reserve(call->args.size());
            for (auto* a : call->args) {
                argv.push_back(evalExpr(a, env, ctx));
            }
            return it->second(argv);
        }
        else if (auto idx = dynamic_cast<IndexExpr*>(expr)) {
            double index = evalExpr(idx->index, env, ctx);
            int i = static_cast<int>(index);
            std::string key = idx->name + "[" + std::to_string(i) + "]";

            auto it = env.find(key);
            if (it == env.end()) {
                throw std::runtime_error(
                        "Undefined variable access: '" + key +
                        "'.\nMake sure it is declared in [VAR] section (e.g., [VAR] "
                        + idx->name + "[" + std::to_string(i) + "], ... )");
            }
            /* If range-based variable did not defined correctly (ex: only defiend x[1], x[2] for sum(i, 1, 3, x[i])), throw error.
             Else if range-based variable is overly defined (ex: defiend x[1], x[2], x[3] for sum(i, 1, 2, x[i])), exceeded amounts are ignored.
             */

            return it->second;
        }


        throw std::runtime_error("Unknown expression node");
    }

    /**
    * @brief Evaluates whether a specific constraint is satisfied under the current environment.
    * @param c The constraint node containing the left/right expressions and a comparator.
    * @param env The environment containing current variable values.
    * @return true if the constraint is satisfied, false otherwise.
    * @note Floating-point comparisons (EQ, NEQ) use a tolerance of 1e-9 to account for precision errors.
    */
    static bool evalConstraint(Constraint* c, Env& env, EvalContext& ctx) {
        double left = evalExpr(c->left, env, ctx);
        double right = evalExpr(c->right, env, ctx);
        switch (c->comparator) {
            case TokenType::LEQ: return left <= right;
            case TokenType::GEQ: return left >= right;
            case TokenType::EQ:  return std::fabs(left - right) < 1e-9; // Use 1e-9 for adjust double data.
            case TokenType::NEQ: return std::fabs(left - right) >= 1e-9;
            case TokenType::LT:  return left < right;
            case TokenType::GT:  return left > right;
            default: throw std::runtime_error("Unsupported comparator");
        }
    }

    /**
    * @brief Transforms a parsed Program (AST) into an executable HSProblem structure.
    * @param program The root node of the Abstract Syntax Tree.
    * @return An HSProblem object containing variables, objective functions, and penalty logic.
    * @details This function handles the expansion of range-based variables (e.g., x[1..3])
    * and wraps the objective/constraints into lambda functions for the solver.
    */
    HSProblem buildHSProblem(Program* program) {
        HSProblem prob;
        AliasTable constAliases;
        AliasTable funcAliases;

        const auto& builtinConsts = hsl::builtinConstants();

        auto validateAliasName = [&](const std::string& name, const std::string& sectionName) {
            if (builtinConsts.find(name) != builtinConsts.end()) {
                throw std::runtime_error(sectionName + " cannot redefine built-in constant '" + name + "'");
            }
            if (constAliases.find(name) != constAliases.end() || funcAliases.find(name) != funcAliases.end()) {
                throw std::runtime_error("Duplicate alias name: '" + name + "'");
            }
        };

        for (auto* c : program->consts) {
            validateAliasName(c->name, "[CONST]");
            constAliases[c->name] = c->value;
        }
        for (auto* f : program->funcs) {
            validateAliasName(f->name, "[FUNC]");
            funcAliases[f->name] = f->value;
        }

        // Validate alias and variable conflicts in practical scope.
        for (auto* v : program->vars) {
            if (constAliases.find(v->name) != constAliases.end() || funcAliases.find(v->name) != funcAliases.end()) {
                throw std::runtime_error("Name conflict between [VAR] and alias: '" + v->name + "'");
            }
        }

        for (auto* v : program->vars) {
            Env boundEnv;
            EvalContext boundCtx{constAliases, funcAliases};
            double lower = evalExpr(v->lower, boundEnv, boundCtx);
            double upper = evalExpr(v->upper, boundEnv, boundCtx);

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
                } // For those defined with range, define them one by one in the form x[1], x[2].
            } else {
                // standalone variable
                Variable var{name, {lower, upper}, v->isInt};
                prob.variables.push_back(var);
            }
        }
        // Variable definition and scope assignment are actually performed in here.

        Objective* obj = program->obj;
        prob.maximize = obj->isMax;
        prob.objective = [=, constAliases = constAliases, funcAliases = funcAliases]
                (const std::vector<double>& values) {
            Env env;
            for (size_t i = 0; i < prob.variables.size(); i++) {
                env[prob.variables[i].name] = values[i];
            }
            EvalContext ctx{constAliases, funcAliases};
            return evalExpr(obj->expr, env, ctx);
        };
        // Interpret Objective function.

        auto constraints = program->constraints;
        prob.penalty = [=, constAliases = constAliases, funcAliases = funcAliases]
                (const std::vector<double>& values) {
            Env env;
            for (size_t i = 0; i < prob.variables.size(); i++) {
                env[prob.variables[i].name] = values[i];
            }
            EvalContext ctx{constAliases, funcAliases};
            for (auto* c : constraints) {
                if (!evalConstraint(c, env, ctx)) {
                    // Hard constaint: If constration violation detected, impose a penalty as infinity
                    return std::numeric_limits<double>::infinity();
                }
            }
            return 0.0;
        };
        // Interpret the constraints and then check whether these conditions are actually satisfied

        return prob;
    }
}
