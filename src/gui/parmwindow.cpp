#include "parmwindow.h"
#include <wx/filedlg.h>
#include <wx/file.h>
namespace hslgui {
ParmWindow::ParmWindow(wxWindow* parent, const ParamStruct& current)
: wxDialog(parent, wxID_ANY, "Parameter Option", wxDefaultPosition, wxSize(360,310)), params_(current){
    BuildUI(); BindEvents(); Centre();
}
void ParmWindow::BuildUI(){
    auto* root=new wxBoxSizer(wxVERTICAL);
    auto row=[&](const char* label, wxTextCtrl*& out, const wxString& init){
        auto* s=new wxBoxSizer(wxHORIZONTAL);
        s->Add(new wxStaticText(this, wxID_ANY, label, wxDefaultPosition, wxSize(70,-1)),0,wxALL,5);
        out=new wxTextCtrl(this, wxID_ANY, init); s->Add(out,1,wxALL|wxEXPAND,5); root->Add(s,0,wxEXPAND,0);
    };
    row("HMS",tHMS,wxString::Format("%d",params_.HMS));
    row("HMCR",tHMCR,wxString::Format("%g",params_.HMCR));
    row("PAR",tPAR,wxString::Format("%g",params_.PAR));
    row("MaxImp",tMaxImp,wxString::Format("%u",params_.MaxImp));
    row("N_Seg",tNSeg,wxString::Format("%d",params_.N_Seg));
    auto* btns=new wxBoxSizer(wxHORIZONTAL);
    btns->Add(new wxButton(this, wxID_OK, "OK"),0,wxALL,5);
    btns->Add(new wxButton(this, wxID_ANY,"Reset"),0,wxALL,5);
    btns->Add(new wxButton(this, wxID_HELP,"Help"),0,wxALL,5);
    btns->Add(new wxButton(this, wxID_SAVE,"Save"),0,wxALL,5);
    root->Add(btns,0,wxEXPAND,0); SetSizerAndFit(root);
}
void ParmWindow::BindEvents(){
    Bind(wxEVT_BUTTON,&ParmWindow::OnOK,this,wxID_OK);
    if(auto* reset=FindWindowByLabel("Reset")) reset->Bind(wxEVT_BUTTON,&ParmWindow::OnReset,this);
    Bind(wxEVT_BUTTON,&ParmWindow::OnHelp,this,wxID_HELP);
    Bind(wxEVT_BUTTON,&ParmWindow::OnSave,this,wxID_SAVE);
}
void ParmWindow::OnOK(wxCommandEvent&){
    try{
        params_.HMS=std::stoi(tHMS->GetValue().ToStdString());
        params_.HMCR=std::stod(tHMCR->GetValue().ToStdString());
        params_.PAR=std::stod(tPAR->GetValue().ToStdString());
        params_.MaxImp=(unsigned int)std::stoul(tMaxImp->GetValue().ToStdString());
        params_.N_Seg=std::stoi(tNSeg->GetValue().ToStdString());
        EndModal(wxID_OK);
    }catch(...){ wxMessageBox("Please enter valid numeric values.","HS-L",wxICON_ERROR); }
}
void ParmWindow::OnReset(wxCommandEvent&){
    params_=ParamStruct{};
    tHMS->ChangeValue(wxString::Format("%d",params_.HMS));
    tHMCR->ChangeValue(wxString::Format("%g",params_.HMCR));
    tPAR->ChangeValue(wxString::Format("%g",params_.PAR));
    tMaxImp->ChangeValue(wxString::Format("%u",params_.MaxImp));
    tNSeg->ChangeValue(wxString::Format("%d",params_.N_Seg));
}
void ParmWindow::OnHelp(wxCommandEvent&) {
    wxDialog dlg(this, wxID_ANY, "Parameter Help", wxDefaultPosition, wxSize(500, 550));

    wxScrolledWindow* panel = new wxScrolledWindow(&dlg, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
    panel->SetScrollRate(10, 10);

    auto* contentSizer = new wxBoxSizer(wxVERTICAL);

    auto addSection = [&](const wxString& title, const wxString& desc) {
        auto* box = new wxStaticBoxSizer(wxVERTICAL, panel, title);
        auto* text = new wxStaticText(box->GetStaticBox(), wxID_ANY, desc, wxDefaultPosition, wxSize(550, -1), wxTE_MULTILINE);
        text->Wrap(540);
        box->Add(text, 0, wxALL | wxEXPAND, 8);
        contentSizer->Add(box, 0, wxALL | wxEXPAND, 10);
    };

    addSection("HMS (Harmony Memory Size)",
        "The number of solution vectors stored in the harmony memory.\n"
        "Represents the population size of the algorithm.\n"
        "Enter a positive integer. (Default: 30)");

    addSection("HMCR (Harmony Memory Considering Rate)",
        "The probability of selecting decision variable values from existing harmonies.\n"
        "Determines how much the algorithm relies on past solutions.\n"
        "Enter a floating-point value between 0 and 1. (Default: 0.95)");

    addSection("PAR (Pitch Adjusting Rate)",
        "The probability of slightly adjusting the chosen value (fine-tuning step).\n"
        "Higher values increase local exploitation, while lower values favor exploration.\n"
        "Enter a positive floating-point value. (Default: 0.7)");

    addSection("MaxImp (Maximum Improvisations)",
        "The maximum number of improvisations (iterations) to be performed.\n"
        "Defines when the optimization process will terminate.\n"
        "Enter a positive integer. (Default: 30000)");

    addSection("N_Seg (Number of Segments)",
        "The number of segments used to discretize variable ranges.\n"
        "Higher values improve precision but may increase computation time.\n"
        "Enter a positive integer. (Default: 300)");

    panel->SetSizer(contentSizer);
    panel->FitInside();

    auto* okButton = new wxButton(&dlg, wxID_OK, "OK");

    auto* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(panel, 1, wxEXPAND | wxALL, 10);
    mainSizer->Add(okButton, 0, wxALIGN_CENTER | wxALL, 10);

    dlg.SetSizer(mainSizer);
    dlg.SetMinSize(wxSize(500, 570));
    dlg.SetSize(wxSize(500, 570));
    dlg.Centre();
    dlg.ShowModal();
}



void ParmWindow::OnSave(wxCommandEvent&){
    wxFileDialog dlg(this,"Save Parameter","","","HS-L Parameter (*.hsparm)|*.hsparm|All files (*.*)|*.*",
        wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if(dlg.ShowModal()!=wxID_OK) return;
    wxFile f(dlg.GetPath(), wxFile::write);
    f.Write(wxString::FromUTF8(params_.ToCSV().c_str())); f.Close();
}
} // namespace hslgui
