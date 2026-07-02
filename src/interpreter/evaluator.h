/**
 * @file evaluator.h
 * @brief Header file which define HSProblem for HS engine.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-10-05
 */


#ifndef HSL_EVALUATOR
#define HSL_EVALUATOR

#include <string>
#include <vector>
#include <functional>
#include "ast.h"

namespace hsl{
    // Variables are stored with its name and range.
    struct Variable {
        std::string name;
        std::pair<double, double> range;
        bool isInt;
    };

    /* Define structure of the HSProblem.
    HSproblem contains the information of the problem which HS have to solve.
    HS engine is executed with provided problem and additional parameter information.
    */
    struct HSProblem {
        std::vector<Variable> variables;
        std::function<double(const std::vector<double>&)> objective;
        std::function<double(const std::vector<double>&)> penalty;
        bool maximize;
    };

    HSProblem buildHSProblem(Program* program);
}

#endif
