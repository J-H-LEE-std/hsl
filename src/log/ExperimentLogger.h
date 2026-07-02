/**
 * @file ExperimentLogger.h
 * @brief Asynchronous logging utility for recording Harmony Search experiment data.
 * @details Supports multiple output formats (TXT, CSV, JSONL) and uses a dedicated
 * worker thread to handle I/O operations without blocking the main optimization loop.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2026-02-24
 */

#ifndef HSL_EXPERIMENT_LOGGER_H
#define HSL_EXPERIMENT_LOGGER_H

#include <functional>
#include <atomic>
#include <fstream>
#include <ostream>
#include <string>
#include <vector>
#include <cstddef>
#include <deque>
#include <mutex>
#include <condition_variable>
#include "../hs/params.h"
#include "../utils/jthread.h"

namespace hsl {
    struct Harmony;

    enum class JsonMode {
        Summary,
        Snapshot,
        Full
    };

    struct LoggerOptions {
        std::string log_dir = "logs";
        std::string log_prefix = "hsl";
        bool enable_txt = true;
        bool enable_csv = true;
        bool enable_jsonl = true;
        JsonMode json_mode = JsonMode::Summary;
        int json_flush_every = 100; // flush buffered JSONL every N iterations (0 = never by iteration)
        std::size_t json_buffer_bytes = 4 * 1024 * 1024; // flush when buffer reaches this size
        bool reopen_on_flush = false; // close and reopen JSONL file on each flush
        int json_precision = 6;
        int json_stride = 50; // used for Snapshot mode
        int text_stride = 10; // 0 = every iteration
        bool quiet = false;
        int flush_every = 0; // 0 = never
        bool log_new_best = true;
        std::ostream* mirror = nullptr;
        std::function<void(const std::string&)> text_callback;
    };

    class ExperimentLogger {
        public:
            ExperimentLogger(const LoggerOptions& options,
                            const std::string& source_file,
                            const HSParams& params,
                            unsigned int seed);
            ~ExperimentLogger();

            void logRunStart();
            void logRunEnd(const Harmony& best);
            void logRunFooter(double cpu_time_sec, double log_io_sec);
            void logIteration(int iteration,
                            const Harmony& best,
                            double avg,
                            const std::vector<Harmony>& hm,
                            bool maximize);
            void logNewBest(int iteration, const Harmony& best);
            void flush();

            std::string textLogPath() const { return txt_path_; }
            std::string csvLogPath() const { return csv_path_; }
            std::string jsonlLogPath() const { return jsonl_path_; }
            bool isQuiet() const { return options_.quiet; }
            double logIoTimeSeconds() const;

        private:
            void emitText(const std::string& line);
            void writeRunHeader();
            void writeHMSnapshot(int iteration,
                                const Harmony& best,
                                double avg,
                                const std::vector<Harmony>& hm,
                                bool maximize);
            void appendJsonLine(const std::string& line, int iteration_for_flush);
            void flushJsonBuffer();
            void flushStreamsNow();

            std::string timestamp_;
            std::string prefix_;
            std::string txt_path_;
            std::string csv_path_;
            std::string jsonl_path_;
            LoggerOptions options_;
            HSParams params_;
            std::string source_file_;
            unsigned int seed_;
            std::ofstream txt_;
            std::ofstream csv_;
            std::ofstream jsonl_;
            bool started_ = false;
            bool ended_ = false;
            bool footer_written_ = false;
            bool json_ok_ = true;
            std::string json_buffer_;
            std::mutex mtx_;
            std::condition_variable cv_;
            std::deque<std::function<void()>> tasks_;
            bool stop_ = false;
            hsl::jthread worker_;
            int text_line_count_ = 0;
            std::atomic<long long> log_io_time_us_{0};

            void startWorker();
            void workerLoop();
            void enqueueTask(std::function<void()> fn);
    };
}

#endif
