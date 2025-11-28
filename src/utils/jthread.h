#ifndef HSL_JTHREAD_
#define HSL_JTHREAD_
//Apple에서 지원하지 않는 jtherad를 사용하기 위한 Wrapper.

#include <thread>
#include <utility>

namespace hsl {

#if defined(__cpp_lib_jthread) && (__cpp_lib_jthread >= 201911L)
        using jthread = std::jthread;
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
            // 스코프를 벗어날 때 자동으로 join을 호출하여 리소스 누수 방지
            t.join();
        }
    }
    
    jthread(const jthread&) = delete;
    jthread& operator=(const jthread&) = delete;

    jthread(jthread&& other) noexcept
        : t(std::move(other.t))
    {}

    jthread& operator=(jthread&& other) noexcept {
        if (this != &other) {
            // 현재 객체가 소유한 스레드가 joinable 상태라면 강제로 join (std::jthread와 유사한 동작)
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

} // namespace hsl

#endif
