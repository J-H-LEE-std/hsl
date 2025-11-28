#include "io.h"
#include "../gui/bridge.h"

namespace hsl {
    // GUI 빌드: GUI 로그창(Bridge::cout)으로 출력
    std::ostream& cout = hslgui::Bridge::cout;
}
