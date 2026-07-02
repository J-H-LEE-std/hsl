/**
 * @file io.h
 * @brief Header file for define standard stream.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-11-28
 */

#ifndef HSL_IO_H
#define HSL_IO_H

#include <iostream>

namespace hsl {
    /**
     * @brief Define out stream commaonly used in HS-L.
     * For CLI, hsl::cout is connected to std::cout.
     * For GUI, hsl::cout is connected to hslgui::Bridge::cout.
     */
    extern std::ostream& cout;
}

#endif // HSL_IO_H
