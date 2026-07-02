/**
 * @file func.h
 * @brief Header file for callable built-in function and constants.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-10-06
 */


#ifndef HSL_FUNC_
#define HSL_FUNC_

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace hsl {
    using BuiltinFunc = std::function<double(const std::vector<double>&)>;

    const std::unordered_map<std::string, BuiltinFunc>& builtinFunctions();
    const std::unordered_map<std::string, double>& builtinConstants();
}

#endif
