#include <wx/wx.h>
#include <wx/taskbar.h>

#include <boost/filesystem.hpp>

#include "share_icon.xpm"
#include "TaskBarIcon.h"
#include "IPCserver.h"
#include "Settings.h"
#include "MainApp.h"

#include <filesystem> //per la funzione "copy"

enum {
	TIMER_ID = 15000,
	IMG_ID
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
EVT_CLOSE(MainFrame::OnCloseWindow)
EVT_TIMER(TIMER_ID, MainFrame::OnTimer)
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
	m_changeImage = new wxButton
	(
		this,
		IMG_ID,
		"CAMBIA IMMAGINE",
		wxDefaultPosition,
		wxDefaultSize
	);
	m_settings->setImagePath("" + m_settings->getGeneralPath() + "profilo.png");
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
	m_settings->getStato() ? stato = "offline" : stato = "online";	//TODO gestire passaggio da online a offline

	sizer2->Add(new wxStaticText
	(
		this,
		wxID_ANY,
		wxT("Stato: " + stato)
	), flags);
	sizer2->Add(m_changeImage, flags);

	sizer1->Add(sizer2, flags);

	sizerTop->Add(sizer1, flags);

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

	m_timer->Start(5000); //5 sec
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
	m_timer->Stop();
	Destroy();
}

void MainFrame::OnTimer(wxTimerEvent& event)
{
	utente user = m_settings->getUtenteProprietario();
	m_elencoUser->Clear();
	for (auto it : user.getUtentiConnessi()) {
		(*m_elencoUser) << it.getUsername();
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
	//TODO: cambiare immagine di profilo con quella nuova
	boost::filesystem::copy_file(openFileDialog.GetPath().ToStdString(), m_settings->getImagePath(), boost::filesystem::copy_option::overwrite_if_exists);
	
	wxPNGHandler *handler = new wxPNGHandler();
	wxImage::AddHandler(handler);
	wxImage *img = new wxImage();
	img->LoadFile(m_settings->getImagePath(), wxBITMAP_TYPE_PNG, -1);
	m_userImage->SetBitmap(wxBitmap(img->Scale(70, 70, wxIMAGE_QUALITY_HIGH)));
	Update();
}

bool MainFrame::StartServer() {
	m_server = new MyServer(this);
	wxString servername = IPC_SERVICE;
	if (!m_server->Create(servername)) {
		wxDELETE(m_server);
		return false;
	}

	return true;
}

bool MainFrame::StartClient() {
	m_client = new MyClient();
	if (!m_client->Connect(IPC_HOST, IPC_SERVICE, IPC_TOPIC)) {
		wxDELETE(m_client);
		return false;
	}
	return true;
}

void MainFrame::SendFile(std::string path) {
	if (boost::filesystem::is_directory(path))
		m_settings->setIsDir(true);
	else if (boost::filesystem::is_regular_file(path))
		m_settings->setIsDir(false);
	else {
		wxMessageBox("Il path specificato � non valido!", "Warning", wxOK | wxICON_EXCLAMATION);
		return;
	}
	m_settings->setSendPath(path);
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
static bool Online = true;
static bool Offline = false;
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
	Online = true;
	Offline = false;
	m_settings->setStatoOn();

}
void TaskBarIcon::OnMenuUICheckmarkOnline(wxUpdateUIEvent &event)
{
	event.Check(Online);
}

void TaskBarIcon::OnMenuCheckmarkOffline(wxCommandEvent&) {
	Online = false;
	Offline = true;
	m_settings->setStatoOff();
}
void TaskBarIcon::OnMenuUICheckmarkOffline(wxUpdateUIEvent &event)
{
	event.Check(Offline);
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