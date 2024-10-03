#include "wx/wxprec.h"
#include "srt_server.h"
#include "gst_util.h"
#include <iostream>
#include <codecvt>
#include <locale.h>

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifndef wxHAS_IMAGES_IN_RESOURCES
#include "../sample.xpm"
#endif

#define API "wasapi"


struct Config* config = (struct Config*)malloc(sizeof(struct Config));

class MyApp : public wxApp
{
public:
    virtual bool OnInit() wxOVERRIDE;
};

class MyFrame : public wxFrame
{
public:
    MyFrame(const wxString& title);

    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnStart(wxCommandEvent& event); // Adicionado
    void OnStop(wxCommandEvent& event);

private:

    wxBoxSizer* sizer;
    wxButton* startButton;
    wxButton* stopButton;
    wxComboBox* comboBoxDevices;
    wxComboBox* comboBoxPort;
    wxTextCtrl* portTextCtrl;
    wxStaticText* labelComboBoxDevices;
    wxStaticText* labelComboBoxPort;
    wxStaticText* labelPortTextCtrl;
    wxDECLARE_EVENT_TABLE();
};

enum
{
    Minimal_Quit = wxID_EXIT,
    Minimal_About = wxID_ABOUT,
    Minimal_Start = wxID_HIGHEST + 1,  // Adicionado
    Minimal_Stop = wxID_HIGHEST + 2
};

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_MENU(Minimal_Quit, MyFrame::OnQuit)
EVT_MENU(Minimal_About, MyFrame::OnAbout)
EVT_BUTTON(Minimal_Start, MyFrame::OnStart)  // Adicionado
EVT_BUTTON(Minimal_Stop, MyFrame::OnStop)
wxEND_EVENT_TABLE()

//wxIMPLEMENT_APP(MyApp);
wxIMPLEMENT_APP_CONSOLE(MyApp);


bool MyApp::OnInit()
{
    if (!wxApp::OnInit())
        return false;

    setlocale(LC_ALL, "");

    MyFrame* frame = new MyFrame("SRT SERVER");
    frame->Show(true);

    return true;
}

MyFrame::MyFrame(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title)
{
    SetIcon(wxICON(sample));
    int size1 = 20, size2 = 100;
    char** list = (char**)malloc(size1 * sizeof(char*));

    this->sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSize(300, 310);

    this->startButton = new wxButton(this, Minimal_Start, wxT("Start"), wxDefaultPosition, wxDefaultSize, 0);
    this->startButton->SetToolTip(wxT("Clique aqui para iniciar o srt_server"));
    this->stopButton = new wxButton(this, Minimal_Stop, wxT("Stop"), wxDefaultPosition, wxDefaultSize, 0);
    this->stopButton->SetToolTip(wxT("Clique aqui para parar o srt_server"));
    this->comboBoxDevices = new wxComboBox(this, wxID_ANY);
    this->comboBoxPort = new wxComboBox(this, wxID_ANY);
    this->portTextCtrl = new wxTextCtrl(this, wxID_ANY);

    //Labels
    labelComboBoxDevices = new wxStaticText(this, wxID_ANY, wxT("Device"));
    labelComboBoxPort = new wxStaticText(this, wxID_ANY, wxT("Porta"));
    labelPortTextCtrl = new wxStaticText(this, wxID_ANY, wxT("Porta SRT"));

    if (list == NULL) {
        printf("Erro! Falha na alocação de memória.\n");
        return;
    }

    for (int i = 0; i < size1; i++) {
        //list[i] = (char*)malloc(size2 * sizeof(char));
        list[i] = NULL;
    }

    listar_dispositivos_audio(list, size1, API, size2);

    for (int i = 0; i < size1; i++) {
        if (list[i] == NULL) {
            break;
        }
        comboBoxDevices->Append(list[i]);
    }
    for (int i = 0; i < 10; i++) {
        comboBoxPort->Append(wxString::Format("COM%d", i));
    }

    this->sizer->Add(this->startButton, 0, wxALL, 5);
    this->sizer->Add(this->stopButton, 0, wxALL, 5);
    this->sizer->Add(this->labelComboBoxDevices, 0, wxALL, 5);
    this->sizer->Add(this->comboBoxDevices, 0, wxALL, 5);
    this->sizer->Add(this->labelComboBoxPort, 0, wxALL, 5);
    this->sizer->Add(this->comboBoxPort, 0, wxALL, 5);
    this->sizer->Add(this->labelPortTextCtrl, 0, wxALL, 5);
    this->sizer->Add(this->portTextCtrl, 0, wxALL, 5);

    this->SetSizer(this->sizer);
    //this->Fit();

#if wxUSE_STATUSBAR
    CreateStatusBar(2);
    SetStatusText("SRT SERVER");
#endif
}

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    *(config->stop) = 1;
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox("Start function called", "Notification", wxOK | wxICON_INFORMATION, this);
}

void MyFrame::OnStart(wxCommandEvent& WXUNUSED(event))
{

    std::cout << "Endereco config: " << config << std::endl;
    config->com = (wchar_t*)malloc(100 * sizeof(WCHAR));
    config->uri = (char*)malloc(100 * sizeof(char));
    config->device = (char*)malloc(100 * sizeof(char));
    config->running = (int*)malloc(100 * sizeof(int));
    config->stop = (int*)malloc(100 * sizeof(int));

    const char* device_name = this->comboBoxDevices->GetValue().c_str();
    char* buffer = new char[strlen(device_name) + 1];
    strcpy_s(buffer, 100, device_name);

    std::cout << buffer << std::endl;

    mbstowcs(config->com, this->comboBoxPort->GetValue().c_str(), 100);
    strcpy_s(config->device, 100, obter_id_dispositivo(buffer, API));
    delete(buffer);

    strcpy_s(config->uri, 100, wxString::Format("srt://:%s", this->portTextCtrl->GetValue().c_str()));

    *config->running = 1;
    *config->stop = 0;
    srt_server(config);

}

void MyFrame::OnStop(wxCommandEvent& WXUNUSED(event)) {

    *config->running = 0;
    *config->stop = 1;

}
