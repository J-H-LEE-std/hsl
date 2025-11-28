#include "mainwindow.h"
#include "../utils/jthread.h"
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/valnum.h>
#include <thread>
#include <ctime>
#include <sstream>
#include <random>

namespace hslgui {

static wxString now_hhmmss(){
    std::time_t t=std::time(nullptr); std::tm tm;
#ifdef _WIN32
    localtime_s(&tm,&t);
#else
    localtime_r(&t,&tm);
#endif
    char buf[32]; std::strftime(buf,sizeof(buf),"%H:%M:%S",&tm);
    return wxString::FromUTF8(buf);
} //현재 시간은 이쪽에서 출력.

MainWindow::MainWindow()
: wxFrame(nullptr, wxID_ANY, "HS-L: Harmony Search Language", wxDefaultPosition, wxSize(920,580)) {
    BuildUI(); BindEvents(); SetStatusBar(new wxStatusBar(this, wxID_ANY)); SetStatusLine("Ready"); Centre();
}

void MainWindow::BuildUI(){
    auto* root=new wxBoxSizer(wxHORIZONTAL);

    auto* left=new wxBoxSizer(wxVERTICAL);
    btnOpen=new wxButton(this, wxID_OPEN, "Open Source");
    btnSave=new wxButton(this, wxID_SAVE, "Save Source");
    btnOpt =new wxButton(this, wxID_ANY,  "Option");
    btnRun =new wxButton(this, wxID_ANY,  "Run");
    left->Add(btnOpen,0,wxALL,5);
    left->Add(btnSave,0,wxALL,5);
    left->Add(btnOpt, 0,wxALL,5);
    left->Add(btnRun, 0,wxALL,5);
    ckboxSeed = new wxCheckBox(this, wxID_ANY, "Use Seed");
    textSeed  = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    textSeed->SetValidator(wxIntegerValidator<unsigned int>());
    left->AddSpacer(10);
    left->Add(ckboxSeed, 0, wxLEFT|wxRIGHT|wxTOP, 8);
    left->Add(textSeed,  0, wxEXPAND|wxALL, 5);
    //좌단 컨트롤박스 부분


    root->Add(left,0,wxEXPAND,0);

    auto* codeBox=new wxStaticBoxSizer(wxVERTICAL,this,"Source Code");
    textCode=new wxTextCtrl(codeBox->GetStaticBox(), wxID_ANY, "", wxDefaultPosition, wxSize(480,480),
                            wxTE_MULTILINE|wxTE_RICH2|wxVSCROLL);
    codeBox->Add(textCode,1,wxALL|wxEXPAND,5);
    root->Add(codeBox,1,wxEXPAND,5);
    //중간의 소스코드 입력창

    auto* right=new wxBoxSizer(wxVERTICAL);
    auto* logBox=new wxStaticBoxSizer(wxVERTICAL,this,"Execution Log");
    textLog=new wxTextCtrl(logBox->GetStaticBox(), wxID_ANY, "", wxDefaultPosition, wxSize(240,200),
                           wxTE_MULTILINE|wxTE_READONLY|wxVSCROLL);
    logBox->Add(textLog,1,wxALL|wxEXPAND,5); right->Add(logBox,1,wxEXPAND,5);

    auto* resBox=new wxStaticBoxSizer(wxVERTICAL,this,"Result");
    textResult=new wxTextCtrl(resBox->GetStaticBox(), wxID_ANY, "", wxDefaultPosition, wxSize(240,240),
                              wxTE_MULTILINE|wxTE_READONLY|wxVSCROLL);
    resBox->Add(textResult,1,wxALL|wxEXPAND,5); right->Add(resBox,1,wxEXPAND,5);
    //우단의 로그와 결과창. 즉, 출력 담당

    root->Add(right,1,wxEXPAND,5); SetSizerAndFit(root);

    auto* mbar=new wxMenuBar();
    auto* file=new wxMenu();
    file->Append(wxID_NEW,"New");
    file->Append(wxID_OPEN,"Import Source (.hs)");
    file->Append(2001,"Import Parameter (.hsparm)");
    file->AppendSeparator();
    file->Append(wxID_SAVE,"Export Source (.hs)");
    file->Append(2002,"Export Parameter (.hsparm)");
    file->AppendSeparator();
    file->Append(2003,"Export Log");
    file->AppendSeparator();
    file->Append(wxID_EXIT,"Quit");
    mbar->Append(file,"File");

    auto* run=new wxMenu(); run->Append(3001,"Run"); run->Append(3002,"Option"); mbar->Append(run,"Run");
    auto* help=new wxMenu(); help->Append(wxID_HELP,"Help"); help->Append(wxID_ABOUT,"About"); mbar->Append(help,"Help");
    SetMenuBar(mbar);
    // 메뉴바 생성 코드
}

void MainWindow::BindEvents(){
    btnOpen->Bind(wxEVT_BUTTON,&MainWindow::OnOpen,this);
    btnSave->Bind(wxEVT_BUTTON,&MainWindow::OnSave,this);
    btnOpt ->Bind(wxEVT_BUTTON,&MainWindow::OnOption,this);
    btnRun ->Bind(wxEVT_BUTTON,&MainWindow::OnRun,this);
    ckboxSeed->Bind(wxEVT_CHECKBOX, &MainWindow::OnToggleSeed, this);

    Bind(wxEVT_MENU,&MainWindow::OnNew,this,wxID_NEW);
    Bind(wxEVT_MENU,&MainWindow::OnOpen,this,wxID_OPEN);
    Bind(wxEVT_MENU,&MainWindow::OnSave,this,wxID_SAVE);
    Bind(wxEVT_MENU,&MainWindow::OnImportParam,this,2001);
    Bind(wxEVT_MENU,&MainWindow::OnExportParam,this,2002);
    Bind(wxEVT_MENU,&MainWindow::OnExportLog,this,2003);
    Bind(wxEVT_MENU,&MainWindow::OnQuit,this,wxID_EXIT);
    Bind(wxEVT_MENU,&MainWindow::OnRun,this,3001);
    Bind(wxEVT_MENU,&MainWindow::OnOption,this,3002);
    Bind(wxEVT_MENU,&MainWindow::OnHelp,this,wxID_HELP);
    Bind(wxEVT_MENU,&MainWindow::OnAbout,this,wxID_ABOUT);
} //동작과 이벤트 연결

void MainWindow::AppendLog(const wxString& line){ textLog->AppendText(line+"\n"); }

void MainWindow::SetStatusLine(const wxString& s){
    if(GetStatusBar()) GetStatusBar()->SetStatusText(s);
    hslgui::Bridge::cout << "[Status] " << std::string(s.mb_str()) << std::endl;
} //하단 상태바 생성 코드

void MainWindow::EnableRunUI(bool enable){ btnRun->Enable(enable); }

//이하 코드에 대해서, 모두 연결될 이벤트

void MainWindow::OnOpen(wxCommandEvent&){
    #ifdef __APPLE__
        wxGenericFileDialog dlg(this, "Open HS-L", "", "",
        "HS-L files (*.hs)|*.hs|Text files (*.txt)|*.txt|All files (*.*)|*.*",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    #else
        wxFileDialog dlg(this, "Open HS-L", "", "",
        "HS-L files (*.hs)|*.hs|Text files (*.txt)|*.txt|All files (*.*)|*.*",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    #endif
    if(dlg.ShowModal()!=wxID_OK) {
        return;
    }
    currentHsPath=dlg.GetPath();
    wxString content; wxFile file(currentHsPath); file.ReadAll(&content);
    textCode->ChangeValue(content);
    SetStatusLine(wxString::Format("File %s imported.", dlg.GetFilename()));
}

void MainWindow::OnSave(wxCommandEvent&){
    #ifdef __APPLE__
        wxGenericFileDialog dlg(this,"Save HS-L","","","HS-L files (*.hs)|*.hs|All files (*.*)|*.*",
        wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    #else
        wxFileDialog dlg(this,"Save HS-L","","","HS-L files (*.hs)|*.hs|All files (*.*)|*.*",
        wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    #endif
    
    if(dlg.ShowModal()!=wxID_OK) return;
    wxFile file(dlg.GetPath(), wxFile::write); file.Write(textCode->GetValue()); file.Close();
    SetStatusLine(wxString::Format("File %s saved.", dlg.GetFilename()));
}

void MainWindow::OnOption(wxCommandEvent&){
    ParmWindow dlg(this, params);
    if(dlg.ShowModal()==wxID_OK){ params=dlg.GetParams(); SetStatusLine("Parameters updated."); }
}

void MainWindow::OnRun(wxCommandEvent&){
    EnableRunUI(false); // 만약 여기서 Run을 막지 않으면 쓸데없이 중복 실행이 되어 job을 block하는 일이 벌어지게 됨.
    AppendLog(wxString::Format("[%s] HS-L Start Running....", now_hhmmss()));
    SetStatusLine("HS-L Running...");

    const std::string src = textCode->GetValue().ToStdString();
    auto param = params;

    unsigned int seed;
    if (ckboxSeed->IsChecked()) {
        wxString seedStr = textSeed->GetValue().Trim(true).Trim(false);
        unsigned long tmp = 0;

        if (!seedStr.ToULong(&tmp)) {
            AppendLog("Seed parse failed, using random_device");
            seed = std::random_device{}();
        } else {
            seed = static_cast<unsigned int>(tmp);
            AppendLog(wxString::Format("Using fixed seed = %u", seed));
        }
    } else {
        seed = std::random_device{}();
        AppendLog(wxString::Format("Using auto seed = %u", seed));
    }
 // HS 코어에 시드 넘김
    AppendLog(wxString::Format("Current seed = %u", seed));

    (void) hsl::jthread([this, src, param, seed](){
        auto result = Bridge::Run(src, param, seed);
        wxTheApp->CallAfter([this, result](){
            if(!result.error_msg.empty()){
                textResult->ChangeValue(wxString::FromUTF8(("Error: "+result.error_msg).c_str()));
            }else{
                std::ostringstream oss;
                oss<<"Best value: "<<result.best_value<<"\n";
                for(auto& kv: result.variables) oss<<"  "<<kv.first<<" = "<<kv.second<<"\n";
                oss<<"CPU Time: "<<result.cpu_time<<" sec\n";
                textResult->ChangeValue(wxString::FromUTF8(oss.str().c_str()));
            }
            AppendLog(wxString::Format("[%s] Optimization Finished.", now_hhmmss()));
            SetStatusLine("Completed");
            EnableRunUI(true);
        }); //이 코드를 통해 HS-core의 작업은 새 스레드가 백그라운드로 실행, 그 후 결과를 GUI wrapper의 스레드가 취득해 갱신하게 됨.
    }); //HS-core는 새 스레드에서 백그라운드로 돌아가게 됨. 실행이 끝나면 결과값만 넘겨주고 소멸.
}

void MainWindow::OnToggleSeed(wxCommandEvent&){
    bool checked = ckboxSeed->IsChecked();
    textSeed->SetEditable(checked);
    textSeed->SetWindowStyleFlag(checked ? 0 : wxTE_READONLY);
    if(!checked) textSeed->Clear();
}

void MainWindow::OnExportLog(wxCommandEvent&){
    wxFileDialog dlg(this,"Export Log","","","Text files (*.txt)|*.txt|All files (*.*)|*.*",
        wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if(dlg.ShowModal()!=wxID_OK) return;
    wxFile f(dlg.GetPath(), wxFile::write);
    f.Write(textLog->GetValue()); f.Write("\n"); f.Write(textResult->GetValue()); f.Close();
    SetStatusLine(wxString::Format("File %s saved.", dlg.GetFilename()));
}

void MainWindow::OnNew(wxCommandEvent&){
    textCode->Clear(); textLog->Clear(); textResult->Clear(); currentHsPath.clear();
    SetStatusLine("New session started.");
}

void MainWindow::OnImportParam(wxCommandEvent&){
    wxFileDialog dlg(this,"Import Parameter","","","HS-L Parameter (*.hsparm)|*.hsparm|All files (*.*)|*.*",
        wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if(dlg.ShowModal()!=wxID_OK) return;
    wxString content; wxFile file(dlg.GetPath()); file.ReadAll(&content);
    params.FromCSV(std::string(content.mb_str()));
    SetStatusLine(wxString::Format("File %s imported.", dlg.GetFilename()));
}

void MainWindow::OnExportParam(wxCommandEvent&){
    wxFileDialog dlg(this,"Export Parameter","","","HS-L Parameter (*.hsparm)|*.hsparm|All files (*.*)|*.*",
        wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if(dlg.ShowModal()!=wxID_OK) return;
    wxFile file(dlg.GetPath(), wxFile::write);
    file.Write(wxString::FromUTF8(params.ToCSV().c_str())); file.Close();
    SetStatusLine(wxString::Format("File %s saved.", dlg.GetFilename()));
}

void MainWindow::OnHelp(wxCommandEvent&){ wxLaunchDefaultBrowser("https://github.com/J-H-LEE-std/hsl/wiki"); }

void MainWindow::OnAbout(wxCommandEvent&){
    wxDialog dlg(this, wxID_ANY, "About HS-L", wxDefaultPosition, wxDefaultSize);
    auto* s=new wxBoxSizer(wxVERTICAL);
    s->Add(new wxStaticText(&dlg, wxID_ANY, "HS-L: Harmony Search Language"),0,wxALL,5);
    s->Add(new wxStaticText(&dlg, wxID_ANY, "version: Prerelease-0.2.0"),0,wxALL,5);
    s->Add(new wxStaticText(&dlg, wxID_ANY, "Copyright (c) 2025, Jaehyeong Lee and Zong Woo Geem"),0,wxALL,5);
    s->Add(new wxHyperlinkCtrl(&dlg, wxID_ANY, "BSD 3-Clause License","https://github.com/J-H-LEE-std/hsl/blob/main/LICENSE"),0,wxALL,5);
    s->Add(new wxHyperlinkCtrl(&dlg, wxID_ANY, "Third-party software notice","https://github.com/J-H-LEE-std/hsl/blob/main/THIRD_PARTY_NOTICES.md"),0,wxALL,5);
    s->Add(new wxHyperlinkCtrl(&dlg, wxID_ANY, "Github Repository","https://github.com/J-H-LEE-std/hsl"),0,wxALL,5);
    s->Add(new wxButton(&dlg, wxID_OK, "OK"),0,wxALIGN_CENTER|wxALL,5);
    dlg.SetSizerAndFit(s); dlg.ShowModal();
}

void MainWindow::OnQuit(wxCommandEvent&){ Close(true); }

}
