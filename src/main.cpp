/**
 * @file main.cpp
 * @brief Main enter point for GUI program.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-10-05
 */

#include <wx/wx.h>
#include "gui/mainwindow.h"

class HSApp : public wxApp {
public:
    virtual bool OnInit() override {
        auto* frame = new hslgui::MainWindow();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(HSApp); // CLI program is processed separately in climain.cpp.
