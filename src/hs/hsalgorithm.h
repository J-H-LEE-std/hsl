/**
 * @file hsalgorithm.h
 * @brief Header define for essential HS algorithm.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-10-05
 */

#ifndef HSL_HSALGORITHM_
#define HSL_HSALGORITHM_

#include <vector>
#include <random>
#include <ostream>
#include "params.h"
#include "../interpreter/evaluator.h"

namespace hsl {
    class ExperimentLogger; // Logger for HS-L execution.

    /* Define Harmony as new data type.
     This structure includes optimal and arguments for logging.
     Operators are overrided for ease of sorting.
    */
    struct Harmony {
        std::vector<double> vars;
        double value;
        bool operator<(const Harmony& other) const { return value < other.value; }
        bool operator>(const Harmony& other) const { return value > other.value; }
        bool operator==(const Harmony& other) const { return vars == other.vars && value == other.value; }
    };

    // Result of HS include arguments, value, and times to finish optimize.
    struct HSResult {
        std::vector<double> vars;
        double value = 0.0;
        double cpu_time = 0.0;
        double log_io_time = 0.0;
    };

    class HarmonySearch {
    public:
        HarmonySearch(const HSProblem& prob, const HSParams& params,
                      unsigned int seed = std::random_device{}(),
                      ExperimentLogger* logger = nullptr,
                      bool suppressProgress = false);
        Harmony optimize();
    private:
        const HSProblem& problem;
        HSParams params;
        std::mt19937 rng;
        std::vector<Harmony> HM;
        ExperimentLogger* logger;
        bool suppressProgress;
        Harmony generateFeasibleSolution();
        double evaluate(const std::vector<double>& solution);
        void insertHarmony(const Harmony& h);
    };
}
#endif
