#include <cmath>
#include <cstdlib>
#include <numbers>
#include <limits>
#include "func.h"

namespace hsl {

    const std::unordered_map<std::string, BuiltinFunc>& builtinFunctions() {
        static const std::unordered_map<std::string, BuiltinFunc> builtins = {
                {"abs",   [](const std::vector<double>& a){ return std::fabs(a[0]); }},
                {"sqrt",  [](const std::vector<double>& a){ return std::sqrt(a[0]); }},
                {"exp",   [](const std::vector<double>& a){ return std::exp(a[0]); }},
                {"log",   [](const std::vector<double>& a){ return std::log(a[0]); }},
                {"log10", [](const std::vector<double>& a){ return std::log10(a[0]); }},
                {"sin",   [](const std::vector<double>& a){ return std::sin(a[0]); }},
                {"cos",   [](const std::vector<double>& a){ return std::cos(a[0]); }},
                {"tan",   [](const std::vector<double>& a){ return std::tan(a[0]); }},
                {"asin",  [](const std::vector<double>& a){ return std::asin(a[0]); }},
                {"acos",  [](const std::vector<double>& a){ return std::acos(a[0]); }},
                {"atan",  [](const std::vector<double>& a){ return std::atan(a[0]); }},
                {"sinh",  [](const std::vector<double>& a){ return std::sinh(a[0]); }},
                {"cosh",  [](const std::vector<double>& a){ return std::cosh(a[0]); }},
                {"tanh",  [](const std::vector<double>& a){ return std::tanh(a[0]); }},
                {"floor", [](const std::vector<double>& a){ return std::floor(a[0]); }},
                {"ceil",  [](const std::vector<double>& a){ return std::ceil(a[0]); }},
                {"round", [](const std::vector<double>& a){ return std::round(a[0]); }},
                {"sign",  [](const std::vector<double>& a){ return (a[0] > 0) - (a[0] < 0); }},
                {"rand",  [](const std::vector<double>&){ return std::rand() / static_cast<double>(RAND_MAX); }},
                };

        return builtins;
    }

    const std::unordered_map<std::string, double>& builtinConstants() {
        static const std::unordered_map<std::string, double> constants = {
                {"pi", std::numbers::pi},
                {"e",  std::numbers::e},
                {"inf", std::numeric_limits<double>::infinity()}
        };
        return constants;
    }

}
