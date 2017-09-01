#include <wx/wx.h>
#include <wx/taskbar.h>

#include "share_icon.xpm"
#include "TaskBarIcon.h"
#include "IPCserver.h"
#include "Settings.h"
#include "MainApp.h"
#include <fstream>
#include <filesystem> //per la funzione "copy"

#define BUFLEN 65536

enum {
	TIMER_ID = 15000,
	IMG_ID,
	RADIO_ID,
	SAVE_ID
};

// ----------------------------------------------------------------------------
// global variables
// ----------------------------------------------------------------------------

static MainFrame *gs_dialog = NULL;


// ----------------------------------------------------------------------------
// MainFrame implementation
// ----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_BUTTON(wxID_ABOUT, MainFrame::OnAbout)
EVT_BUTTON(wxID_OK, MainFrame::OnOK)
EVT_BUTTON(wxID_EXIT, MainFrame::OnExit)
EVT_BUTTON(IMG_ID, MainFrame::OnImage)
EVT_BUTTON(SAVE_ID, MainFrame::OnChangeSavePath)
EVT_CLOSE(MainFrame::OnCloseWindow)
EVT_TIMER(TIMER_ID, MainFrame::OnTimer)
EVT_RADIOBOX(RADIO_ID, MainFrame::OnRadioBox)
EVT_UPDATE_UI(RADIO_ID, MainFrame::OnMenuUICheckmark)
wxEND_EVENT_TABLE()


MainFrame::MainFrame(const wxString& title, class Settings* settings) : wxFrame(NULL, wxID_ANY, title)
{
	m_selectUser = NULL;
	m_client = NULL;
	m_server = NULL;
	m_settings = settings;
	m_timer = new wxTimer(this, TIMER_ID);
	m_elencoUser = new wxTextCtrl
	(
		this,
		wxID_ANY,
		wxT("Ricerca degli utenti connessi in corso"),
		wxDefaultPosition,
		wxSize(300, 80),
		wxTE_MULTILINE | wxTE_READONLY
	);
	wxString* items = new wxString[2];
	items[0] = wxT("Online");
	items[1] = wxT("Offline");
	m_status = new wxRadioBox
	(
		this,
		RADIO_ID,
		wxT("Stato: "),
		wxDefaultPosition,
		wxDefaultSize,
		2,
		items,
		0,
		wxRA_SPECIFY_ROWS
	);

	items[0] = wxT("Non domandare salvataggio");
	items[1] = wxT("Domanda salvataggio");
	m_saved = new wxRadioBox
	(
		this,
		RADIO_ID,
		wxT("Salvataggio: "),
		wxDefaultPosition,
		wxDefaultSize,
		2,
		items,
		0,
		wxRA_SPECIFY_ROWS
	);

	m_changeImage = new wxButton
	(
		this,
		IMG_ID,
		"CAMBIA IMMAGINE",
		wxDefaultPosition,
		wxDefaultSize
	);
	m_changeSavePath = new wxButton
	(
		this,
		SAVE_ID,
		"CAMBIA",
		wxDefaultPosition,
		wxDefaultSize
	);
	wxPNGHandler *handler = new wxPNGHandler();
	wxImage::AddHandler(handler);
	wxImage *img = new wxImage();
	img->LoadFile(m_settings->getImagePath(), wxBITMAP_TYPE_PNG, -1);
	m_userImage = new wxStaticBitmap
	(
		this,
		wxID_ANY,
		wxBitmap(img->Scale(70, 70, wxIMAGE_QUALITY_HIGH)),
		wxDefaultPosition,
		wxDefaultSize
	);

	wxSizer* const sizerTop = new wxBoxSizer(wxVERTICAL);

	wxSizerFlags flags;
	flags.Border(wxALL, 10);

	wxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	sizer1->Add(m_userImage, flags);

	wxSizer* sizer2 = new wxBoxSizer(wxVERTICAL);
	sizer2->Add(new wxStaticText
	(
		this,
		wxID_ANY,
		wxT("" + m_settings->getUserName())
	), flags);
	
	std::string stato;
	m_settings->getStato() ? stato = "offline" : stato = "online";	
	m_textStato = new wxStaticText(this, wxID_ANY, stato);
	sizer2->Add(m_textStato, flags);
	sizer2->Add(m_changeImage, flags);

	sizer1->Add(sizer2, flags);

	wxSizer* sizer3 = new wxBoxSizer(wxHORIZONTAL);
	sizer3->Add(new wxStaticText
	(
		this,
		wxID_ANY,
		wxT("Salva in: ")
	), flags);
	m_textSavePath = new wxStaticText
	(
		this, 
		wxID_ANY, 
		m_settings->getSavePath(), 
		wxDefaultPosition,
		wxDefaultSize,
		wxST_NO_AUTORESIZE | wxST_ELLIPSIZE_START
	);
	sizer3->Add(m_textSavePath, flags);
	sizer3->Add(m_changeSavePath, flags);

	sizerTop->Add(sizer1, flags);

	sizerTop->Add(m_status, flags);

	sizerTop->Add(m_saved, flags);

	sizerTop->Add(sizer3, flags);

	sizerTop->Add(m_elencoUser, flags);

	sizerTop->Add(new wxStaticText
	(
		this,
		wxID_ANY,
		wxT("Press 'Hide me' to hide this window, Exit to quit.")
	), flags);

	sizerTop->Add(new wxStaticText
	(
		this,
		wxID_ANY,
		wxT("Double-click on the taskbar icon to show me again.")
	), flags);

	sizerTop->AddStretchSpacer()->SetMinSize(200, 50);

	wxSizer * const sizerBtns = new wxBoxSizer(wxHORIZONTAL);
	sizerBtns->Add(new wxButton(this, wxID_ABOUT, wxT("&About")), flags);
	sizerBtns->Add(new wxButton(this, wxID_OK, wxT("&Hide")), flags);
	sizerBtns->Add(new wxButton(this, wxID_EXIT, wxT("E&xit")), flags);

	sizerTop->Add(sizerBtns, flags.Align(wxALIGN_CENTER_HORIZONTAL));
	SetSizerAndFit(sizerTop);
	Centre();

	m_taskBarIcon = new TaskBarIcon(m_settings);

	if (!m_taskBarIcon->SetIcon(wxIcon(share_icon),
		"Sharing service\n"
		"Click to start sharing!"))
	{
		wxLogError(wxT("Could not set icon."));	//TODO gestire l'errore
	}

	gs_dialog = this;

	m_timer->Start(1000); //2 sec
}

MainFrame::~MainFrame()
{
	delete m_taskBarIcon;
	wxDELETE(m_client);
	wxDELETE(m_server);
}

void MainFrame::showBal(std::string title, std::string message) {
	m_taskBarIcon->ShowBalloon(title, message, 15000, wxICON_INFORMATION);
}

void MainFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	static const char * const title = "About wxWidgets Taskbar Sample";
	static const char * const message
		= "wxWidgets sample showing wxTaskBarIcon class\n"
		"\n"
		"(C) 1997 Julian Smart\n"
		"(C) 2007 Vadim Zeitlin";

	m_taskBarIcon->ShowBalloon(title, message, 15000, wxICON_INFORMATION);
}

void MainFrame::OnOK(wxCommandEvent& WXUNUSED(event))
{
	Show(false);
}

void MainFrame::OnExit(wxCommandEvent& WXUNUSED(event))
{
	Close(true);
}

void MainFrame::OnCloseWindow(wxCloseEvent& WXUNUSED(event))
{

	m_settings->exit_recive_udp.store(true);
	m_settings->reciveUdpMessageThread.join();
	m_settings->exit_send_udp.store(true);
	m_settings->sendUdpMessageThread.join();
	m_settings->exit_recive_tcp.store(true);
	m_settings->io_service_tcp_file.stop();
	m_settings->sendUdpMessageThread.join();
	m_timer->Stop();
	Destroy();
}

void MainFrame::OnTimer(wxTimerEvent& event)
{
	utente user = m_settings->getUtenteProprietario();
	m_elencoUser->Clear();
	for (auto it : user.getUtentiOnline()) {
		(*m_elencoUser) << it.getUsername() + " ";
	}
	if (m_elencoUser->IsEmpty())
		(*m_elencoUser) << "Nessun utente connesso.";
}

void MainFrame::OnImage(wxCommandEvent& event)
{
	wxFileDialog openFileDialog(this, _("Scegli un'immagine"), wxT("C:\\Users\\" + m_settings->getUserName() + "\\Desktop\\"), "",
			"All files (*.*)|*.*|JPG files (*jpg)|*jpg|JPEG files (*jpeg)|*jpeg|PNG files (*png)|*png", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return;     // the user changed idea...
	boost::filesystem::copy_file(openFileDialog.GetPath().ToStdString(), m_settings->getImagePath(), boost::filesystem::copy_option::overwrite_if_exists);
	
	//******************************//
	//Invia l'immagine aggiornata a tutti gli utenti iscritti
	utente user = m_settings->getUtenteProprietario();
	for (auto it : user.getUtentiConnessi()) {
		sendImage(m_settings->getImagePath(), it.getIpAddr());
	}
	//***************************//
	wxPNGHandler *handler = new wxPNGHandler();
	wxImage::AddHandler(handler);
	wxImage *img = new wxImage();
	img->LoadFile(m_settings->getImagePath(), wxBITMAP_TYPE_PNG, -1);
	m_userImage->SetBitmap(wxBitmap(img->Scale(70, 70, wxIMAGE_QUALITY_HIGH)));
	Update();
}

void MainFrame::OnChangeSavePath(wxCommandEvent& event)
{
	wxDirDialog selectDirDialog(this, "Seleziona cartella di salvataggio", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	if (selectDirDialog.ShowModal() == wxID_CANCEL)
		return;     // the user changed idea...
	m_settings->setSavePath(selectDirDialog.GetPath().ToStdString());
	m_textSavePath->SetLabel(selectDirDialog.GetPath());
	std::fstream save_path_file;
	save_path_file.open(m_settings->getGeneralPath() + "stato.txt", std::fstream::out);
	save_path_file << m_settings->getSavePath() << std::endl;
	if (m_settings->getStato()==0) {
		save_path_file << "online\n";
	}
	else {
		save_path_file << "offline\n";
	}
	save_path_file.close();
	Update();
	
}


void MainFrame::OnRadioBox(wxCommandEvent& event)
{
	int sel = m_status->GetSelection();
	if (sel == 0) {
		//wxMessageBox("Setto Online!", wxT("INFO"), wxOK | wxICON_INFORMATION);
		m_settings->setStatoOn();
		m_textStato->SetLabel("online");
	}
	else {
		//wxMessageBox("Setto Offline!", wxT("INFO"), wxOK | wxICON_INFORMATION);
		m_settings->setStatoOff();
		m_textStato->SetLabel("offline");
	}

	sel = m_saved->GetSelection();
	if (sel == 0) {
		//wxMessageBox("salvataggio automatico!", wxT("INFO"), wxOK | wxICON_INFORMATION);
		m_settings->setAutoSavedOff();
	}
	else {
		//wxMessageBox("salvataggio su richesta!", wxT("INFO"), wxOK | wxICON_INFORMATION);
		m_settings->setAutoSavedOn();
	}

	std::fstream save_path_file;
	save_path_file.open(m_settings->getGeneralPath() + "stato.txt", std::fstream::out);
	save_path_file << m_settings->getSavePath() << std::endl;
	if (m_settings->getStato() == 0) {
		save_path_file << "online\n";
	}
	else {
		save_path_file << "offline\n";
	}
	save_path_file.close();

	Update();
	wxQueueEvent(m_taskBarIcon, new wxUpdateUIEvent);
}

void MainFrame::OnMenuUICheckmark(wxUpdateUIEvent& event)
{
	
	m_status->SetSelection(m_settings->getStato());
	std::string stato;
	m_settings->getStato() ? stato = "offline" : stato = "online";
	m_textStato->SetLabel(stato);
	Update();
}


bool MainFrame::StartServer() 
{
	m_server = new MyServer(this);
	wxString servername = IPC_SERVICE;
	if (!m_server->Create(servername)) {
		wxDELETE(m_server);
		return false;
	}

	return true;
}

bool MainFrame::StartClient() 
{
	m_client = new MyClient();
	if (!m_client->Connect(IPC_HOST, IPC_SERVICE, IPC_TOPIC)) {
		wxDELETE(m_client);
		return false;
	}
	return true;
}

void MainFrame::SendFile(std::string path)
{
	m_settings->setSendPath(path);
	if (boost::filesystem::is_directory(path))
		m_settings->setIsDir(true);
	else if (boost::filesystem::is_regular_file(path))
		m_settings->setIsDir(false);
	else {
		wxMessageBox("Il path specificato è non valido!", "Warning", wxOK | wxICON_EXCLAMATION);
		return;
	}
	m_selectUser = new WindowSelectUser(this, m_settings);
	this->Hide();
	m_selectUser->Show();
}


// ----------------------------------------------------------------------------
// MyTaskBarIcon implementation
// ----------------------------------------------------------------------------

enum
{
	PU_RESTORE = 10001,
	PU_EXIT,
	PU_CHECKMARK,
	PU_CHECKMARK_ONLINE,
	PU_CHECKMARK_OFFLINE,
	PU_SUB1,
	PU_SUB2,
	PU_SUBMAIN
};


wxBEGIN_EVENT_TABLE(TaskBarIcon, wxTaskBarIcon)
EVT_MENU(PU_RESTORE, TaskBarIcon::OnMenuRestore)
EVT_MENU(PU_EXIT, TaskBarIcon::OnMenuExit)
EVT_MENU(PU_CHECKMARK, TaskBarIcon::OnMenuCheckmark)
EVT_UPDATE_UI(PU_CHECKMARK, TaskBarIcon::OnMenuUICheckmark)
//MIO
EVT_MENU(PU_CHECKMARK_ONLINE, TaskBarIcon::OnMenuCheckmarkOnline)
EVT_UPDATE_UI(PU_CHECKMARK_ONLINE, TaskBarIcon::OnMenuUICheckmarkOnline)
EVT_MENU(PU_CHECKMARK_OFFLINE, TaskBarIcon::OnMenuCheckmarkOffline)
EVT_UPDATE_UI(PU_CHECKMARK_OFFLINE, TaskBarIcon::OnMenuUICheckmarkOffline)
//MIO
EVT_TASKBAR_LEFT_DCLICK(TaskBarIcon::OnLeftButtonDClick)
EVT_MENU(PU_SUB1, TaskBarIcon::OnMenuSub)
EVT_MENU(PU_SUB2, TaskBarIcon::OnMenuSub)
wxEND_EVENT_TABLE()



void TaskBarIcon::OnMenuRestore(wxCommandEvent&)
{
	gs_dialog->Show(true);
}

void TaskBarIcon::OnMenuExit(wxCommandEvent&)
{
	
	gs_dialog->Close(true);
}

static bool check = true;
//static bool Online = true;
//static bool Offline = false;
void TaskBarIcon::OnMenuCheckmark(wxCommandEvent&)
{
	check = !check;
}

void TaskBarIcon::OnMenuUICheckmark(wxUpdateUIEvent &event)
{
	event.Check(check);
}

/*MIO*/
void TaskBarIcon::OnMenuCheckmarkOnline(wxCommandEvent&) {
	//Online = true;
	//Offline = false;
	m_settings->setStatoOn();
	std::fstream save_path_file;
	save_path_file.open(m_settings->getGeneralPath() + "stato.txt", std::fstream::out);
	save_path_file << m_settings->getSavePath() << std::endl;
	save_path_file << "online\n";
	save_path_file.close();
	wxQueueEvent(gs_dialog, new wxUpdateUIEvent);
}
void TaskBarIcon::OnMenuUICheckmarkOnline(wxUpdateUIEvent &event)
{
	event.Check(!m_settings->getStato());
}


void TaskBarIcon::OnMenuCheckmarkOffline(wxCommandEvent&) {
	//Online = false;
	//Offline = true;
	m_settings->setStatoOff();
	std::fstream save_path_file;
	save_path_file.open(m_settings->getGeneralPath() + "stato.txt", std::fstream::out);
	save_path_file << m_settings->getSavePath() << std::endl;
	save_path_file << "offline\n";
	save_path_file.close();
	wxQueueEvent(gs_dialog, new wxUpdateUIEvent);
}
void TaskBarIcon::OnMenuUICheckmarkOffline(wxUpdateUIEvent &event)
{
	event.Check(m_settings->getStato());
}

void TaskBarIcon::OnMenuSub(wxCommandEvent&)
{
	wxMessageBox(wxT("You clicked on a submenu!"));
}

// Overridables
wxMenu *TaskBarIcon::CreatePopupMenu()
{
	wxMenu *menu = new wxMenu;
	menu->Append(PU_RESTORE, wxT("&Restore main window"));
	menu->AppendSeparator();
	menu->AppendCheckItem(PU_CHECKMARK, wxT("Test &check mark"));
	menu->AppendRadioItem(-1, wxT("Prova bottone"));
	menu->AppendSeparator();
	wxMenu *submenu = new wxMenu;
	submenu->AppendCheckItem(PU_CHECKMARK_ONLINE, wxT("&Online"));
	submenu->AppendCheckItem(PU_CHECKMARK_OFFLINE, wxT("&Offline"));
	/*submenu->Append(PU_SUB1, wxT("One submenu"));
	submenu->AppendSeparator();
	submenu->Append(PU_SUB2, wxT("Another submenu"));*/
	menu->Append(PU_SUBMAIN, wxT("Stato"), submenu);
	/* OSX has built-in quit menu for the dock menu, but not for the status item */
#ifdef __WXOSX__ 
	if (OSXIsStatusItem())
#endif
	{
		menu->AppendSeparator();
		menu->Append(PU_EXIT, wxT("E&xit"));
	}
	return menu;
}

void TaskBarIcon::OnLeftButtonDClick(wxTaskBarIconEvent&)
{
	gs_dialog->Show(true);
}