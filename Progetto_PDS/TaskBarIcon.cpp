//NON COMMENTATO 
#include <wx/wx.h>
#include <wx/taskbar.h>
#include <wx/artprov.h>
#include <wx/caret.h>
#include <shellapi.h>

#include "share_icon.xpm"
#include "share_icon_offline.xpm"
#include <boost/algorithm/string/classification.hpp> 
#include <boost/algorithm/string/split.hpp> 
#include "TaskBarIcon.h"
#include "IPCserver.h"
#include "Settings.h"
#include "MainApp.h"
#include "protoType.h"
#include <fstream>
#include <filesystem>

#define	SIZE 100 //dimensione immagine in pixel

enum {
	TIMER_ID = 15000,
	IMG_ID,
	SAVE_ID,
	USER_ID,
	CONTEXT_ID,
	RADIO_ID1,
	RADIO_ID2,
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
EVT_BUTTON(CONTEXT_ID, MainFrame::OnContextMenu)
EVT_BUTTON(USER_ID, MainFrame::OnChangeUsername)
EVT_CLOSE(MainFrame::OnCloseWindow)
EVT_TIMER(TIMER_ID, MainFrame::OnTimer)
EVT_RADIOBOX(RADIO_ID1, MainFrame::OnRadioBoxStato)
EVT_COMMAND(RADIO_ID1, UPDATE_EVENT, MainFrame::OnMenuUICheckmark)
EVT_RADIOBOX(RADIO_ID2, MainFrame::OnRadioBoxSalvataggio)
EVT_TEXT_ENTER(USER_ID, MainFrame::OnChangeUsername)
wxEND_EVENT_TABLE()


MainFrame::MainFrame() : wxFrame(NULL, wxID_ANY, "dumb window") 
{
	m_taskBarIcon = NULL;
	m_selectUser = NULL;
	m_client = NULL;
	m_server = NULL;
};

MainFrame::MainFrame(const wxString& title, class Settings* settings) : 
	wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN)
{
	this->SetBackgroundColour(wxColour(240,242,245));
	this->SetFont(this->GetFont().Bold().Scale(0.9f));
	m_selectUser = NULL;
	m_client = NULL;
	m_server = NULL;
	m_settings = settings;
	m_timer = new wxTimer(this, TIMER_ID);

	/*
		Elenco utenti connessi
	*/
	m_elencoUser = new wxTextCtrl
	(
		this,
		wxID_ANY,
		wxT("Ricerca degli utenti connessi in corso"),
		wxDefaultPosition,
		wxSize(350, 40),
		wxTE_MULTILINE | wxTE_READONLY 
	);
	m_elencoUser->SetScrollbar(wxVERTICAL, 0,0,0);
	m_elencoUser->Bind(wxEVT_SET_FOCUS, [&](wxFocusEvent&) {
		HideCaret(m_elencoUser->GetHWND());
	});


	/*
		Pulsanti scelta stato
	*/
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

	/*
		Pulsanti scelta salvataggio automatico	
	*/
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
	
	/*
		Bottone cambia immagine profilo
	*/
	wxBitmap* cambia_immagine = new wxBitmap();
	cambia_immagine->LoadFile(m_settings->getGeneralPath() + "bottoni\\cambia_immagine.png", wxBITMAP_TYPE_ANY);
	wxBitmap* cambia_immagine_hover = new wxBitmap();
	cambia_immagine_hover->LoadFile(m_settings->getGeneralPath() + "bottoni\\cambia_immagine_hover.png", wxBITMAP_TYPE_ANY);
	m_changeImage = new wxBitmapButton
	(
		this,
		IMG_ID,
		*cambia_immagine,
		wxDefaultPosition,
		wxDefaultSize
	);
	m_changeImage->SetWindowStyle(wxNO_BORDER);
	m_changeImage->SetBitmapHover(*cambia_immagine_hover);
	m_changeImage->SetBackgroundColour(this->GetBackgroundColour());
	
	/*
		Bottone gestione scorciatoia context menu
	*/
	int lung = 91, alt = 26;
	wxBitmap* aggiungi = new wxBitmap();
	aggiungi->LoadFile(m_settings->getGeneralPath() + "bottoni\\aggiungi.png", wxBITMAP_TYPE_ANY);
	wxBitmap* aggiungi_hover = new wxBitmap();
	aggiungi_hover->LoadFile(m_settings->getGeneralPath() + "bottoni\\aggiungi_hover.png", wxBITMAP_TYPE_ANY);

	wxBitmap* rimuovi = new wxBitmap();
	rimuovi->LoadFile(m_settings->getGeneralPath() + "bottoni\\rimuovi.png", wxBITMAP_TYPE_ANY);
	wxBitmap* rimuovi_hover = new wxBitmap();
	rimuovi_hover->LoadFile(m_settings->getGeneralPath() + "bottoni\\rimuovi_hover.png", wxBITMAP_TYPE_ANY);

	m_contextMenu = new wxBitmapButton
	(
		this,
		CONTEXT_ID,
		*aggiungi,
		wxDefaultPosition,
		wxDefaultSize
	);
	m_contextMenu->SetWindowStyle(wxNO_BORDER);
	m_contextMenu->SetBackgroundColour(this->GetBackgroundColour());
	if (m_settings->getScorciatoia() == scorciatoia::SCORCIATOIA_ASSENTE) {
		m_contextMenu->SetBitmap(*aggiungi);
		m_contextMenu->SetBitmapHover(*aggiungi_hover);
	}
	else {
		m_contextMenu->SetBitmap(*rimuovi);
		m_contextMenu->SetBitmapHover(*rimuovi_hover);
	}
	
	/*
		Bottone modifica path autosalvataggio
	*/
	wxBitmap* cambia = new wxBitmap();
	cambia->LoadFile(m_settings->getGeneralPath() + "bottoni\\cambia.png", wxBITMAP_TYPE_ANY);
	wxBitmap* cambia_hover = new wxBitmap();
	cambia_hover->LoadFile(m_settings->getGeneralPath() + "bottoni\\cambia_hover.png", wxBITMAP_TYPE_ANY);
	m_changeSavePath = new wxBitmapButton
	(
		this,
		SAVE_ID,
		*cambia,
		wxDefaultPosition,
		wxDefaultSize
	);
	m_changeSavePath->SetWindowStyle(wxNO_BORDER);
	m_changeSavePath->SetBitmapHover(*cambia_hover);
	m_changeSavePath->SetBackgroundColour(this->GetBackgroundColour());

	/*
		Path autosalvataggio corrente
	*/
	m_textSavePath = new wxStaticText
	(
		this,
		wxID_ANY,
		m_settings->getSavePath(),
		wxDefaultPosition,
		wxDefaultSize,
		wxST_NO_AUTORESIZE | wxST_ELLIPSIZE_START
	);
	m_textSavePath->SetToolTip(m_settings->getSavePath());
	m_textSavePath->SetMaxSize(wxSize(200, 50));

	/*
		Immagine profilo
	*/
	wxImage* img = new wxImage();
	img->LoadFile(m_settings->getImagePath(), wxBITMAP_TYPE_ANY, -1);
	m_userImage = new wxStaticBitmap
	(
		this,
		wxID_ANY,
		wxBitmap(img->Scale(SIZE, SIZE, wxIMAGE_QUALITY_HIGH)),
		wxDefaultPosition,
		wxDefaultSize
	);

	/*
		Nome utente modificabile
	*/
	wxStaticBitmap* editIcon = new wxStaticBitmap(this, wxID_ANY, wxBITMAP_PNG(edit_icon));
	m_nome = new wxTextCtrl
	(
		this,
		USER_ID,
		wxT("" + m_settings->getUserName()),
		wxDefaultPosition,
		wxSize(170, 30),
		wxNO_BORDER
	);
	m_nome->SetMaxLength(20);
	m_nome->SetBackgroundColour(this->GetBackgroundColour());
	m_nome->SetFont(m_nome->GetFont().Bold().Scaled(1.3f));
	m_nome->SetToolTip("Clicca per modificare il nome utente");
	m_nome->Bind(wxEVT_KILL_FOCUS, &MainFrame::OnLoseFocus, this);

	/*
		Bottone invia file
	*/
	wxBitmap* inviaFile = new wxBitmap();
	inviaFile->LoadFile(m_settings->getGeneralPath() + "bottoni\\invia_file.png", wxBITMAP_TYPE_ANY);
	wxBitmap* inviaFile_hover = new wxBitmap();
	inviaFile_hover->LoadFile(m_settings->getGeneralPath() + "bottoni\\invia_file_hover.png", wxBITMAP_TYPE_ANY);
	wxBitmapButton * m_inviaFile = new wxBitmapButton
	(
		this,
		wxID_FILE,
		*inviaFile,
		wxDefaultPosition
	);
	m_inviaFile->SetWindowStyle(wxNO_BORDER);
	m_inviaFile->SetBitmapHover(*inviaFile_hover);
	m_inviaFile->SetBackgroundColour(this->GetBackgroundColour());

	/*
		Bottone invia cartella
	*/
	lung = 90, alt = 25;
	wxBitmap* inviaCartella = new wxBitmap();
	inviaCartella->LoadFile(m_settings->getGeneralPath() + "bottoni\\invia_cartella.png", wxBITMAP_TYPE_ANY);
	wxBitmap* inviaCartella_hover = new wxBitmap();
	inviaCartella_hover->LoadFile(m_settings->getGeneralPath() + "bottoni\\invia_cartella_hover.png", wxBITMAP_TYPE_ANY);
	wxBitmapButton * m_inviaCartella = new wxBitmapButton
	(
		this,
		wxID_FILE1,
		*inviaCartella,
		wxDefaultPosition
	);
	m_inviaCartella->SetWindowStyle(wxNO_BORDER);
	m_inviaCartella->SetBitmapHover(*inviaCartella_hover);
	m_inviaCartella->SetBackgroundColour(this->GetBackgroundColour());


	/*
		Bottone nascondi finestra principale
	*/
	wxBitmap* nascondi = new wxBitmap();
	nascondi->LoadFile(m_settings->getGeneralPath() + "bottoni\\nascondi.png", wxBITMAP_TYPE_ANY);
	wxBitmap* nascondi_hover = new wxBitmap();
	nascondi_hover->LoadFile(m_settings->getGeneralPath() + "bottoni\\nascondi_hover.png", wxBITMAP_TYPE_ANY);
	wxBitmapButton * m_nascondi = new wxBitmapButton
	(
		this,
		wxID_OK,
		wxBitmap(*nascondi),
		wxDefaultPosition
	);
	m_nascondi->SetWindowStyle(wxNO_BORDER);
	m_nascondi->SetBitmapHover(*nascondi_hover);
	m_nascondi->SetBackgroundColour(this->GetBackgroundColour());

	/*
		Bottone chiusura applicazione
	*/
	wxBitmap* esci = new wxBitmap();
	esci->LoadFile(m_settings->getGeneralPath() + "bottoni\\esci.png", wxBITMAP_TYPE_ANY);
	wxBitmap* esci_hover = new wxBitmap();
	esci_hover->LoadFile(m_settings->getGeneralPath() + "bottoni\\esci_hover.png", wxBITMAP_TYPE_ANY);
	wxBitmapButton * m_esci = new wxBitmapButton
	(
		this,
		wxID_EXIT,
		*esci,
		wxDefaultPosition
	);
	m_esci->SetWindowStyle(wxNO_BORDER);
	m_esci->SetBitmapHover(*esci_hover);
	m_esci->SetBackgroundColour(this->GetBackgroundColour());


	/*
	GERARCHIA SIZER E CONTROLLI

	+sizerTop
		+sizerImage
		 	m_userImage
			+sizerUserName
				+sizerName 
					editIcon
					m_nome
				m_changeImage
		+sizerBox
			m_status
			m_saved
		+sizerGrid
			text(Scorciatoia context menù)
			m_contextMenu
			+sizerText
				text(Salva in)
				m_textSavePath
			m_changeSavePath
		text(Utenti online)
		m_elencoUser
		+sizerBtns1
			m_inviaFile
			m_inviaCartella
		spazio
		+sizerBtns2
			m_nascondi
			m_esci
	*/

	wxSizerFlags flags;
	flags.Border(wxALL, 10);
	wxSizer* sizerTop = new wxBoxSizer(wxVERTICAL);
	wxSizer* sizerImage = new wxBoxSizer(wxHORIZONTAL);
	wxSizer* sizerUserName = new wxBoxSizer(wxVERTICAL);
	wxSizer* sizerName = new wxBoxSizer(wxHORIZONTAL);
	wxSizer* sizerGrid = new wxFlexGridSizer(2, 20, 20);
	wxSizer* sizerText = new wxBoxSizer(wxHORIZONTAL);
	wxSizer* sizerBox = new wxBoxSizer(wxHORIZONTAL);
	wxSizer* sizerBtns1 = new wxBoxSizer(wxHORIZONTAL);
	wxSizer* sizerBtns2 = new wxBoxSizer(wxHORIZONTAL);

	sizerName->Add(editIcon, 0, wxRIGHT, 3);
	sizerName->Add(m_nome);

	sizerUserName->Add(sizerName, 0, wxALL, 10);
	sizerUserName->Add(m_changeImage, flags);
	
	sizerImage->Add(m_userImage, flags);
	sizerImage->Add(sizerUserName, 0, wxALIGN_LEFT | wxLEFT, 10);
	
	sizerBox->Add(m_status, flags);
	sizerBox->Add(m_saved, flags);

	sizerText->Add(new wxStaticText
	(
		this,
		wxID_ANY,
		wxT("Salva in: ")
	), 0, wxALIGN_CENTER_VERTICAL);
	sizerText->Add(m_textSavePath);

	sizerGrid->Add(new wxStaticText
	(
		this,
		wxID_ANY,
		wxT("Scorciatoia nel context menù")
	), 1, wxALIGN_CENTRE_VERTICAL);
	sizerGrid->Add(m_contextMenu, 0, wxALIGN_CENTRE_HORIZONTAL);
	sizerGrid->Add(sizerText, 1, wxALIGN_CENTER_VERTICAL);
	sizerGrid->Add(m_changeSavePath, 0, wxALIGN_CENTRE_HORIZONTAL);

	sizerBtns1->Add(m_inviaFile, flags);
	sizerBtns1->Add(m_inviaCartella, flags);

	sizerBtns2->Add(m_nascondi, flags);
	sizerBtns2->Add(m_esci, flags);

	sizerTop->Add(sizerImage, flags);
	sizerTop->Add(sizerBox, flags);
	sizerTop->Add(sizerGrid, flags);
	sizerTop->Add(new wxStaticText
	(
		this,
		wxID_ANY,
		wxT("Utenti attualmente online:")
	), flags);
	sizerTop->Add(m_elencoUser, flags);
	sizerTop->Add(sizerBtns1, flags.Align(wxALIGN_CENTER_HORIZONTAL));
	sizerTop->AddStretchSpacer()->SetMinSize(200, 50);
	sizerTop->Add(sizerBtns2, flags.Align(wxALIGN_CENTER_HORIZONTAL));
	
	SetSizerAndFit(sizerTop);
	Centre();

	m_taskBarIcon = new TaskBarIcon(m_settings);
	m_settings->setTaskBarIcon(m_taskBarIcon);
	UpdateIcon();

	gs_dialog = this;

	m_timer->Start(UPDATE_WINDOW); //1 sec
}

MainFrame::~MainFrame()
{
	if (m_taskBarIcon != NULL)
		delete m_taskBarIcon;
	wxDELETE(m_server);
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
	m_settings->closeSocket();
	m_settings->reciveUdpMessageThread.join();
	m_settings->setExitSend(true);
	m_settings->sendUdpMessageThread.join();
	m_timer->Stop();
	for (boost::filesystem::recursive_directory_iterator it(m_settings->getGeneralPath() + "local_image"); it != boost::filesystem::recursive_directory_iterator(); ++it) {
		if (boost::filesystem::is_regular_file(*it)) {
			boost::filesystem::remove(*it);
		}
	}
	Destroy();
}

void MainFrame::OnTimer(wxTimerEvent& event)
{
	utente user = m_settings->getUtenteProprietario();
	m_elencoUser->Clear();
	bool first = true;
	for (auto it : user.getUtentiOnline()) {
		if (first) {
			(*m_elencoUser) << it.getUsername();
			first = false;
		}
		else {
			(*m_elencoUser) << ", " + it.getUsername();
		}

	}
	if (m_elencoUser->IsEmpty()) {
		(*m_elencoUser) << "Nessun utente connesso.";
		first = true;
	}

	if (m_elencoUser->GetValue().ToStdString().length()>=224) {
		m_elencoUser->SetScrollbar(wxVERTICAL, 0, 0, 50);
	}
	else {
		m_elencoUser->SetScrollbar(wxVERTICAL, 0, 0, 0);
	}
}

void MainFrame::OnChangeUsername(wxCommandEvent& event)
{
	std::string oldUsername(m_settings->getUtenteProprietario().getUsername());
	std::string username(m_nome->GetValue());
	if (username.empty()==true) {
		m_settings->getUtenteProprietario().setUsername(oldUsername);
	}
	else {
		m_settings->getUtenteProprietario().setUsername(username);
	}
	
	
	m_settings->updateState();
	Update();
}

void MainFrame::OnLoseFocus(wxFocusEvent& event)
{
	m_nome->SetLabel(m_settings->getUserName());
	event.Skip();
	Update();
}

void MainFrame::OnImage(wxCommandEvent& event)
{
	wxFileDialog openFileDialog(this, _("Scegli un'immagine"), wxT("C:\\Users\\" + m_settings->getUserNamePc() + "\\Desktop\\"), "",
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
	wxImage *img = new wxImage();
	img->LoadFile(m_settings->getImagePath(), wxBITMAP_TYPE_ANY, -1);
	m_userImage->SetBitmap(wxBitmap(img->Scale(SIZE, SIZE, wxIMAGE_QUALITY_HIGH)));
	Update();
}

void MainFrame::OnChangeSavePath(wxCommandEvent& event)
{
	wxDirDialog selectDirDialog(this, "Seleziona cartella di salvataggio", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	if (selectDirDialog.ShowModal() == wxID_CANCEL)
		return;     // the user changed idea...
	m_settings->setSavePath(selectDirDialog.GetPath().ToStdString());
	m_textSavePath->SetLabel(selectDirDialog.GetPath());
	m_textSavePath->SetToolTip(selectDirDialog.GetPath());
	Layout();
	Update();
}

void MainFrame::OnRadioBoxStato(wxCommandEvent& event)
{
	int sel = m_status->GetSelection();
	if (sel == 0) {
		m_settings->setStatoOn();
	}
	else {
		m_settings->setStatoOff();
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

void MainFrame::OnContextMenu(wxCommandEvent& event)
{
	if (m_settings->getMod() == modalità::MOD_USER) {
		std::string str = m_settings->getGeneralPath() + "Progetto_PDS.exe";
		std::wstring stemp = std::wstring(str.begin(), str.end());
		LPCTSTR path = stemp.c_str();

		wxDELETE(m_server);

		ShellExecute(NULL,
			TEXT("runas"),
			path,
			TEXT("_ADMIN_PRIVILEGES"),
			NULL,
			SW_SHOWNORMAL
		);

		Close(true);
		return;
	}

	if (m_settings->getScorciatoia() == scorciatoia::SCORCIATOIA_ASSENTE)
		m_settings->AddRegKey();
	else
		m_settings->RemRegKey();
	wxBitmap* aggiungi = new wxBitmap();
	aggiungi->LoadFile(m_settings->getGeneralPath() + "bottoni\\aggiungi.png", wxBITMAP_TYPE_ANY);
	wxBitmap* aggiungi_hover = new wxBitmap();
	aggiungi_hover->LoadFile(m_settings->getGeneralPath() + "bottoni\\aggiungi_hover.png", wxBITMAP_TYPE_ANY);

	wxBitmap* rimuovi = new wxBitmap();
	rimuovi->LoadFile(m_settings->getGeneralPath() + "bottoni\\rimuovi.png", wxBITMAP_TYPE_ANY);
	wxBitmap* rimuovi_hover = new wxBitmap();
	rimuovi_hover->LoadFile(m_settings->getGeneralPath() + "bottoni\\rimuovi_hover.png", wxBITMAP_TYPE_ANY);
	m_contextMenu->SetWindowStyle(wxNO_BORDER);
	m_contextMenu->SetBackgroundColour(this->GetBackgroundColour());
	if (m_settings->getScorciatoia() == scorciatoia::SCORCIATOIA_ASSENTE) {
		m_contextMenu->SetBitmap(*aggiungi);
		m_contextMenu->SetBitmapHover(*aggiungi_hover);
	}
	else {
		m_contextMenu->SetBitmap(*rimuovi);
		m_contextMenu->SetBitmapHover(*rimuovi_hover);
	}

	Update();
}

void MainFrame::OnMenuUICheckmark(wxCommandEvent& event)
{
	if (m_settings->getStato()) {
		m_status->SetSelection(1);
	}
	else {
		m_status->SetSelection(0);
	}
	UpdateIcon();
	Update();
}

void MainFrame::UpdateIcon() {
	if (!m_settings->getStato()) {
		if (!m_taskBarIcon->SetIcon(wxIcon(share_icon),
			"Sharing service\n"
			"Clicca per iniziare a condividere file!"))
		{
			wxLogError(wxT("Non è stato possibile settare l'icona."));
		}
	}
	else {
		if (!m_taskBarIcon->SetIcon(wxIcon(share_icon_offline),
			"Sharing service\n"
			"Clicca per iniziare a condividere file!"))
		{
			wxLogError(wxT("Non è stato possibile settare l'icona taskbar."));
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

void MainFrame::SendFile(std::string path)
{
	m_settings->setSendPath(path);
	if (boost::filesystem::is_directory(path))
		m_settings->setIsDir(true);
	else if (boost::filesystem::is_regular_file(path))
		m_settings->setIsDir(false);
	else {
		wxMessageBox("Il path specificato è non valido", "Errore", wxOK | wxICON_ERROR);
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
	PU_DOWNLOAD
};


wxBEGIN_EVENT_TABLE(TaskBarIcon, wxTaskBarIcon)
EVT_MENU(PU_RESTORE, TaskBarIcon::OnMenuRestore)
EVT_TASKBAR_LEFT_DCLICK(TaskBarIcon::OnLeftButtonDClick)
EVT_MENU(PU_EXIT, TaskBarIcon::OnMenuExit)
EVT_MENU(PU_STATO, TaskBarIcon::OnMenuStato)
//EVT_MENU(PU_DOWNLOAD, TaskBarIcon::OnMenuDownload)
EVT_UPDATE_UI(PU_STATO, TaskBarIcon::OnMenuUIStato)
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

//void TaskBarIcon::OnMenuDownload(wxCommandEvent&) {
//	m_settings->showDownload();
//}

wxMenu *TaskBarIcon::CreatePopupMenu()
{
	wxMenu *menu = new wxMenu;
	menu->Append(PU_RESTORE, wxT("Apri Sharing Service"));
	menu->AppendSeparator();
	wxMenu *submenu = new wxMenu;
	std::string stato;
	m_settings->getStato() ? stato = "Vai online" : stato = "Vai offline";
	menu->Append(PU_STATO, stato);
	menu->AppendSeparator();
	menu->AppendSeparator();
	menu->Append(PU_EXIT, wxT("Esci"));

	return menu;
}