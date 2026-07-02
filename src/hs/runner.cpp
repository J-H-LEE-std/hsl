/**
 * @file hsalgorithm.cpp
 * @brief HS engine caller and runner for HS-L(interpreter + optimization engine).
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-10-05
 */

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

#include "../interpreter/lexer.h"
#include "../interpreter/parser.h"
#include "params.h"
#include "hsalgorithm.h"
#include "runner.h"
#include "../utils/timing.h"
#include "../log/ExperimentLogger.h"

namespace hsl {
    /**
     * @brief Reads the entire content of a file into a string.
     * @param path The filesystem path to the HS source file.
     * @return A string containing the file's full content.
     * @throws std::runtime_error If the file cannot be opened.
     */
    static std::string readAll(const std::string& path) {
        std::ifstream in(path);
        if (!in) throw std::runtime_error("Cannot open HS source file: " + path);
        std::ostringstream ss; ss << in.rdbuf();
        return ss.str();
    }

    /**
     * @brief High-level entry point to run Harmony Search with a pre-built problem and parameters.
     * @param prob The defined optimization problem.
     * @param params The algorithm parameters (HMS, HMCR, etc.).
     * @param seed Random number generator seed.
     * @return The best Harmony found during optimization.
     */
    Harmony runHarmonySearch(const HSProblem& prob, const HSParams& params, unsigned int seed) {
        HarmonySearch hs(prob, params, seed);
        return hs.optimize();
    }

    /**
     * @brief Runs Harmony Search by first building a problem from a parsed Program AST.
     * @param program Pointer to the parsed AST.
     * @param params The algorithm parameters.
     * @param seed Random number generator seed.
     * @return The best Harmony found.
     */
    Harmony runHarmonySearch(Program* program, const HSParams& params, unsigned int seed) {
        HSProblem prob = buildHSProblem(program);
        return runHarmonySearch(prob, params, seed);
    }

    /**
     * @brief Loads an HS-L script from a file, parses it, and executes the Harmony Search solver.
     * @param hsFilePath Path to the .hs script file.
     * @param params The algorithm parameters.
     * @param seed Random number generator seed.
     * @param parseErrors Pointer to a vector where parsing errors will be stored (optional).
     * @param logger Pointer to an ExperimentLogger for tracking progress (optional).
     * @return An HSResult containing the best value, variables, and performance metrics.
     * @throws std::runtime_error If parsing fails or the file cannot be read.
     */
    HSResult runHarmonySearchFromFile(const std::string& hsFilePath,
                                     const HSParams& params,
                                     unsigned int seed,
                                     std::vector<std::string>* parseErrors,
                                     ExperimentLogger* logger) {
        std::string src = readAll(hsFilePath);

        hsl::Lexer lex(src);
        hsl::Parser parser(lex);

        Program* program = parser.parseProgram();

        // Copy errors when for utilizing
        if (parseErrors) {
            *parseErrors = parser.getErrors();
        }

        // If parse error detected, HS-L throw its error.
        const auto& errs = parser.getErrors();
        if (!errs.empty()) {
            std::ostringstream msg;
            msg << "Parse failed with " << errs.size() << " error(s):\n";
            for (const auto& e : errs) msg << "  - " << e << "\n";
            throw std::runtime_error(msg.str());
        }
        // TODO: REPL enhancement (improvements needed such as providing detailed error information)

        return runHarmonySearchWithLogger(buildHSProblem(program), params, seed, logger);
    }

    /**
     * @brief Internal runner that handles execution timing and experiment logging.
     * @param prob The defined optimization problem.
     * @param params The algorithm parameters.
     * @param seed Random number generator seed.
     * @param logger Pointer to the active ExperimentLogger.
     * @return An HSResult including CPU time and I/O time metrics.
     * @details This function measures the CPU time used for optimization and coordinates
     * various logging phases (Start, Iteration, End, Footer).
     */
    HSResult runHarmonySearchWithLogger(const HSProblem& prob,
                          const HSParams& params,
                          unsigned int seed,
                          ExperimentLogger* logger)
    {
        double cpu_start = get_process_cpu_time_sec();
        bool suppressProgress = logger && logger->isQuiet();
        HarmonySearch hs(prob, params, seed, logger, suppressProgress);
        if (logger) logger->logRunStart();
        Harmony best = hs.optimize();
        double cpu_end = get_process_cpu_time_sec();
        double cpu_elapsed = cpu_end - cpu_start;
        if (logger) {
            logger->logRunEnd(best);
            double log_io_time = logger->logIoTimeSeconds();
            logger->logRunFooter(cpu_elapsed, log_io_time);
        }

        HSResult result;
        result.value = best.value;
        result.vars = best.vars;
        result.cpu_time = cpu_elapsed;
        result.log_io_time = logger ? logger->logIoTimeSeconds() : 0.0;
        return result;
    }
}
