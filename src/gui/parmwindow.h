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
