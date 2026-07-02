/**
 * @file timing.h
 * @brief Utility implemenatation counting CPU and wall time.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2026-02-24
 */

#include "timing.h"

#include <ctime>
#include <chrono>
#ifdef _WIN32
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif
    #include <windows.h>
#else
    #include <time.h>
#endif

namespace hsl {
    /**
    * @brief Retrieves the total CPU time consumed by the current process.
    * @return Total CPU time in seconds (sum of kernel and user mode time).
    * @details This function provides high-precision timing across different platforms:
    * Windows: Uses `GetProcessTimes` for 100-nanosecond precision.
    * POSIX (Linux/macOS): Uses `clock_gettime` with `CLOCK_PROCESS_CPUTIME_ID`.
    * Fallback: Uses standard `std::clock()` if specialized APIs are unavailable.
    */
    double get_process_cpu_time_sec() {
        #ifdef _WIN32
            FILETIME create_time;
            FILETIME exit_time;
            FILETIME kernel_time;
            FILETIME user_time;
            if (GetProcessTimes(GetCurrentProcess(), &create_time, &exit_time, &kernel_time, &user_time)) {
                ULARGE_INTEGER k;
                ULARGE_INTEGER u;
                k.LowPart = kernel_time.dwLowDateTime;
                k.HighPart = kernel_time.dwHighDateTime;
                u.LowPart = user_time.dwLowDateTime;
                u.HighPart = user_time.dwHighDateTime;
                unsigned long long total_100ns = k.QuadPart + u.QuadPart;
                return static_cast<double>(total_100ns) * 1e-7;
            }
        #endif

        #if defined(CLOCK_PROCESS_CPUTIME_ID)
            struct timespec ts;
            if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts) == 0) {
                return static_cast<double>(ts.tv_sec) + static_cast<double>(ts.tv_nsec) * 1e-9;
            }
        #endif

            return static_cast<double>(std::clock()) / static_cast<double>(CLOCKS_PER_SEC);
    }

    /**
    * @brief Retrieves the current wall-clock time using a steady clock.
    * @return Current time in seconds since the clock's epoch.
    * @details Uses `std::chrono::steady_clock` to ensure the time is monotonic,
    * making it ideal for measuring elapsed real-world intervals regardless of system clock updates.
    */
    double get_wall_time_sec() {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration<double>(now.time_since_epoch()).count();
    }
}
