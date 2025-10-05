#include <iostream>
#include <CLI/CLI.hpp>
#include "hs/params.h"
#include "hs/runner.h"

int main(int argc, char** argv) {
    CLI::App app{"HS-L Optimizer"};

    std::string source_file = "input.hs";
    std::string param_file = "parameter.hsparm";
    double HMCR = 0.95;
    double PAR = 0.7;
    int HMS = 30;
    unsigned int max_iter = 30000;

    app.add_option("-s,--source", source_file, "Source file path");
    app.add_option("-p,--param", param_file, "Parameter file path");
    app.add_option("--HMCR", HMCR, "Harmony Memory Consideration Rate");
    app.add_option("--PAR", PAR, "Pitch Adjusting Rate");
    app.add_option("--HMS", HMS, "Harmony Memory Size");
    app.add_option("--max_iter", max_iter, "Maximum number of iterations");

    CLI11_PARSE(app, argc, argv);
    std::cout << "[INFO] Starting HS-L...\n" << std::flush;

    auto start = std::chrono::high_resolution_clock::now();

    hsl::HSParams params = hsl::loadParams(param_file);
    hsl::editParams(params, HMS, HMCR, PAR, max_iter);

    try {
        hsl::Harmony best = hsl::runHarmonySearchFromFile(source_file, params);
        std::cout << "=== Harmony Search Result ===" << std::endl;
        std::cout << "Best value: " << best.value << std::endl;
        for (size_t i = 0; i < best.vars.size(); ++i) {
            std::cout << "  x" << (i + 1) << " = " << best.vars[i] << std::endl;
        }

        std::cout.flush();
        std::cerr.flush();
    } catch (const std::exception& ex) {
        std::cerr << "[ERROR] " << ex.what() << std::endl;
        return 1;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "CPU Time: " << elapsed.count() << " sec" << std::endl;

    std::cout.flush();
    std::cerr.flush();

    return 0;
}
