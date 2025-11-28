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

wxIMPLEMENT_APP(HSApp); //cli 프로그램은 따로 climain에서 처리
