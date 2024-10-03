#include "wx/wxprec.h"
#include "srt_client.h"
#include "../util/gst_util.h"
#include <iostream>
#include <codecvt>

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifndef wxHAS_IMAGES_IN_RESOURCES
#include "../sample.xpm"
#endif

#define TEXT_COLOR "#f7f8fc"
#define BACKGROUND_COLOR "#314c53"
#define BUTTON_BACKGROUND_COLOR "#f7f8fc"
#define F_COLOUR_1 "#1BB0CE"
#define F_COLOUR_2 "#4F8699"

#define API "wasapi"
#define ENDERECO "srt://200.15.1.70:%s"
//#define ENDERECO "srt://10.13.24.80:%s" // Para teste

struct Config* config = (struct Config*)malloc(sizeof(struct Config));

void print(std::string s) {
    std::cout << s << std::endl;
}

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
    wxStatusBar* statusBar;
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

    MyFrame* frame = new MyFrame("SRT CLIENT");
    frame->Show(true);

    return true;
}

MyFrame::MyFrame(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title)
{
    SetIcon(wxIcon("./icone.ico", wxBITMAP_TYPE_ICO));

    wxColour bgColour(BACKGROUND_COLOR);
    wxColour textColour(TEXT_COLOR);
    wxColour buttonBackgroundColour(BUTTON_BACKGROUND_COLOR);
    wxColour fColour1(F_COLOUR_1);
    wxColour fColour2(F_COLOUR_2);

    SetBackgroundColour(bgColour);
    int size1 = 20, size2 = 100;
    char** list = (char**)malloc(size1 * sizeof(char*));

    this->sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSize(400, 310);

    this->startButton = new wxButton(this, Minimal_Start, wxT("Start"), wxDefaultPosition, wxDefaultSize, 0);
    this->startButton->SetToolTip(wxT("Clique aqui para iniciar o srt_server"));
    this->startButton->SetBackgroundColour(buttonBackgroundColour);
    this->stopButton = new wxButton(this, Minimal_Stop, wxT("Stop"), wxDefaultPosition, wxDefaultSize, 0);
    this->stopButton->SetToolTip(wxT("Clique aqui para parar o srt_server"));
    this->stopButton->SetBackgroundColour(buttonBackgroundColour);
    this->comboBoxDevices = new wxComboBox(this, wxID_ANY);
    this->comboBoxDevices->SetBackgroundColour(buttonBackgroundColour);
    this->comboBoxPort = new wxComboBox(this, wxID_ANY);
    this->comboBoxPort->SetBackgroundColour(buttonBackgroundColour);
    this->portTextCtrl = new wxTextCtrl(this, wxID_ANY);
    this->portTextCtrl->SetBackgroundColour(buttonBackgroundColour);

    //labels

    labelComboBoxDevices = new wxStaticText(this, wxID_ANY, wxT("DEVICE"));
    labelComboBoxDevices->SetForegroundColour(textColour);
    labelComboBoxPort = new wxStaticText(this, wxID_ANY, wxT("PORTA COM"));
    labelComboBoxPort->SetForegroundColour(textColour);
    labelPortTextCtrl = new wxStaticText(this, wxID_ANY, wxT("PORTA SRT"));
    labelPortTextCtrl->SetForegroundColour(textColour);

    if (list == NULL) {
        printf("Erro! Falha na alocação de memória.\n");
        return;
    }

    for (int i = 0; i < size1; i++) {
        //list[i] = (char*)malloc(size2 * sizeof(char));
        list[i] = NULL;
    }

    listar_dispositivos_audio_s(list, size1, API, size2);

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
    this->statusBar = CreateStatusBar(2);
    SetStatusText("SRT CLIENT");
    this->statusBar->SetBackgroundColour(buttonBackgroundColour);

#endif
}

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    stop_loop();
    *(config->stop) = 1;
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox("Start function called", "Notification", wxOK | wxICON_INFORMATION, this);
}

void MyFrame::OnStart(wxCommandEvent& WXUNUSED(event))
{
    while (1)
    {
        try {
            print("Start chamado.");
            //if (config->running != NULL) {
            //    print("Config Presente.");
            //    if (*config->running) {
            //        std::cout << "Já está rodando" << std::endl;
            //        return;
            //    }
            //}
            print("Passado teste rodando.");
            std::cout << "Endereco config: " << config << std::endl;
            config->com = (wchar_t*)malloc(100 * sizeof(WCHAR));
            print("Alocada memoria COM");
            config->uri = (char*)malloc(100 * sizeof(char));
            print("Alocada memoria URI");
            config->device = (char*)malloc(100 * sizeof(char));
            print("Alocada memoria device");
            config->running = (int*)malloc(100 * sizeof(int));
            print("Alocada memoria running");
            config->stop = (int*)malloc(100 * sizeof(int));
            print("Alocada memoria Stop");

            const char* device_name = this->comboBoxDevices->GetValue().c_str();
            char* buffer = new char[strlen(device_name) + 1];
            strcpy_s(buffer, strlen(device_name) + 1, device_name);

            std::cout << buffer << std::endl;

            mbstowcs(config->com, this->comboBoxPort->GetValue().c_str(), 100);
            strcpy_s(config->device, 100, obter_id_dispositivo_s(buffer, API));
            delete(buffer);

            std::cout << "1" << std::endl;

            //strcpy_s(config->uri, 100, wxString::Format("srt://10.13.24.35:%s", this->portTextCtrl->GetValue().c_str()));
            strcpy_s(config->uri, 100, wxString::Format(ENDERECO, this->portTextCtrl->GetValue().c_str()));

            *config->running = 1;
            *config->stop = 0;
            srt_thread(config);
            break;
        }
        catch (int erro) {
            print("Erro ao startar!!!");
            std::cout << "Codigo erro: " << erro << std::endl;
        }
    }
}
void MyFrame::OnStop(wxCommandEvent & WXUNUSED(event)) {

    *config->running = 0;
    *config->stop = 1;
    stop_loop();


}
