#ifndef HSL_HSALGORITHM_
#define HSL_HSALGORITHM_

#include <vector>
#include <random>
#include "params.h"
#include "../interpreter/evaluator.h"


namespace hsl{
    struct Harmony {
        std::vector<double> vars;
        double value;

        bool operator<(const Harmony& other) const {
            return value < other.value;
        }
        bool operator>(const Harmony& other) const {
            return value > other.value;
        }
        bool operator==(const Harmony& other) const {
            return vars == other.vars && value == other.value;
        }
    }; // Harmony 자료형을 만들어서 최적해 정보와 그 fitting 값을 동시에 저장.

    class HarmonySearch {
    public:
        HarmonySearch(const HSProblem& prob, const HSParams& params, unsigned int seed = std::random_device{}());

        Harmony optimize();  // 최적 해 전체 반환

    private:
        const HSProblem& problem;
        HSParams params;

        std::mt19937 rng;
        std::vector<Harmony> HM;  // Harmony Memory (값+변수 통합)

        Harmony generateFeasibleSolution();   // 제약을 만족하는 해 생성
        double evaluate(const std::vector<double>& solution);
        void insertHarmony(const Harmony& h); // HM 업데이트
    };

    Harmony runHarmonySearch(const HSProblem& prob, const HSParams& params);
}

#endif
