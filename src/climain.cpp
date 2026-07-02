/**
 * @file climain.cpp
 * @brief Main enter point for CLI program.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-11-28
 */

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <iostream>
#include <iomanip>
#include <random>
#include <string>
#include <CLI/CLI.hpp>
#include "hs/params.h"
#include "hs/runner.h"
#include "log/ExperimentLogger.h"

int main(int argc, char** argv) {
    CLI::App app{"HS-L Command Line Interface"};

    std::string source_file = "input.hs";
    std::string param_file;
    int HMS = 30;
    double HMCR = 0.95;
    double PAR = 0.7;
    unsigned int max_iter = 30000;
    unsigned int seed = std::random_device{}();
    std::string log_dir = "logs";
    std::string log_prefix = "hsl";
    bool log_txt = true;
    bool log_csv = true;
    bool log_jsonl = true;
    std::string json_mode = "summary";
    int json_flush_every = 100;
    std::size_t json_buffer_bytes = 4 * 1024 * 1024;
    bool reopen_on_flush = false;
    int json_precision = 6;
    int json_stride = 50;
    int text_stride = 10;
    bool quiet = false;

    // Define CLI options.
    app.add_option("-s,--source", source_file, "HS-L source file (.hs)");
    app.add_option("-p,--param", param_file, "Parameter file (.hsparm)");
    app.add_option("--HMS", HMS, "Harmony Memory Size (default: 30)");
    app.add_option("--HMCR", HMCR, "Harmony Memory Consideration Rate (default: 0.95)");
    app.add_option("--PAR", PAR, "Pitch Adjusting Rate (default: 0.7)");
    app.add_option("--max_iter", max_iter, "Maximum number of iterations (default: 30000)");
    app.add_option("--seed", seed, "Random seed (default: random_device)");
    app.add_option("--log_dir", log_dir, "Directory to write logs (default: logs)");
    app.add_option("--log_prefix", log_prefix, "Log file prefix (default: hsl)");
    app.add_option("--log_txt", log_txt, "Enable text log file (default: 1)")->default_val(true);
    app.add_option("--log_csv", log_csv, "Enable CSV log file (default: 1)")->default_val(true);
    app.add_option("--log_jsonl", log_jsonl, "Enable JSONL log file (default: 1)")->default_val(true);
    app.add_option("--json_mode", json_mode, "JSONL logging mode: summary|snapshot|full (default: summary)")
        ->check(CLI::IsMember({"summary", "snapshot", "full"}, CLI::ignore_case));
    app.add_option("--json_flush_every", json_flush_every, "Flush buffered JSONL every N iterations (default: 100)")->check(CLI::PositiveNumber);
    app.add_option("--json_buffer_bytes", json_buffer_bytes, "Flush JSONL buffer when reaching this byte size (default: 4194304)");
    app.add_flag("--reopen_on_flush", reopen_on_flush, "Close/reopen JSONL file on each flush (default: off)");
    app.add_option("--json_precision", json_precision, "Floating point precision in JSON logs (default: 6)");
    app.add_option("--json_stride", json_stride, "Stride for HM snapshot logging in snapshot mode (default: 50)");
    app.add_option("--text_stride", text_stride, "Stride for text iteration logs (default: 10; 0=every)");
    app.add_flag("--quiet", quiet, "Only log important events (default: off)");
    CLI11_PARSE(app, argc, argv);

    std::cout << "[INFO] Starting HS-L...\n";
    try {
        // Setting parameters.
        hsl::HSParams params;

        if (!param_file.empty() && std::filesystem::exists(param_file)) {
            params = hsl::loadParams(param_file);
            std::cout << "[INFO] Loaded parameter file: " << param_file << "\n";
        } else {
            std::cerr << "[WARN] Parameter file not found, using default CLI parameters.\n";
            params.HMS    = 30;
            params.HMCR   = 0.95;
            params.PAR    = 0.7;
            params.MaxImp = 30000;
            params.N_Seg  = 300;
        }

        if (app.count("--HMS"))    params.HMS    = HMS;
        if (app.count("--HMCR"))   params.HMCR   = HMCR;
        if (app.count("--PAR"))    params.PAR    = PAR;
        if (app.count("--max_iter")) params.MaxImp = max_iter;

        // Setting log oprions.
        hsl::LoggerOptions log_opts;
        log_opts.log_dir = log_dir;
        log_opts.log_prefix = log_prefix;
        log_opts.enable_txt = log_txt;
        log_opts.enable_csv = log_csv;
        log_opts.enable_jsonl = log_jsonl;
        std::string json_mode_lc = json_mode;
        std::transform(json_mode_lc.begin(), json_mode_lc.end(), json_mode_lc.begin(),
                       [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        if (json_mode_lc == "snapshot") {
            log_opts.json_mode = hsl::JsonMode::Snapshot;
        } else if (json_mode_lc == "full") {
            log_opts.json_mode = hsl::JsonMode::Full;
        } else {
            log_opts.json_mode = hsl::JsonMode::Summary;
        }
        log_opts.json_flush_every = json_flush_every;
        log_opts.json_buffer_bytes = json_buffer_bytes;
        log_opts.reopen_on_flush = reopen_on_flush;
        log_opts.json_precision = json_precision;
        log_opts.json_stride = json_stride;
        log_opts.text_stride = text_stride;
        log_opts.quiet = quiet;
        log_opts.mirror = &std::cout;

        // Execute interpreter and solver.
        hsl::ExperimentLogger logger(log_opts, source_file, params, seed);
        auto best = hsl::runHarmonySearchFromFile(source_file, params, seed, nullptr, &logger);

        std::cout << "Best value: " << best.value << "\n";
        for (size_t i = 0; i < best.vars.size(); ++i)
            std::cout << "x[" << i + 1 << "] = " << best.vars[i] << "\n";
        std::cout << std::fixed << std::setprecision(6);
    } catch (const std::exception& e) {
        std::cerr << "[Error] " << e.what() << "\n";
        return 1;
    }
    return 0;
}
