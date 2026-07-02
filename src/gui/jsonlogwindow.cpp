/**
 * @file jsonlogwindow.cpp
 * @brief Implementation of the JsonLogWindow class for logging HS optimization.
 * @author Lee Jaehyeong(J-H-LEE-std)
 * @date 2026-02-24
 */

#include "jsonlogwindow.h"
#include <limits>
#include <wx/config.h>

namespace hslgui {
    // Code snippet for setting json log mode.
    namespace {
        constexpr const char* kConfigRoot = "/JsonLog";

        int ModeToIndex(hsl::JsonMode mode) {
            switch (mode) {
                case hsl::JsonMode::Summary: return 0;
                case hsl::JsonMode::Snapshot: return 1;
                case hsl::JsonMode::Full: return 2;
            }
            return 0;
        }

        hsl::JsonMode IndexToMode(int index) {
            switch (index) {
                case 1: return hsl::JsonMode::Snapshot;
                case 2: return hsl::JsonMode::Full;
                default: return hsl::JsonMode::Summary;
            }
        }
    }

    /**
    * @brief Loads JSON logging configurations from the system registry or config file.
    * @return A JsonLogSettings structure populated with values retrieved via wxConfig.
    * @details Retrieves various logging parameters such as stride, buffer size, and precision
    * using a predefined configuration root path. If a setting is missing, it retains
    * the default value defined in the JsonLogSettings constructor.
    */
    JsonLogSettings LoadJsonLogSettings() {
        JsonLogSettings settings;
        wxConfig config("hsl_gui");
        const wxString root = wxString::FromUTF8(kConfigRoot);

        bool enable_logging = settings.enable_logging;
        if (config.Read(root + "/EnableLogging", &enable_logging)) {
            settings.enable_logging = enable_logging;
        }

        bool enable_txt = settings.enable_txt;
        if (config.Read(root + "/EnableTxt", &enable_txt)) {
            settings.enable_txt = enable_txt;
        }

        bool enable_csv = settings.enable_csv;
        if (config.Read(root + "/EnableCsv", &enable_csv)) {
            settings.enable_csv = enable_csv;
        }

        bool enable_jsonl = settings.enable_jsonl;
        if (config.Read(root + "/EnableJsonl", &enable_jsonl)) {
            settings.enable_jsonl = enable_jsonl;
        }

        long mode = ModeToIndex(settings.json_mode);
        if (config.Read(root + "/JsonMode", &mode)) {
            settings.json_mode = IndexToMode(static_cast<int>(mode));
        }

        long json_stride = settings.json_stride;
        if (config.Read(root + "/JsonStride", &json_stride)) {
            settings.json_stride = static_cast<int>(json_stride);
        }

        long json_flush = settings.json_flush_every;
        if (config.Read(root + "/JsonFlushEvery", &json_flush)) {
            settings.json_flush_every = static_cast<int>(json_flush);
        }

        long json_buffer = static_cast<long>(settings.json_buffer_bytes);
        if (config.Read(root + "/JsonBufferBytes", &json_buffer)) {
            settings.json_buffer_bytes = static_cast<std::size_t>(json_buffer);
        }

        bool reopen_on_flush = settings.reopen_on_flush;
        if (config.Read(root + "/ReopenOnFlush", &reopen_on_flush)) {
            settings.reopen_on_flush = reopen_on_flush;
        }

        long json_precision = settings.json_precision;
        if (config.Read(root + "/JsonPrecision", &json_precision)) {
            settings.json_precision = static_cast<int>(json_precision);
        }

        bool quiet = settings.quiet;
        if (config.Read(root + "/Quiet", &quiet)) {
            settings.quiet = quiet;
        }

        return settings;
    }

    /**
    * @brief Persists the current JSON logging settings to the system registry or config file.
    * @param settings The JsonLogSettings object containing the values to be saved.
    * @details Writes all logging configurations (EnableJsonl, JsonMode, Precision, etc.)
    * to the persistent storage and calls Flush() to ensure immediate synchronization with the disk.
    */
    void SaveJsonLogSettings(const JsonLogSettings& settings) {
        wxConfig config("hsl_gui");
        const wxString root = wxString::FromUTF8(kConfigRoot);
        config.Write(root + "/EnableLogging", settings.enable_logging);
        config.Write(root + "/EnableTxt", settings.enable_txt);
        config.Write(root + "/EnableCsv", settings.enable_csv);
        config.Write(root + "/EnableJsonl", settings.enable_jsonl);
        config.Write(root + "/JsonMode", ModeToIndex(settings.json_mode));
        config.Write(root + "/JsonStride", static_cast<long>(settings.json_stride));
        config.Write(root + "/JsonFlushEvery", static_cast<long>(settings.json_flush_every));
        config.Write(root + "/JsonBufferBytes", static_cast<long>(settings.json_buffer_bytes));
        config.Write(root + "/ReopenOnFlush", settings.reopen_on_flush);
        config.Write(root + "/JsonPrecision", static_cast<long>(settings.json_precision));
        config.Write(root + "/Quiet", settings.quiet);
        config.Flush();
    }

    JsonLogWindow::JsonLogWindow(wxWindow* parent, JsonLogSettings& settings)
        : wxDialog(parent, wxID_ANY, "JSON Log Option", wxDefaultPosition, wxSize(420, 420)),
        target_(settings),
        settings_(settings) {
        BuildUI();
        BindEvents();
        LoadFromSettings();
        UpdateControlState();
        Centre();
    }

    /**
     * @brief Core UI construction logic.
     */
    void JsonLogWindow::BuildUI() {
        auto* root = new wxBoxSizer(wxVERTICAL);

        chkEnableLogging_ = new wxCheckBox(this, wxID_ANY, "Enable logging");
        root->Add(chkEnableLogging_, 0, wxALL, 8);

        chkEnableTxt_ = new wxCheckBox(this, wxID_ANY, "Enable text logging (.txt)");
        root->Add(chkEnableTxt_, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);

        chkEnableCsv_ = new wxCheckBox(this, wxID_ANY, "Enable CSV logging");
        root->Add(chkEnableCsv_, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);

        chkEnableJsonl_ = new wxCheckBox(this, wxID_ANY, "Enable JSONL logging");
        root->Add(chkEnableJsonl_, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);

        wxArrayString modeChoices;
        modeChoices.Add("Summary");
        modeChoices.Add("Snapshot");
        modeChoices.Add("Full");
        radioMode_ = new wxRadioBox(this, wxID_ANY, "JSON Mode", wxDefaultPosition, wxDefaultSize, modeChoices, 1);
        root->Add(radioMode_, 0, wxALL | wxEXPAND, 8);

        auto* grid = new wxFlexGridSizer(2, 6, 8);
        grid->AddGrowableCol(1, 1);

        grid->Add(new wxStaticText(this, wxID_ANY, "Snapshot stride"), 0, wxALIGN_CENTER_VERTICAL);
        spinStride_ = new wxSpinCtrl(this, wxID_ANY);
        spinStride_->SetRange(0, std::numeric_limits<int>::max());
        grid->Add(spinStride_, 1, wxEXPAND);

        grid->Add(new wxStaticText(this, wxID_ANY, "Flush every (iter)"), 0, wxALIGN_CENTER_VERTICAL);
        spinFlushEvery_ = new wxSpinCtrl(this, wxID_ANY);
        spinFlushEvery_->SetRange(0, std::numeric_limits<int>::max());
        grid->Add(spinFlushEvery_, 1, wxEXPAND);

        grid->Add(new wxStaticText(this, wxID_ANY, "Buffer bytes"), 0, wxALIGN_CENTER_VERTICAL);
        spinBufferBytes_ = new wxSpinCtrl(this, wxID_ANY);
        spinBufferBytes_->SetRange(0, std::numeric_limits<int>::max());
        grid->Add(spinBufferBytes_, 1, wxEXPAND);

        grid->Add(new wxStaticText(this, wxID_ANY, "JSON precision"), 0, wxALIGN_CENTER_VERTICAL);
        spinPrecision_ = new wxSpinCtrl(this, wxID_ANY);
        spinPrecision_->SetRange(0, 15);
        grid->Add(spinPrecision_, 1, wxEXPAND);

        root->Add(grid, 0, wxALL | wxEXPAND, 8);

        chkReopen_ = new wxCheckBox(this, wxID_ANY, "Reopen file on each flush");
        root->Add(chkReopen_, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);

        chkQuiet_ = new wxCheckBox(this, wxID_ANY, "Quiet text log (suppress iteration summaries)");
        root->Add(chkQuiet_, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);

        auto* note = new wxStaticText(
            this,
            wxID_ANY,
            "Note: GUI console shows NEW BEST/status lines only.\n"
            "For full iteration output, enable text logs.");
        root->Add(note, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);

        auto* buttons = CreateSeparatedButtonSizer(wxOK | wxCANCEL | wxAPPLY);
        if (buttons) root->Add(buttons, 0, wxALL | wxEXPAND, 8);

        SetSizerAndFit(root);
    }

    /**
     * @brief Bind GUI components and actions.
     */
    void JsonLogWindow::BindEvents() {
        Bind(wxEVT_BUTTON, &JsonLogWindow::OnApply, this, wxID_APPLY);
        Bind(wxEVT_BUTTON, &JsonLogWindow::OnOK, this, wxID_OK);
        chkEnableLogging_->Bind(wxEVT_CHECKBOX, &JsonLogWindow::OnEnableLogging, this);
        chkEnableJsonl_->Bind(wxEVT_CHECKBOX, &JsonLogWindow::OnEnableJsonl, this);
        radioMode_->Bind(wxEVT_RADIOBOX, &JsonLogWindow::OnModeChanged, this);
    }

    // For the code below, implement all the events to be connected.
    void JsonLogWindow::LoadFromSettings() {
        chkEnableLogging_->SetValue(settings_.enable_logging);
        chkEnableTxt_->SetValue(settings_.enable_txt);
        chkEnableCsv_->SetValue(settings_.enable_csv);
        chkEnableJsonl_->SetValue(settings_.enable_jsonl);
        radioMode_->SetSelection(ModeToIndex(settings_.json_mode));
        spinStride_->SetValue(settings_.json_stride);
        spinFlushEvery_->SetValue(settings_.json_flush_every);
        spinBufferBytes_->SetValue(static_cast<int>(settings_.json_buffer_bytes));
        spinPrecision_->SetValue(settings_.json_precision);
        chkReopen_->SetValue(settings_.reopen_on_flush);
        chkQuiet_->SetValue(settings_.quiet);
    }

    void JsonLogWindow::UpdateControlState() {
        bool enable_logging = chkEnableLogging_->IsChecked();
        chkEnableTxt_->Enable(enable_logging);
        chkEnableCsv_->Enable(enable_logging);
        chkEnableJsonl_->Enable(enable_logging);
        chkQuiet_->Enable(enable_logging);

        radioMode_->Enable(enable_logging);
        spinFlushEvery_->Enable(enable_logging);
        spinBufferBytes_->Enable(enable_logging);
        spinPrecision_->Enable(enable_logging);
        chkReopen_->Enable(enable_logging);

        bool snapshot_mode = (radioMode_->GetSelection() == 1);
        bool enable_json = enable_logging && chkEnableJsonl_->IsChecked();
        spinStride_->Enable(enable_json && snapshot_mode);
    }

    void JsonLogWindow::ApplySettings() {
        settings_.enable_logging = chkEnableLogging_->GetValue();
        settings_.enable_txt = chkEnableTxt_->GetValue();
        settings_.enable_csv = chkEnableCsv_->GetValue();
        settings_.enable_jsonl = chkEnableJsonl_->GetValue();
        settings_.json_mode = IndexToMode(radioMode_->GetSelection());
        settings_.json_stride = spinStride_->GetValue();
        settings_.json_flush_every = spinFlushEvery_->GetValue();
        settings_.json_buffer_bytes = static_cast<std::size_t>(spinBufferBytes_->GetValue());
        settings_.json_precision = spinPrecision_->GetValue();
        settings_.reopen_on_flush = chkReopen_->GetValue();
        settings_.quiet = chkQuiet_->GetValue();
        target_ = settings_;
        SaveJsonLogSettings(settings_);
    }

    void JsonLogWindow::OnApply(wxCommandEvent&) {
        ApplySettings();
    }

    void JsonLogWindow::OnOK(wxCommandEvent&) {
        ApplySettings();
        EndModal(wxID_OK);
    }

    void JsonLogWindow::OnModeChanged(wxCommandEvent&) {
        UpdateControlState();
    }

    void JsonLogWindow::OnEnableLogging(wxCommandEvent&) {
        UpdateControlState();
    }

    void JsonLogWindow::OnEnableJsonl(wxCommandEvent&) {
        UpdateControlState();
    }
}
