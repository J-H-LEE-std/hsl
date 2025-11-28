#ifndef HSL_HSALGORITHM_
#define HSL_HSALGORITHM_

#include <vector>
#include <random>
#include <ostream>
#include "params.h"
#include "../interpreter/evaluator.h"

namespace hsl {

    struct Harmony {
        std::vector<double> vars;
        double value;
        bool operator<(const Harmony& other) const { return value < other.value; }
        bool operator>(const Harmony& other) const { return value > other.value; }
        bool operator==(const Harmony& other) const { return vars == other.vars && value == other.value; }
    };

    struct HSResult {
        std::vector<double> vars;
        double value = 0.0;
        double cpu_time = 0.0;
    };

    class HarmonySearch {
    public:
        HarmonySearch(const HSProblem& prob, const HSParams& params,
                      unsigned int seed = std::random_device{}());
        Harmony optimize();
    private:
        const HSProblem& problem;
        HSParams params;
        std::mt19937 rng;
        std::vector<Harmony> HM;
        Harmony generateFeasibleSolution();
        double evaluate(const std::vector<double>& solution);
        void insertHarmony(const Harmony& h);
    };

    HSResult runHarmonySearch(const HSProblem& prob, const HSParams& params,
                              unsigned int seed, std::ostream& log);
}
#endif
