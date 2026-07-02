/**
 * @file hsalgorithm.cpp
 * @brief Header of HS engine caller and runner for HS-L(interpreter + optimization engine).
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-10-05
 */

#ifndef HSL_RUNNER_
#define HSL_RUNNER_

#include <string>
#include <vector>
#include <chrono>
#include "../interpreter/ast.h"
#include "../interpreter/evaluator.h"
#include "hsalgorithm.h"
#include "params.h"
#include "../log/ExperimentLogger.h"

namespace hsl {

    // 1. If AST exist: evaluator → run HS
    Harmony runHarmonySearch(Program* program, const HSParams& params, unsigned int seed);

    // 2. If HSProblem exsit: run HS directly
    Harmony runHarmonySearch(const HSProblem& prob, const HSParams& params, unsigned int seed);

    HSResult runHarmonySearchWithLogger(const HSProblem& prob,
                          const HSParams& params,
                          unsigned int seed,
                          ExperimentLogger* logger);


    /* 3. When onluy .hs file path exsit: load file → Lexer → Parser → evaluator → run HS
    Passing the parseErrors pointer fills in the parsing error message.
    */
    HSResult runHarmonySearchFromFile(const std::string& hsFilePath,
                                     const HSParams& params,
                                     unsigned int seed = std::random_device{}(),
                                     std::vector<std::string>* parseErrors = nullptr,
                                     ExperimentLogger* logger = nullptr);
}

#endif
