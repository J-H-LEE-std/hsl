/**
 * @file hsalgorithm.cpp
 * @brief Detailed implementation for essential HS algorithm.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-10-05
 */

#include <iomanip>
#include <algorithm>
#include <limits>
#include <iostream>
#include <fstream>
#include <numeric>
#include <stdexcept>
#include <chrono>
#include "hsalgorithm.h"
#include "io.h"
#include "../log/ExperimentLogger.h"

namespace hsl {
    // Functions for comparison and calculation with each Harmony
    namespace {
        bool isBetter(const Harmony& a, const Harmony& b, bool maximize) {
            return maximize ? a.value > b.value : a.value < b.value;
        }

        const Harmony& pickBest(const std::vector<Harmony>& hm, bool maximize) {
            if (hm.empty()) throw std::runtime_error("Harmony memory is empty");
            if (maximize) {
                return *std::max_element(
                    hm.begin(), hm.end(),
                    [](const Harmony& lhs, const Harmony& rhs) { return lhs.value < rhs.value; }
                );
            }
            return *std::min_element(
                hm.begin(), hm.end(),
                [](const Harmony& lhs, const Harmony& rhs) { return lhs.value < rhs.value; }
            );
        }

        double averageValue(const std::vector<Harmony>& hm) {
            if (hm.empty()) return 0.0;
            double sum = std::accumulate(
                hm.begin(), hm.end(), 0.0,
                [](double acc, const Harmony& h) { return acc + h.value; }
            );
            return sum / static_cast<double>(hm.size());
        }
    }

    HarmonySearch::HarmonySearch(const HSProblem& prob, const HSParams& params, unsigned int seed,
                                 ExperimentLogger* logger, bool suppressProgress)
            : problem(prob), params(params), rng(seed), logger(logger), suppressProgress(suppressProgress) {
    }

    /**
     * @brief Evaluates the objective value of a given solution while considering constraints.
     * @param solution A vector of values representing a candidate solution.
     * @return The objective value. Returns infinity or lowest double if constraints are violated.
     * @note Penalty handling follows a "hard constraint" approach by invalidating infeasible solutions.
     */
    double HarmonySearch::evaluate(const std::vector<double>& solution) {
        double obj = problem.objective(solution);
        double pen = problem.penalty(solution);

        if (std::isinf(pen)) {
            // Constraint violation is considered as invalid solution.
            return problem.maximize ?
                std::numeric_limits<double>::lowest() :
                std::numeric_limits<double>::infinity();
        }
        return obj; // Due to unray problem, return value not inversed.
    }

    /**
     * @brief Generates a random solution that satisfies all defined constraints.
     * @return A Harmony object containing a feasible set of variables and its evaluated value.
     * @details Uses a trial-and-error approach (re-sampling) until a valid solution is found,
     * ensuring the initial Harmony Memory (HM) is filled with feasible candidates.
     */
    Harmony HarmonySearch::generateFeasibleSolution() {
        while (true) {
            std::vector<double> vars(problem.variables.size());

            for (size_t i = 0; i < problem.variables.size(); i++) {
                const auto& var = problem.variables[i];
                if (var.isInt) {
                    std::uniform_int_distribution<int> idist(
                        static_cast<int>(var.range.first),
                        static_cast<int>(var.range.second)
                    );
                    vars[i] = idist(rng);
                } else {
                    std::uniform_real_distribution<double> rdist(
                        var.range.first, var.range.second
                    );
                    vars[i] = rdist(rng);
                }
            }

            // Check constraint
            if (problem.penalty(vars) == 0.0) {
                double val = evaluate(vars);
                return {vars, val};
            }
            // Loop when invalid solution created.
        }
    }

    /**
     * @brief Updates the Harmony Memory by replacing the worst harmony with a better candidate.
     * @param h The new candidate harmony to be considered for the memory.
     * @details Depending on whether the problem is maximization or minimization,
     * it identifies the worst performing member in HM and replaces it if the candidate is superior.
     */
    void HarmonySearch::insertHarmony(const Harmony& h) {
        if (problem.maximize) {
            auto worstIt = std::min_element(
                HM.begin(), HM.end(),
                [](const Harmony& a, const Harmony& b) { return a.value < b.value; }
            );
            if (h.value > worstIt->value) *worstIt = h;
        } else {
            auto worstIt = std::max_element(
                HM.begin(), HM.end(),
                [](const Harmony& a, const Harmony& b) { return a.value < b.value; }
            );
            if (h.value < worstIt->value) *worstIt = h;
        }
    }

    /**
     * @brief Executes the main Harmony Search optimization loop.
     * @return The best harmony found after reaching the maximum number of improvisations.
     * @details The process involves:
     * 1. Initializing the Harmony Memory (HM).
     * 2. Improvising new harmonies based on HMCR and PAR parameters.
     * 3. Updating the HM and logging progress/results.
     */
    Harmony HarmonySearch::optimize() {
        HM.clear();
        HM.reserve(params.HMS);

        // 1. Initial HM generation
        for (int i = 0; i < params.HMS; ++i)
            HM.push_back(generateFeasibleSolution());

        Harmony bestSoFar = pickBest(HM, problem.maximize);

        // 2. Setting Progressbar
        const int barWidth = 50;
        if (!suppressProgress) hsl::cout << "[INFO] Optimization started...\n";

        auto print_progress = [&](int iter) {
            if (suppressProgress) return;
            float progress = static_cast<float>(iter) / params.MaxImp;
            int pos = static_cast<int>(barWidth * progress);

            hsl::cout << "\r[";
            for (int i = 0; i < barWidth; ++i)
                hsl::cout << (i < pos ? "#" : "-");
            hsl::cout << "] "
                      << std::setw(3) << int(progress * 100.0f) << "% "
                      << std::flush;
        };

        // 3. Iterate and update solutions
        for (int iter = 1; iter <= static_cast<int>(params.MaxImp); ++iter) {
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
                            static_cast<int>(var.range.second)
                        );
                        newVars[i] = idist(rng);
                    } else {
                        std::uniform_real_distribution<double> rdist(
                            var.range.first, var.range.second
                        );
                        newVars[i] = rdist(rng);
                    }
                }
            }

            if (problem.penalty(newVars) == 0.0) {
                double newVal = evaluate(newVars);
                insertHarmony({newVars, newVal});
            }

            Harmony currentBest = pickBest(HM, problem.maximize);
            double avg = averageValue(HM);
            if (logger) {
                logger->logIteration(iter, currentBest, avg, HM, problem.maximize);
            }
            if (isBetter(currentBest, bestSoFar, problem.maximize)) {
                bestSoFar = currentBest;
                if (logger) logger->logNewBest(iter, bestSoFar);
            }

            if (iter % 100 == 0 || iter == params.MaxImp)
                print_progress(iter);
        }

        if (!suppressProgress) hsl::cout << '\n';

        // 4. Return optimal solution
        return pickBest(HM, problem.maximize);
    }

    /**
     * @brief Loads Harmony Search parameters from a configuration file.
     * @param filename Path to the .hsparm file.
     * @return An HSParams structure populated with the loaded values.
     * @note Expected format: "KEY,VALUE" (e.g., HMS,30).
     * @note If parameter file not provided or cannot be open, initial parameter is used.
     */
    HSParams loadParams(const std::string& filename) {
        HSParams p{};
        std::ifstream in(filename);
        // Parsing parameter as CSV document with .hsparm file
        auto trim = [](std::string& s) {
            const auto first = s.find_first_not_of(" \t\r\n");
            if (first == std::string::npos) {
                s.clear();
                return;
            }
            const auto last = s.find_last_not_of(" \t\r\n");
            s = s.substr(first, last - first + 1);
        };

        std::string line;
        while (std::getline(in, line)) {
            auto comma = line.find(',');
            if (comma == std::string::npos) continue;

            std::string key = line.substr(0, comma);
            std::string value = line.substr(comma + 1);
            trim(key);
            trim(value);

            try {
                if (key == "HMS") p.HMS = std::stoi(value);
                else if (key == "HMCR") p.HMCR = std::stod(value);
                else if (key == "PAR") p.PAR = std::stod(value);
                else if (key == "MaxImp") p.MaxImp = static_cast<unsigned int>(std::stoul(value));
                else if (key == "N_Seg") p.N_Seg = std::stoi(value);
            } catch (...) {
                continue;
            }
        }
        return p;
    }

    /**
     * @brief Manually updates the HS parameters with new values.
     * @param param Reference to the HSParams object to be modified.
     * @param HMS Harmony Memory Size.
     * @param HMCR Harmony Memory Consideration Rate.
     * @param PAR Pitch Adjustment Rate.
     * @param maxiter Maximum number of improvisations (MaxImp).
     */
    void editParams(HSParams& param, int HMS, double HMCR, double PAR, unsigned int maxiter) {
        param.HMS = HMS;
        param.HMCR = HMCR;
        param.PAR = PAR;
        param.MaxImp = maxiter;
    }
}
