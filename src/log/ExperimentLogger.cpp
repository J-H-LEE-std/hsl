/**
 * @file ExperimentLogger.cpp
 * @brief Implementation of asynchronous task queuing and multi-format file writing.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2026-02-24
 */

#include "ExperimentLogger.h"

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "../hs/hsalgorithm.h"

namespace fs = std::filesystem;

namespace hsl {
    namespace {
        /**
        * @brief Make timestamp for JSONL creating time.
        */
        std::string makeTimestamp() {
            auto now = std::chrono::system_clock::now();
            std::time_t t = std::chrono::system_clock::to_time_t(now);
            std::tm tm{};
        #ifdef _WIN32
            localtime_s(&tm, &t);
        #else
            localtime_r(&t, &tm);
        #endif
            std::ostringstream oss;
            oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
            return oss.str();
        }

        /**
        * @brief Internal helper to escape special characters for valid JSON output.
        */
        std::string escapeJson(const std::string& s) {
            std::string out;
            out.reserve(s.size());
            for (char c : s) {
                switch (c) {
                    case '\\': out += "\\\\"; break;
                    case '"': out += "\\\""; break;
                    case '\n': out += "\\n"; break;
                    case '\r': out += "\\r"; break;
                    case '\t': out += "\\t"; break;
                    default: out += c; break;
                }
            }
            return out;
        }

        std::string toJsonArray(const std::vector<double>& values, int precision) {
            std::ostringstream oss;
            oss << "[";
            for (size_t i = 0; i < values.size(); ++i) {
                if (i) oss << ",";
                oss << std::setprecision(precision) << values[i];
            }
            oss << "]";
            return oss.str();
        }

        class ScopedLogTimer {
            public:
                explicit ScopedLogTimer(std::atomic<long long>& acc)
                    : acc_(acc), start_(std::chrono::high_resolution_clock::now()) {}
                ~ScopedLogTimer() {
                    auto end = std::chrono::high_resolution_clock::now();
                    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start_).count();
                    acc_.fetch_add(us, std::memory_order_relaxed);
                }
            private:
                std::atomic<long long>& acc_;
                std::chrono::high_resolution_clock::time_point start_;
        };
    }

    ExperimentLogger::ExperimentLogger(const LoggerOptions& options,
                                    const std::string& source_file,
                                    const HSParams& params,
                                    unsigned int seed)
            : options_(options), params_(params), source_file_(source_file), seed_(seed) {
        timestamp_ = makeTimestamp();

        std::string base_prefix = options_.log_prefix.empty() ? "hsl" : options_.log_prefix;
        prefix_ = timestamp_ + "_seed" + std::to_string(seed_);
        if (!base_prefix.empty()) prefix_ = base_prefix + "_" + prefix_;

        // Basically logs are saved in "logs" directory created with same location with program binary.
        fs::path dir = options_.log_dir.empty() ? fs::path("logs") : fs::path(options_.log_dir);
        auto emitWarning = [&](const std::string& msg) {
            std::cerr << msg << '\n';
            if (options_.mirror) (*options_.mirror) << msg << '\n';
            if (options_.text_callback) options_.text_callback(msg);
        };
        try {
            fs::create_directories(dir);
        } catch (const std::exception&) {
            emitWarning("[HS-L] Failed to create log directory: " + dir.string());
            // keep going; files may fail to open
        }

        auto logOpenError = [&](const std::string& path){
            emitWarning("[HS-L] Failed to open log file: " + path);
        };

        // Check option and open proper log file
        if (options_.enable_txt) {
            txt_path_ = (dir / (prefix_ + ".txt")).string();
            txt_.open(txt_path_, std::ios::out | std::ios::trunc);
            if (!txt_.is_open()) logOpenError(txt_path_);
        }
        if (options_.enable_csv) {
            csv_path_ = (dir / (prefix_ + ".csv")).string();
            csv_.open(csv_path_, std::ios::out | std::ios::trunc);
            if (csv_) csv_ << "iter,best,avg\n";
            else logOpenError(csv_path_);
        }
        if (options_.enable_jsonl) {
            jsonl_path_ = (dir / (prefix_ + ".jsonl")).string();
            jsonl_.open(jsonl_path_, std::ios::out | std::ios::trunc);
            if (!jsonl_.is_open()) {
                json_ok_ = false;
                logOpenError(jsonl_path_);
            }
        }

        startWorker();
    }

    ExperimentLogger::~ExperimentLogger() {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            stop_ = true;
        }
        cv_.notify_all();
    }

    double ExperimentLogger::logIoTimeSeconds() const {
        return static_cast<double>(log_io_time_us_.load(std::memory_order_relaxed)) / 1000000.0;
    }

    /**
    * @brief Asynchronously emits a text log line to all configured outputs.
    * @param line The log message to be processed.
    * @details Enqueues a task to write the log to file, mirror, and GUI callback simultaneously.
    * It also manages conditional stream flushing based on the 'flush_every' threshold for data persistence.
    */
    void ExperimentLogger::emitText(const std::string& line) {
        enqueueTask([=]() {
            if (options_.enable_txt && txt_.is_open()) {
                txt_ << line << '\n';
            }
            if (options_.mirror) {
                (*options_.mirror) << line << '\n';
            }
            if (options_.text_callback) {
                options_.text_callback(line);
            }
            if (options_.flush_every > 0) {
                ++text_line_count_;
                // Flushing buffers is conducted periodically decided by user setting.
                if (text_line_count_ % options_.flush_every == 0) {
                    if (txt_.is_open()) txt_.flush();
                    if (csv_.is_open()) csv_.flush();
                    if (jsonl_.is_open()) jsonl_.flush();
                    if (options_.mirror) options_.mirror->flush();
                }
            }
        });
    }

    /**
    * @brief Buffers a JSON line and determines if a disk flush is required.
    * @param line The JSON string to append.
    * @param iteration_for_flush The current iteration number used to trigger periodic flushes.
    * @details Manages an internal string buffer to reduce the frequency of expensive write syscalls.
    * A flush is triggered if the buffer size exceeds json_buffer_bytes or if the iteration reaches the json_flush_every threshold.
    */
    void ExperimentLogger::appendJsonLine(const std::string& line, int iteration_for_flush) {
        enqueueTask([this, line, iteration_for_flush]() {
            if (!options_.enable_jsonl || !json_ok_) return;
            json_buffer_.append(line);
            json_buffer_.push_back('\n');
            bool hit_stride = options_.json_flush_every > 0 &&
                            iteration_for_flush > 0 &&
                            (iteration_for_flush % options_.json_flush_every == 0);
            bool hit_size = options_.json_buffer_bytes > 0 &&
                            json_buffer_.size() >= options_.json_buffer_bytes;
            if (hit_stride || hit_size) {
                flushJsonBuffer();
            }
        });
    }

    /**
    * @brief Flushes the accumulated JSON string buffer to the physical .jsonl file.
    * @details This method handles the actual disk I/O for buffered JSON data. It includes
    * error detection to disable logging upon failure and supports an optional "reopen-on-flush"
    * strategy to ensure file stream stability across multiple write operations.
    */
    void ExperimentLogger::flushJsonBuffer() {
        if (!options_.enable_jsonl || !json_ok_ || json_buffer_.empty()) return;

        auto warnDisable = [this]() {
            json_ok_ = false;
            std::string msg = "[HS-L] JSON log write failed; disabling JSON logging.";
            std::cerr << msg << '\n';
            if (options_.mirror) (*options_.mirror) << msg << '\n';
            if (options_.text_callback) options_.text_callback(msg);
        };

        // Enable log integrity safely by save and reload them.
        if (options_.reopen_on_flush) {
            if (jsonl_.is_open()) jsonl_.close();
            jsonl_.open(jsonl_path_, std::ios::out | std::ios::app);
            if (!jsonl_.is_open()) {
                warnDisable();
                json_buffer_.clear();
                return;
            }
        } else if (!jsonl_.is_open()) {
            warnDisable();
            json_buffer_.clear();
            return;
        }

        // JSON buffer safely stores logs and writes them periodically.
        jsonl_ << json_buffer_;
        if (!jsonl_) {
            warnDisable();
        }
        // Buffer flush after writing successfully.
        json_buffer_.clear();
    }

    /**
    * @brief Physically writes all buffered data to the respective file streams.
    * @details This method is typically executed by the worker thread. It ensures that
    * TXT, CSV, and JSONL streams are synchronized with the file system, and handles
    * the optional "reopen_on_flush" logic to maintain file integrity.
    * @note If flush does not apply, program might be shutted down due to memory problem.
    */
    void ExperimentLogger::flushStreamsNow() {
        flushJsonBuffer();
        if (txt_.is_open()) txt_.flush();
        if (csv_.is_open()) csv_.flush();
        if (jsonl_.is_open()) jsonl_.flush();
    }

    /**
    * @brief Captures and records a header of the optimization log.
    * @details JSONL log's header contain factors which affect reproducibility(parameter, source file, and seed).
    */
    void ExperimentLogger::writeRunHeader() {
        if (!options_.enable_jsonl || !json_ok_) return;

        std::ostringstream oss;
        oss << "{\"type\":\"run_header\","
            << "\"sourcefile\":\"" << escapeJson(source_file_) << "\","
            << "\"parameter\":{"
            << "\"hms\":" << params_.HMS << ","
            << "\"hmcr\":" << std::setprecision(options_.json_precision) << params_.HMCR << ","
            << "\"par\":" << std::setprecision(options_.json_precision) << params_.PAR << ","
            << "\"maxiter\":" << params_.MaxImp
            << "},"
            << "\"seed\":" << seed_
            << "}";
        appendJsonLine(oss.str(), 0);
    }

    /**
    * @brief Captures and records start time of running HS engine.
    */
    void ExperimentLogger::logRunStart() {
        if (started_) return;
        ScopedLogTimer timer(log_io_time_us_);
        started_ = true;
        writeRunHeader();

        std::ostringstream oss;
        oss << "[HS-L] Run start | seed=" << seed_
            << " | hms=" << params_.HMS
            << " | hmcr=" << params_.HMCR
            << " | par=" << params_.PAR
            << " | max_iter=" << params_.MaxImp
            << " | source=" << source_file_;
        emitText(oss.str());

        if (!txt_path_.empty()) emitText("[HS-L] Text log file: " + txt_path_);
        if (!csv_path_.empty()) emitText("[HS-L] CSV log file: " + csv_path_);
        if (!jsonl_path_.empty()) emitText("[HS-L] JSONL log file: " + jsonl_path_);
    }

    /**
    * @brief Captures and records a detailed snapshot of the current Harmony Memory.
    * @details Depending on the JsonMode, this function serializes the entire state of the
    * optimization (all candidate solutions and their values) into a structured JSON format.
    * Primarily used for post-run analysis and visualization of algorithm convergence.
    */
    void ExperimentLogger::writeHMSnapshot(int iteration,
                                        const Harmony& best,
                                        double avg,
                                        const std::vector<Harmony>& hm,
                                        bool maximize) {
        if (!options_.enable_jsonl || !json_ok_) return;
        if (options_.json_mode == JsonMode::Summary) return;

        if (hm.empty()) return;
        size_t best_index = 0;
        // Find best solution by searching index.
        for (size_t i = 1; i < hm.size(); ++i) {
            if (maximize ? hm[i].value > hm[best_index].value
                        : hm[i].value < hm[best_index].value) {
                best_index = i;
            }
        }

        std::ostringstream oss;
        // Create JSONL sentence with best solution.
        oss << "{\"type\":\"hm_snapshot\",\"iteration\":" << iteration << ",\"hm\":[";
        for (size_t i = 0; i < hm.size(); ++i) {
            if (i) oss << ",";
            oss << "{\"vector\":" << toJsonArray(hm[i].vars, options_.json_precision)
                << ",\"optima\":" << std::setprecision(options_.json_precision) << hm[i].value
                << "}";
        }
        oss << "],\"best\":{\"index\":" << best_index
            << ",\"optima\":" << std::setprecision(options_.json_precision) << best.value
            << "},\"avg\":" << std::setprecision(options_.json_precision) << avg
            << "}";

        appendJsonLine(oss.str(), iteration);
    }

    /**
    * @brief Captures and records every iterations of optimizing process.
    * @details The method generates appropriate log text for each iteration
    * based on the given options and outputs it via a stream.
    */
    void ExperimentLogger::logIteration(int iteration,
                                        const Harmony& best,
                                        double avg,
                                        const std::vector<Harmony>& hm,
                                        bool maximize) {
        if (!started_) logRunStart();
        ScopedLogTimer timer(log_io_time_us_);

        // In quiet mode, best value only emit via stream.
        if (!options_.quiet) {
            if (options_.text_stride == 0 || iteration % options_.text_stride == 0 || iteration == params_.MaxImp) {
                std::ostringstream line;
                line << "[Iter " << iteration << "] best=" << best.value << ", avg=" << avg;
                emitText(line.str());
            }
        }

        // CSV log also recorded when option is on
        if (options_.enable_csv && csv_.is_open()) {
            enqueueTask([=]() {
                if (csv_.is_open()) {
                    csv_ << iteration << "," << std::setprecision(12) << best.value << "," << avg << "\n";
                }
            });
        }
        bool snapshot_stride_hit = options_.json_stride > 0 &&
                                (iteration % options_.json_stride == 0);
        bool should_log_snapshot = (options_.json_mode == JsonMode::Full) ||
                                (options_.json_mode == JsonMode::Snapshot && snapshot_stride_hit);
        if (should_log_snapshot) {
            writeHMSnapshot(iteration, best, avg, hm, maximize);
        }
    }

    /**
     * @brief Records a new "Best" harmony discovery during the run.
     * @param iteration The iteration at which the new best was found.
     * @param best The new best harmony data.
     */
    void ExperimentLogger::logNewBest(int iteration, const Harmony& best) {
        if (!started_) logRunStart();
        ScopedLogTimer timer(log_io_time_us_);

        if (options_.log_new_best) {
            std::ostringstream line;
            line << "[NEW BEST] iter=" << iteration << ", value=" << best.value
                << ", vector=" << toJsonArray(best.vars, options_.json_precision);
            emitText(line.str());
        }

        if (options_.enable_jsonl && json_ok_) {
            std::ostringstream oss;
            oss << "{\"type\":\"new_best\",\"iteration\":" << iteration
                << ",\"best_value\":" << std::setprecision(options_.json_precision) << best.value
                << ",\"best_vector\":" << toJsonArray(best.vars, options_.json_precision) << "}";
            appendJsonLine(oss.str(), iteration);
        }
    }

    /**
    * @brief Captures and records a footer of the optimization log.
    * @param cputime Elipsed CPU time.
    * @param logiotime Elipsed IO time for log.
    * @details JSONL log's footer contains elipsed time by CPU time and wall clock time.
    */
    void ExperimentLogger::logRunFooter(double cpu_time_sec, double log_io_sec) {
        if (footer_written_) return;
        footer_written_ = true;
        ScopedLogTimer timer(log_io_time_us_);
        if (!options_.enable_jsonl || !json_ok_) return;

        std::ostringstream oss;
        oss << "{\"type\":\"run_footer\",\"cpu_time_sec\":"
            << std::setprecision(options_.json_precision) << cpu_time_sec
            << ",\"log_io_sec\":" << std::setprecision(options_.json_precision) << log_io_sec
            << "}";
        appendJsonLine(oss.str(), 0);
    }

    /**
     * @brief End writing log.
     * @param best The new best harmony data.
     */
    void ExperimentLogger::logRunEnd(const Harmony& best) {
        if (!started_) logRunStart();
        if (ended_) return;
        ScopedLogTimer timer(log_io_time_us_);
        ended_ = true;

        std::ostringstream oss;
        oss << "[HS-L] Run finished | best=" << best.value;
        emitText(oss.str());
        flush(); // Flush buffer before destruct it.
    }

    /**
     * @brief Flushes all active file streams.
     * @details Thread-safe call that ensures all buffered log data is written to disk.
     */
    void ExperimentLogger::flush() {
        enqueueTask([this]() {
            flushStreamsNow();
        });
    }

    // HS-L use separated jthread to record the log.
    void ExperimentLogger::startWorker() {
        worker_ = hsl::jthread([this] { workerLoop(); });
    }

    /**
     * @brief Enqueues a logging task to be processed by the worker thread.
     * @param fn A functional object containing the I/O logic to execute.
     */
    void ExperimentLogger::enqueueTask(std::function<void()> fn) {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            tasks_.push_back(std::move(fn));
        }
        cv_.notify_one();
    }

    /**
     * @brief The main loop for the background worker thread.
     * @details Continuously pulls logging tasks from the queue and executes them.
     * This ensures that slow disk I/O does not delay the Harmony Search algorithm.
     */
    void ExperimentLogger::workerLoop() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(mtx_);
                cv_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });
                if (stop_ && tasks_.empty()) break;
                task = std::move(tasks_.front());
                tasks_.pop_front();
            }
            if (task) task();
        }
        flushStreamsNow();
    }
}
