#ifndef HSL_EVALUATOR
#define HSL_EVALUATOR

#include <string>
#include <vector>
#include <functional>
#include "ast.h"

namespace hsl{
    struct Variable {
        std::string name;
        std::pair<double, double> range;
        bool isInt;
    };

    struct HSProblem {
        std::vector<Variable> variables;
        std::function<double(const std::vector<double>&)> objective;
        std::function<double(const std::vector<double>&)> penalty;
        bool maximize;
    };

    HSProblem buildHSProblem(Program* program);
}

#endif
