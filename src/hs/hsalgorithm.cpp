#include <iomanip>
#include <algorithm>
#include <limits>
#include <iostream>
#include <fstream>
#include "hsalgorithm.h"

namespace hsl {

    HarmonySearch::HarmonySearch(const HSProblem& prob, const HSParams& params, unsigned int seed)
            : problem(prob), params(params) {
        rng.seed(seed);
    }

    // 해를 평가
    double HarmonySearch::evaluate(const std::vector<double>& solution) {
        double obj = problem.objective(solution);
        double pen = problem.penalty(solution);
        if (std::isinf(pen)) {
            return problem.maximize ? std::numeric_limits<double>::lowest() : std::numeric_limits<double>::infinity(); // 무효 해 처리
        }
        return problem.maximize ? obj : -obj; // 최소화는 부호 반전
    }

    // 제약을 만족하는 해 생성
    Harmony HarmonySearch::generateFeasibleSolution() {
        std::uniform_real_distribution<double> dist01(0.0, 1.0);

        while (true) {
            std::vector<double> vars(problem.variables.size());

            for (size_t i = 0; i < problem.variables.size(); i++) {
                const auto& var = problem.variables[i];
                if (var.isInt) {
                    std::uniform_int_distribution<int> idist(
                            static_cast<int>(var.range.first),
                            static_cast<int>(var.range.second));
                    vars[i] = idist(rng);
                } else {
                    std::uniform_real_distribution<double> rdist(
                            var.range.first, var.range.second);
                    vars[i] = rdist(rng);
                }
            }

            // 제약 조건 확인
            if (problem.penalty(vars) == 0.0) {
                double val = evaluate(vars);
                return {vars, val};
            }
            // 위반이면 다시 루프 → VBA판과 동일
        }
    }

    // HM 업데이트 (worst 교체)
    void HarmonySearch::insertHarmony(const Harmony& h) {
        auto worstIt = std::min_element(HM.begin(), HM.end());
        if (h.value > worstIt->value) {
            *worstIt = h;
        }
    }

    // 최적화 수행
    Harmony HarmonySearch::optimize() {
        HM.clear();
        HM.reserve(params.HMS);

        // 1. 초기 HM 생성
        for (int i = 0; i < params.HMS; ++i)
            HM.push_back(generateFeasibleSolution());

        // 2. 진행률 표시줄 설정
        const int barWidth = 50;
        std::cerr << "[INFO] Optimization started..." << std::endl;

        auto print_progress = [&](int iter) {
            float progress = static_cast<float>(iter) / params.MaxImp;
            int pos = static_cast<int>(barWidth * progress);

            std::cerr << "\r[";
            for (int i = 0; i < barWidth; ++i)
                std::cerr << (i < pos ? "#" : "-");
            std::cerr << "] "
                      << std::setw(3) << int(progress * 100.0f) << "% "
                      << std::flush;
        };

        // 3. 반복 개선
        for (int iter = 0; iter < params.MaxImp; ++iter) {
            std::vector<double> newVars(problem.variables.size());

            for (size_t i = 0; i < problem.variables.size(); ++i) {
                auto r = std::generate_canonical<double, 10>(rng);
                if (r < params.HMCR) {
                    const auto& randHarmony = HM[rng() % HM.size()];
                    newVars[i] = randHarmony.vars[i];

                    if (std::generate_canonical<double, 10>(rng) < params.PAR) {
                        const auto& var = problem.variables[i];
                        double bw = (var.range.second - var.range.first) / params.N_Seg;
                        if (rng() % 2 == 0)
                            newVars[i] = std::min(var.range.second, newVars[i] + bw);
                        else
                            newVars[i] = std::max(var.range.first, newVars[i] - bw);
                        if (var.isInt) newVars[i] = std::round(newVars[i]);
                    }
                } else {
                    const auto& var = problem.variables[i];
                    if (var.isInt) {
                        std::uniform_int_distribution<int> idist(
                                static_cast<int>(var.range.first),
                                static_cast<int>(var.range.second));
                        newVars[i] = idist(rng);
                    } else {
                        std::uniform_real_distribution<double> rdist(
                                var.range.first, var.range.second);
                        newVars[i] = rdist(rng);
                    }
                }
            }

            if (problem.penalty(newVars) == 0.0) {
                double newVal = evaluate(newVars);
                insertHarmony({newVars, newVal});
            }

            // tqdm 스타일 한 줄 갱신
            if (iter % 100 == 0 || iter == params.MaxImp - 1)
                print_progress(iter + 1);
        }

        std::cerr << std::endl; // 줄 마무리

        // 4. 최적 해 반환
        return *std::max_element(HM.begin(), HM.end());
    }

    HSParams loadParams(const std::string& filename) {
        HSParams p{};
        std::ifstream in(filename);
        std::string key;
        while (in >> key) {
            if (key == "HMS") in.ignore(1, ',') >> p.HMS;
            else if (key == "HMCR") in.ignore(1, ',') >> p.HMCR;
            else if (key == "PAR") in.ignore(1, ',') >> p.PAR;
            else if (key == "MaxImp") in.ignore(1, ',') >> p.MaxImp;
            else if (key == "N_Seg") in.ignore(1, ',') >> p.N_Seg;
        }
        return p;
    }

    void editParams(HSParams param, int HMS, double HMCR, double PAR, unsigned int maxiter){
        param.HMS = HMS;
        param.HMCR = HMCR;
        param.PAR = PAR;
        param.MaxImp = maxiter;
    }

}
