/**
 * @file paramwindow.h
 * @brief Header of parameter window interface for the HS-L GUI application.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2025-11-28
 */

#ifndef HSL_GUI_PARMWINDOW_H
#define HSL_GUI_PARMWINDOW_H

#include <wx/wx.h>
#include "paramstruct.h"

namespace hslgui {
    class ParmWindow : public wxDialog {
    public:
        explicit ParmWindow(wxWindow* parent, const ParamStruct& current);
        ParamStruct GetParams() const { return params_; }
    private:
        ParamStruct params_;
        wxTextCtrl *tHMS, *tHMCR, *tPAR, *tMaxImp, *tNSeg;
        void BuildUI(); void BindEvents();
        void OnOK(wxCommandEvent&); void OnReset(wxCommandEvent&);
        void OnHelp(wxCommandEvent&); void OnSave(wxCommandEvent&);
    };
}

#endif
