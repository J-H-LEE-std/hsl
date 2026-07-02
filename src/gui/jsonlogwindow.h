/**
 * @file jsonlogwindow.cpp
 * @brief Header of the JsonLogWindow class for logging HS optimization.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2026-02-24
 */

#ifndef HSL_GUI_JSONLOGWINDOW_H
#define HSL_GUI_JSONLOGWINDOW_H

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include "jsonlogsettings.h"

namespace hslgui {
    class JsonLogWindow : public wxDialog {
        public:
            JsonLogWindow(wxWindow* parent, JsonLogSettings& settings);

        private:
            JsonLogSettings& target_;
            JsonLogSettings settings_;

            wxCheckBox* chkEnableLogging_ = nullptr;
            wxCheckBox* chkEnableTxt_ = nullptr;
            wxCheckBox* chkEnableCsv_ = nullptr;
            wxCheckBox* chkEnableJsonl_ = nullptr;
            wxRadioBox* radioMode_ = nullptr;
            wxSpinCtrl* spinStride_ = nullptr;
            wxSpinCtrl* spinFlushEvery_ = nullptr;
            wxSpinCtrl* spinBufferBytes_ = nullptr;
            wxSpinCtrl* spinPrecision_ = nullptr;
            wxCheckBox* chkReopen_ = nullptr;
            wxCheckBox* chkQuiet_ = nullptr;

            void BuildUI();
            void BindEvents();
            void LoadFromSettings();
            void UpdateControlState();
            void ApplySettings();

            void OnApply(wxCommandEvent& evt);
            void OnOK(wxCommandEvent& evt);
            void OnModeChanged(wxCommandEvent& evt);
            void OnEnableLogging(wxCommandEvent& evt);
            void OnEnableJsonl(wxCommandEvent& evt);
    };
}

#endif
