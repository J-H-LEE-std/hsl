#include <iostream>
#include <iomanip>
#include <random>
#include <CLI/CLI.hpp>
#include "hs/params.h"
#include "hs/runner.h"

int main(int argc, char** argv) {
    CLI::App app{"HS-L Command Line Interface"};

    std::string source_file = "input.hs";
    std::string param_file;
    int HMS = 30;
    double HMCR = 0.95;
    double PAR = 0.7;
    unsigned int max_iter = 30000;
    unsigned int seed = std::random_device{}();


    app.add_option("-s,--source", source_file, "HS-L source file (.hs)");
    app.add_option("-p,--param", param_file, "Parameter file (.hsparm)");
    app.add_option("--HMS", HMS, "Harmony Memory Size (default: 30)");
    app.add_option("--HMCR", HMCR, "Harmony Memory Consideration Rate (default: 0.95)");
    app.add_option("--PAR", PAR, "Pitch Adjusting Rate (default: 0.7)");
    app.add_option("--max_iter", max_iter, "Maximum number of iterations (default: 30000)");
    app.add_option("--seed", seed, "Random seed (default: random_device)");
    CLI11_PARSE(app, argc, argv);

    std::cout << "[INFO] Starting HS-L..." << std::endl;
    try {
        hsl::HSParams params;

        if (!param_file.empty() && std::filesystem::exists(param_file)) {
            params = hsl::loadParams(param_file);
            std::cout << "[INFO] Loaded parameter file: " << param_file << std::endl;
        } else {
            std::cerr << "[WARN] Parameter file not found, using default CLI parameters.\n";
            params.HMS    = 30;
            params.HMCR   = 0.95;
            params.PAR    = 0.7;
            params.MaxImp = 30000;
            params.N_Seg  = 10;
        }

        if (app.count("--HMS"))    params.HMS    = HMS;
        if (app.count("--HMCR"))   params.HMCR   = HMCR;
        if (app.count("--PAR"))    params.PAR    = PAR;
        if (app.count("--max_iter")) params.MaxImp = max_iter;

        auto best = hsl::runHarmonySearchFromFile(source_file, params, seed, nullptr);

        std::cout << "Best value: " << best.value << "\n";
        for (size_t i = 0; i < best.vars.size(); ++i)
            std::cout << "x[" << i + 1 << "] = " << best.vars[i] << "\n";
        std::cout << std::fixed << std::setprecision(6);
    } catch (const std::exception& e) {
        std::cerr << "[Error] " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
