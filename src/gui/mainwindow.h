/**
 * @file mainwindow.h
 * @brief Header of main window interface for the HS-L GUI application.
 * @details This class defines the primary layout, including the code editor,
 * log console, result display, and various control buttons for the HS-L solver.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-11-28
 */

#ifndef HSL_GUI_MAINWINDOW_H
#define HSL_GUI_MAINWINDOW_H

#include <wx/wx.h>
#include <wx/textctrl.h>
#include <wx/filedlg.h>
#include <wx/file.h>
#include <wx/hyperlink.h>
    #ifdef __APPLE__
    #include <wx/generic/filedlgg.h> // macOS cannot render filedlg correctly, so use filedlgg instead.
    #endif
#include "paramstruct.h"
#include "jsonlogsettings.h"
#include "bridge.h"
#include "parmwindow.h"

namespace hslgui {
    class MainWindow : public wxFrame {
    public:
        MainWindow();

    private:
        // GUI components definitions.
        wxTextCtrl* textCode=nullptr; wxTextCtrl* textLog=nullptr; wxTextCtrl* textResult=nullptr;
        wxButton* btnOpen=nullptr; wxButton* btnSave=nullptr; wxButton* btnOpt=nullptr; wxButton* btnRun=nullptr;
        wxCheckBox* ckboxSeed=nullptr; wxTextCtrl* textSeed=nullptr;
        ParamStruct params; wxString currentHsPath;
        JsonLogSettings json_settings_;

        void BuildUI(); void BindEvents(); void AppendLog(const wxString&); void SetStatusLine(const wxString&); void EnableRunUI(bool);
        void OnOpen(wxCommandEvent&); void OnSave(wxCommandEvent&); void OnOption(wxCommandEvent&); void OnRun(wxCommandEvent&);
        void OnLogOption(wxCommandEvent&);
        void OnExportLog(wxCommandEvent&); void OnNew(wxCommandEvent&); void OnImportParam(wxCommandEvent&);
        void OnExportParam(wxCommandEvent&); void OnHelp(wxCommandEvent&); void OnAbout(wxCommandEvent&); void OnQuit(wxCommandEvent&);
        void OnToggleSeed(wxCommandEvent&);
    };
}

#endif
