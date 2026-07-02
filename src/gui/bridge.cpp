/**
 * @file bridge.cpp
 * @brief Detailed bridge operation for HS engine and GUI.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-11-28
 */

#include "bridge.h"
#include <fstream>
#include <ctime>
#include <memory>
#include <functional>
#include "../hs/io.h"  // for connectiong hsl::cout
#include "../log/ExperimentLogger.h"
#include "../hs/runner.h"

namespace hslgui {
    // Define GUI log stream.
    std::ostringstream Bridge::cout;

    namespace {
        // Callback-based streambuf: Buffer deliver stream by '\n'.
        class CallbackStreambuf : public std::streambuf {
        public:
            explicit CallbackStreambuf(std::function<void(const std::string&)> cb) : callback(std::move(cb)) {}
        protected:
            int overflow(int ch) override {
                if (ch != EOF) {
                    buffer.push_back(static_cast<char>(ch));
                    if (ch == '\n') flush();
                }
                return ch;
            }
            int sync() override {
                flush();
                return 0;
            }
        private:
            void flush() {
                if (!callback || buffer.empty()) {
                    buffer.clear();
                    return;
                }
                if (!buffer.empty() && buffer.back() == '\n') buffer.pop_back();
                callback(buffer);
                buffer.clear();
            }
            std::function<void(const std::string&)> callback;
            std::string buffer;
        };
    }

    /**
    * @brief Internal execution logic that coordinates parsing and algorithm execution for the GUI.
    * @param source The HS-L DSL source code provided by the user.
    * @param param Parameters for the Harmony Search algorithm.
    * @param seed Random seed for reproducibility.
    * @param enable_text_log Boolean to enable or disable textual logging.
    * @param json_settings Configuration for JSON-based experimental logging.
    * @param log_callback A callback function to redirect log output to the GUI console.
    * @return A ResultStruct containing the optimization results or detailed error messages if parsing fails.
    * @details This function handles the full lifecycle of a GUI-triggered run: tokenizing, parsing,
    * building the HS problem, configuring the ExperimentLogger with various export formats (CSV, JSONL),
    * and executing the solver.
    */
    static hslgui::ResultStruct RunParsed(const std::string& source,
                                        const hslgui::ParamStruct& param,
                                        unsigned int seed,
                                        bool enable_text_log,
                                        const hslgui::JsonLogSettings& json_settings,
                                        const std::function<void(const std::string&)>& log_callback) {
        ResultStruct res;

        // Parse
        hsl::Lexer lex(source);
        hsl::Parser parser(lex);
        auto program = parser.parseProgram();
        const auto& errs = parser.getErrors();

        if (!errs.empty()) {
            std::ostringstream em;
            em << "Parse failed with " << errs.size() << " error(s):\n";
            for (auto& e : errs) em << "  - " << e << "\n";
            res.error_msg = em.str();
            return res;
        }

        // Create Evaluator logic and Problem
        auto problem = hsl::buildHSProblem(program);
        hsl::HSParams p;
        p.HMS = param.HMS;
        p.HMCR = param.HMCR;
        p.PAR = param.PAR;
        p.MaxImp = param.MaxImp;
        p.N_Seg = param.N_Seg;

        // Execute HS
        hsl::LoggerOptions log_opts;
        log_opts.log_dir = "logs";
        log_opts.log_prefix = "gui";
        log_opts.enable_txt = json_settings.enable_logging && enable_text_log;
        log_opts.enable_csv = json_settings.enable_logging && json_settings.enable_csv;
        log_opts.enable_jsonl = json_settings.enable_logging && json_settings.enable_jsonl;
        log_opts.json_mode = json_settings.json_mode;
        log_opts.json_stride = json_settings.json_stride;
        log_opts.json_flush_every = json_settings.json_flush_every;
        log_opts.json_buffer_bytes = json_settings.json_buffer_bytes;
        log_opts.reopen_on_flush = json_settings.reopen_on_flush;
        log_opts.json_precision = json_settings.json_precision;
        log_opts.text_stride = 20;
        log_opts.quiet = json_settings.quiet;
        log_opts.flush_every = 0;
        log_opts.text_callback = log_callback;
        hsl::ExperimentLogger logger(log_opts, "gui_input", p, seed);

        auto out = hsl::runHarmonySearchWithLogger(problem, p, seed, &logger);

        // Return value
        res.best_value = out.value;
        res.cpu_time = out.cpu_time;
        res.log_io_time = out.log_io_time;
        res.variables.reserve(problem.variables.size());
        for (size_t i = 0; i < problem.variables.size() && i < out.vars.size(); ++i)
            res.variables.emplace_back(problem.variables[i].name, out.vars[i]);

        return res;
    }

    /**
    * @brief Public API called by the GUI to initiate an optimization task.
    * @param source The HS-L DSL source code.
    * @param param Parameters for the Harmony Search algorithm.
    * @param seed Random seed.
    * @param enable_text_log Flag for textual logging.
    * @param json_settings Detailed settings for JSON logging.
    * @param log_callback Function to handle log strings (e.g., updating a text control).
    * @return The final results of the optimization process.
    * @details This function acts as a wrapper that safely redirects `hsl::cout` to the GUI's
    * logging callback using a custom streambuf, ensuring that algorithm progress is visible
    * in the interface during execution.
    */
    ResultStruct Bridge::Run(const std::string& source,
                            const ParamStruct& param,
                            unsigned int seed,
                            bool enable_text_log,
                            const JsonLogSettings& json_settings,
                            const std::function<void(const std::string&)>& log_callback) {
        // Connect hsl::cout to GUI callback
        std::unique_ptr<CallbackStreambuf> cbBuf;
        std::unique_ptr<std::ostream> cbStream;
        std::streambuf* oldBuf = nullptr;
        if (log_callback) {
            cbBuf = std::make_unique<CallbackStreambuf>(log_callback);
            cbStream = std::make_unique<std::ostream>(cbBuf.get());
            oldBuf = hsl::cout.rdbuf(cbStream->rdbuf());
        }

        auto res = RunParsed(source, param, seed, enable_text_log, json_settings, log_callback);

        if (oldBuf) hsl::cout.rdbuf(oldBuf);
        return res;
    }
}
