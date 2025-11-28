#ifndef HSL_GUI_BRIDGE_H
#define HSL_GUI_BRIDGE_H
#include <string>
#include <sstream>
#include <vector>
#include "../hs/hsalgorithm.h"
#include "../interpreter/lexer.h"
#include "../interpreter/parser.h"
#include "paramstruct.h"

namespace hslgui {
struct ResultStruct {
    double best_value = 0.0;
    std::vector<std::pair<std::string,double>> variables;
    double cpu_time = 0.0;
    std::string error_msg;
};
class Bridge {
public:
    static std::ostringstream cout;
    static ResultStruct Run(const std::string& source, const ParamStruct& param, unsigned int seed);
};
}
#endif
