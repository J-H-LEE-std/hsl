/**
 * @file io_gui.cpp
 * @brief Definate standard stream for GUI.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-11-28
 */

#include "io.h"
#include "../gui/bridge.h"

namespace hsl {
    // For CLI output, hslgui::Bridge::cout is used for GUI text elements.
    std::ostream& cout = hslgui::Bridge::cout;
}
