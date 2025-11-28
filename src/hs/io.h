#ifndef HSL_IO_H
#define HSL_IO_H

#include <iostream>

namespace hsl {

    /**
     * @brief HS-L 공용 출력 스트림.
     *
     * - CLI 빌드에서는 std::cout으로 연결된다.
     * - GUI 빌드에서는 hslgui::Bridge::cout으로 연결된다.
     *
     * 사용 예:
     *   hsl::cout << "[INFO] Optimization started..." << std::endl;
     */
    extern std::ostream& cout;

} // namespace hsl

#endif // HSL_IO_H
