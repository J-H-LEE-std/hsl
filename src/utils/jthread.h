/**
 * @file jthread.h
 * @brief Header file for jthread implemantation for macOS compile environment.
 * @note Whidows and Linux compiler used std::jthread as jthread.
 * This header will be deprecated when Apple Clang support std::jthread.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-11-28
 */

#ifndef HSL_JTHREAD_
#define HSL_JTHREAD_

#include <thread>
#include <utility>

namespace hsl {
    #if defined(__cpp_lib_jthread) && (__cpp_lib_jthread >= 201911L)
        using jthread = std::jthread;
        // If std::jthred defined, use std::jthread.
    #else
    class jthread {
        private:
            std::thread t;

        public:
            jthread() noexcept = default;

            template <typename F, typename... Args>
            explicit jthread(F&& f, Args&&... args)
                : t(std::forward<F>(f), std::forward<Args>(args)...)
            {}

            ~jthread() {
                if (t.joinable()) {
                    t.join(); // Automatically call join when exiting the scope to prevent resource leaks.
                }
            }

            jthread(const jthread&) = delete;
            jthread& operator=(const jthread&) = delete;

            jthread(jthread&& other) noexcept
                : t(std::move(other.t))
            {}

            jthread& operator=(jthread&& other) noexcept {
                if (this != &other) {
                    /* If the thread owned by the current object is in a joinable state, force a join.
                    It works as similar as std::jthread behavior.
                    */
                    if (t.joinable()) {
                        t.join();
                    }
                    t = std::move(other.t);
                }
                return *this;
            }

            bool joinable() const noexcept { return t.joinable(); }
            void join() {
                if (t.joinable()) t.join();
            }
            void detach() { t.detach(); }
            std::thread::id get_id() const noexcept { return t.get_id(); }
            std::thread::native_handle_type native_handle() {
                return t.native_handle();
            }
    };

    #endif
}

#endif
