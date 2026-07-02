/**
 * @file jsonlogwindow.cpp
 * @brief Header of the JsonLogSetting class for logging options for HS optimization.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2026-02-24
 */

#ifndef HSL_GUI_JSONLOGSETTINGS_H
#define HSL_GUI_JSONLOGSETTINGS_H

#include <cstddef>
#include "../log/ExperimentLogger.h"

namespace hslgui {
    struct JsonLogSettings {
        bool enable_logging = false;
        bool enable_txt = false;
        bool enable_csv = false;
        bool enable_jsonl = false;
        hsl::JsonMode json_mode = hsl::JsonMode::Summary;
        int json_stride = 50;
        int json_flush_every = 100;
        std::size_t json_buffer_bytes = 4 * 1024 * 1024;
        bool reopen_on_flush = false;
        int json_precision = 6;
        bool quiet = false;
    };

    JsonLogSettings LoadJsonLogSettings();
    void SaveJsonLogSettings(const JsonLogSettings& settings);

}

#endif
