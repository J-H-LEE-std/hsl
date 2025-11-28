#ifndef HSL_GUI_MAINWINDOW_H
#define HSL_GUI_MAINWINDOW_H
#include <wx/wx.h>
#include <wx/textctrl.h>
#include <wx/filedlg.h>
#include <wx/file.h>
#include <wx/hyperlink.h>
    #ifdef __APPLE__
    #include <wx/generic/filedlgg.h>
    #endif
#include "paramstruct.h"
#include "bridge.h"
#include "parmwindow.h"
namespace hslgui {
class MainWindow : public wxFrame {
public: MainWindow();
private:
    wxTextCtrl* textCode=nullptr; wxTextCtrl* textLog=nullptr; wxTextCtrl* textResult=nullptr;
    wxButton* btnOpen=nullptr; wxButton* btnSave=nullptr; wxButton* btnOpt=nullptr; wxButton* btnRun=nullptr;
    wxCheckBox* ckboxSeed=nullptr; wxTextCtrl* textSeed=nullptr;
    ParamStruct params; wxString currentHsPath;
    //GUI 요소 정의

    void BuildUI(); void BindEvents(); void AppendLog(const wxString&); void SetStatusLine(const wxString&); void EnableRunUI(bool);
    void OnOpen(wxCommandEvent&); void OnSave(wxCommandEvent&); void OnOption(wxCommandEvent&); void OnRun(wxCommandEvent&);
    void OnExportLog(wxCommandEvent&); void OnNew(wxCommandEvent&); void OnImportParam(wxCommandEvent&);
    void OnExportParam(wxCommandEvent&); void OnHelp(wxCommandEvent&); void OnAbout(wxCommandEvent&); void OnQuit(wxCommandEvent&);
    void OnToggleSeed(wxCommandEvent&);
};
}
#endif
