#include <wx/wx.h>
#include <wx/taskbar.h>

#include "share_icon.xpm"
#include "share_icon_offline.xpm"
//#include "share_icon_online.xpm"
//#include "share_icon_offline.xpm"

#include "TaskBarIcon.h"
#include "IPCserver.h"
#include "Settings.h"
#include "MainApp.h"
#include "protoType.h"
#include <fstream>
#include <filesystem> //per la funzione "copy"


enum {
	TIMER_ID = 15000,
	IMG_ID,
	RADIO_ID1,
	RADIO_ID2,
	SAVE_ID
};

// ----------------------------------------------------------------------------
// global variables
// ----------------------------------------------------------------------------

static MainFrame *gs_dialog = NULL;


// ----------------------------------------------------------------------------
// MainFrame implementation
// ----------------------------------------------------------------------------

wxDEFINE_EVENT(UPDATE_EVENT, wxCommandEvent);

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_BUTTON(wxID_FILE, MainFrame::OnInviaFile)
EVT_BUTTON(wxID_FILE1, MainFrame::OnInviaDir)
EVT_BUTTON(wxID_OK, MainFrame::OnOK)
EVT_BUTTON(wxID_EXIT, MainFrame::OnExit)
EVT_BUTTON(IMG_ID, MainFrame::OnImage)
EVT_BUTTON(SAVE_ID, MainFrame::OnChangeSavePath)
EVT_CLOSE(MainFrame::OnCloseWindow)
EVT_TIMER(TIMER_ID, MainFrame::OnTimer)
EVT_RADIOBOX(RADIO_ID1, MainFrame::OnRadioBoxStato)
EVT_COMMAND(RADIO_ID1, UPDATE_EVENT, MainFrame::OnMenuUICheckmark)
EVT_RADIOBOX(RADIO_ID2, MainFrame::OnRadioBoxSalvataggio)
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
		RADIO_ID1,
		wxT("Stato: "),
		wxDefaultPosition,
		wxDefaultSize,
		2,
		items,
		0,
		wxRA_SPECIFY_ROWS
	);
	m_status->SetSelection(m_settings->getStato());

	items[0] = wxT("Non domandare salvataggio");
	items[1] = wxT("Domanda salvataggio");
	m_saved = new wxRadioBox
	(
		this,
		RADIO_ID2,
		wxT("Salvataggio: "),
		wxDefaultPosition,
		wxDefaultSize,
		2,
		items,
		0,
		wxRA_SPECIFY_ROWS
	);
	m_saved->SetSelection(m_settings->getAutoSaved());

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

	wxImage *img = new wxImage();
	img->LoadFile(m_settings->getImagePath(), wxBITMAP_TYPE_ANY, -1);
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
	), 0, wxALIGN_CENTER_VERTICAL);
	m_textSavePath = new wxStaticText
	(
		this, 
		wxID_ANY, 
		m_settings->getSavePath(), 
		wxDefaultPosition,
		wxDefaultSize,
		wxST_NO_AUTORESIZE | wxST_ELLIPSIZE_START
	);
	sizer3->Add(m_textSavePath, 0, wxALIGN_CENTER_VERTICAL);
	sizer3->Add(m_changeSavePath, flags);

	wxSizer* sizerBox = new wxBoxSizer(wxHORIZONTAL);
	sizerBox->Add(m_status, flags);
	sizerBox->Add(m_saved, flags);

	sizerTop->Add(sizer1, flags);

	sizerTop->Add(sizerBox, flags);

	sizerTop->Add(sizer3, flags);

	sizerTop->Add(new wxStaticText
	(
		this,
		wxID_ANY,
		wxT("Utenti attualmente online:")
	), flags);

	sizerTop->Add(m_elencoUser, flags);

	wxSizer * const sizerBtns1 = new wxBoxSizer(wxHORIZONTAL);
	sizerBtns1->Add(new wxButton(this, wxID_FILE, wxT("Invia file")), flags);
	sizerBtns1->Add(new wxButton(this, wxID_FILE1, wxT("Invia cartella")), flags);

	wxSizer * const sizerBtns2 = new wxBoxSizer(wxHORIZONTAL);
	sizerBtns2->Add(new wxButton(this, wxID_OK, wxT("Nascondi")), flags);
	sizerBtns2->Add(new wxButton(this, wxID_EXIT, wxT("Esci")), flags);

	sizerTop->Add(sizerBtns1, flags.Align(wxALIGN_CENTER_HORIZONTAL));
	sizerTop->AddStretchSpacer()->SetMinSize(200, 50);
	sizerTop->Add(sizerBtns2, flags.Align(wxALIGN_CENTER_HORIZONTAL));
	SetSizerAndFit(sizerTop);
	Centre();

	m_taskBarIcon = new TaskBarIcon(m_settings);
	UpdateIcon();	

	gs_dialog = this;

	m_timer->Start(UPDATE_WINDOW); //1 sec
}

MainFrame::~MainFrame()
{
	delete m_taskBarIcon;
	wxDELETE(m_server);
	//wxDELETE(m_client);
}

void MainFrame::showBal(std::string title, std::string message) {
	m_taskBarIcon->ShowBalloon(title, message, 15000, wxICON_INFORMATION);
}

void MainFrame::OnInviaFile(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog selectFileDialog(this, "Seleziona un file o una cartella da inviare", "", "",
		"All files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (selectFileDialog.ShowModal() == wxID_CANCEL)
		return;     // the user changed idea...
	SendFile(selectFileDialog.GetPath().ToStdString());
}

void MainFrame::OnInviaDir(wxCommandEvent& WXUNUSED(event))
{
	wxDirDialog selectDirDialog(this, "Seleziona un file o una cartella da inviare", "",
		wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	if (selectDirDialog.ShowModal() == wxID_CANCEL)
		return;     // the user changed idea...
	SendFile(selectDirDialog.GetPath().ToStdString());
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
	m_settings->getIoService().stop();
	m_settings->reciveTCPfileThread.join();
	m_settings->setExitRecive(true);
	m_settings->reciveUdpMessageThread.join();
	m_settings->setExitSend(true);
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
	m_settings->resizeImage(m_settings->getImagePath());
	//******************************//
	//Invia l'immagine aggiornata a tutti gli utenti iscritti
	utente user = m_settings->getUtenteProprietario();
	for (auto it : user.getUtentiConnessi()) {
		sendImage(m_settings->getImagePath(), it.getIpAddr());
	}
	//***************************//
	//wxPNGHandler *handler = new wxPNGHandler();
	//wxImage::AddHandler(handler);
	wxImage *img = new wxImage();
	img->LoadFile(m_settings->getImagePath(), wxBITMAP_TYPE_ANY, -1);
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
	//stato.txt
	Update();
	
}


void MainFrame::OnRadioBoxStato(wxCommandEvent& event)
{
	int sel = m_status->GetSelection();
	if (sel == 0) {
		m_settings->setStatoOn();
		m_textStato->SetLabel("online");
	}
	else {
		m_settings->setStatoOff();
		m_textStato->SetLabel("offline");
		
	}
	UpdateIcon();
	Update();
}

void MainFrame::OnRadioBoxSalvataggio(wxCommandEvent& event)
{
	int sel = m_saved->GetSelection();

	if (sel == 0) {
		m_settings->setAutoSavedOff();
	}
	else {
		m_settings->setAutoSavedOn();
	}

}

void MainFrame::OnMenuUICheckmark(wxCommandEvent& event)
{
	std::string stato;
	m_status->SetSelection(m_settings->getStato());
	m_settings->getStato() ? stato = "offline" : stato = "online";
	m_textStato->SetLabel(stato);
	UpdateIcon();
	Update();
}

void MainFrame::UpdateIcon() {
	if (!m_settings->getStato()) {
		if (!m_taskBarIcon->SetIcon(wxIcon(share_icon),
			"Sharing service\n"
			"Clicca per iniziare a condividere file!"))
		{
			wxLogError(wxT("Could not set icon."));	//TODO gestire l'errore
		}
	}
	else {
		if (!m_taskBarIcon->SetIcon(wxIcon(share_icon_offline),
			"Sharing service\n"
			"Clicca per iniziare a condividere file!"))
		{
			wxLogError(wxT("Could not set icon."));	//TODO gestire l'errore
		}
	}
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

//bool MainFrame::StartClient() 
//{
//	m_client = new MyClient();
//	if (!m_client->Connect(IPC_HOST, IPC_SERVICE, IPC_TOPIC)) {
//		wxDELETE(m_client);
//		return false;
//	}
//	return true;
//}

void MainFrame::SendFile(std::string path)
{
	m_settings->setSendPath(path);
	if (boost::filesystem::is_directory(path))
		m_settings->setIsDir(true);
	else if (boost::filesystem::is_regular_file(path))
		m_settings->setIsDir(false);
	else {
		wxMessageBox("Il path specificato � non valido", "Errore", wxOK | wxICON_ERROR);
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
	PU_STATO,
};


wxBEGIN_EVENT_TABLE(TaskBarIcon, wxTaskBarIcon)
EVT_MENU(PU_RESTORE, TaskBarIcon::OnMenuRestore)
EVT_TASKBAR_LEFT_DCLICK(TaskBarIcon::OnLeftButtonDClick)
EVT_MENU(PU_EXIT, TaskBarIcon::OnMenuExit)
EVT_MENU(PU_STATO, TaskBarIcon::OnMenuStato)
EVT_UPDATE_UI(PU_STATO, TaskBarIcon::OnMenuUIStato)
//EVT_MENU(PU_CHECKMARK_ONLINE, TaskBarIcon::OnMenuCheckmarkOnline)
//EVT_UPDATE_UI(PU_CHECKMARK_ONLINE, TaskBarIcon::OnMenuUICheckmarkOnline)
//EVT_MENU(PU_CHECKMARK_OFFLINE, TaskBarIcon::OnMenuCheckmarkOffline)
//EVT_UPDATE_UI(PU_CHECKMARK_OFFLINE, TaskBarIcon::OnMenuUICheckmarkOffline)
wxEND_EVENT_TABLE()



void TaskBarIcon::OnMenuRestore(wxCommandEvent&)
{
	gs_dialog->Show(true);
	gs_dialog->Iconize(false);
}

void TaskBarIcon::OnLeftButtonDClick(wxTaskBarIconEvent&)
{
	gs_dialog->Show(true);
	gs_dialog->Iconize(false);
}

void TaskBarIcon::OnMenuExit(wxCommandEvent&)
{
	gs_dialog->Close(true);
}

//void TaskBarIcon::OnMenuCheckmarkOnline(wxCommandEvent&) {
//	m_settings->setStatoOn();
//	wxQueueEvent(gs_dialog, new wxCommandEvent(UPDATE_EVENT, RADIO_ID1));
//}
//
//void TaskBarIcon::OnMenuUICheckmarkOnline(wxUpdateUIEvent &event)
//{
//	event.Check(!m_settings->getStato());
//}
//
//void TaskBarIcon::OnMenuCheckmarkOffline(wxCommandEvent&) {
//	m_settings->setStatoOff();
//	wxQueueEvent(gs_dialog, new wxCommandEvent(UPDATE_EVENT, RADIO_ID1));
//}
//void TaskBarIcon::OnMenuUICheckmarkOffline(wxUpdateUIEvent &event)
//{
//	event.Check(m_settings->getStato());
//}

void TaskBarIcon::OnMenuStato(wxCommandEvent&) {
	m_settings->getStato() ? m_settings->setStatoOn() : m_settings->setStatoOff();
	wxQueueEvent(gs_dialog, new wxCommandEvent(UPDATE_EVENT, RADIO_ID1));
}

void TaskBarIcon::OnMenuUIStato(wxUpdateUIEvent &event)
{
	std::string stato;
	m_settings->getStato() ? stato = "Vai online" : stato = "Vai offline";
	event.SetText(stato);
}

wxMenu *TaskBarIcon::CreatePopupMenu()
{
	wxMenu *menu = new wxMenu;
	menu->Append(PU_RESTORE, wxT("Apri Sharing Service"));
	menu->AppendSeparator();
	wxMenu *submenu = new wxMenu;
	std::string stato;
	m_settings->getStato() ? stato = "Vai online" : stato = "Vai offline";
	menu->Append(PU_STATO, stato);
	//submenu->AppendCheckItem(PU_CHECKMARK_ONLINE, wxT("Online"));
	//submenu->AppendCheckItem(PU_CHECKMARK_OFFLINE, wxT("Offline"));
	/*menu->Append(PU_SUBMAIN, wxT("Stato"), submenu);*/
	/* OSX has built-in quit menu for the dock menu, but not for the status item */
#ifdef __WXOSX__ 
	if (OSXIsStatusItem())
#endif
	{
		menu->AppendSeparator();
		menu->Append(PU_EXIT, wxT("Esci"));
	}
	return menu;
}