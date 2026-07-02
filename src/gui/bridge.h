/**
 * @file bridge.h
 * @brief Header for GUI bridge defination.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-11-28
 */

#ifndef HSL_GUI_BRIDGE_H
#define HSL_GUI_BRIDGE_H

#include <string>
#include <sstream>
#include <vector>
#include <functional>
#include "../log/ExperimentLogger.h"
#include "../hs/hsalgorithm.h"
#include "../interpreter/lexer.h"
#include "../interpreter/parser.h"
#include "paramstruct.h"
#include "jsonlogsettings.h"

namespace hslgui {
    struct ResultStruct {
        double best_value = 0.0;
        std::vector<std::pair<std::string,double>> variables;
        double cpu_time = 0.0;
        double log_io_time = 0.0;
        std::string error_msg;
    };
    class Bridge {
    public:
        static std::ostringstream cout;
        static ResultStruct Run(const std::string& source,
                                const ParamStruct& param,
                                unsigned int seed,
                                bool enable_text_log,
                                const JsonLogSettings& json_settings = JsonLogSettings(),
                                const std::function<void(const std::string&)>& log_callback = {});
    };
}

#endif
