/**
 * @file func.cpp
 * @brief Defination for callable built-in function and constants in HS-L.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-10-06
 */

#include <cmath>
#include <cstdlib>
#include <numbers>
#include <limits>
#include "func.h"

namespace hsl {
    /**
    * @brief Provides a registry of built-in mathematical functions.
    * @return A reference to a static map containing function names and their corresponding C++20 lambda bindings.
    * @details This function maps HS-L DSL function identifiers directly to standard C++ math library implementations.
    */
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

    /**
    * @brief Provides a registry of built-in mathematical constants.
    * @return A reference to a static map containing constant names and their double-precision values.
    * @details This function exposes C++20 standard constants for use within the HS-L interpreter.
    */
    const std::unordered_map<std::string, double>& builtinConstants() {
        static const std::unordered_map<std::string, double> constants = {
                {"pi", std::numbers::pi},
                {"e",  std::numbers::e},
                {"inf", std::numeric_limits<double>::infinity()}
        };
        return constants;
    }
}
