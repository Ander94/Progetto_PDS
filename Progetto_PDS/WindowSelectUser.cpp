#include <wx/wx.h>
#include "share_icon.xpm"
#include <list>
#include <memory>

#include "Settings.h"
//#include "WindowSelectUser.h"
//#include "WindowProgressBar.h"
#include "UserSizer.h"

enum {
	TIMER_ID = 10000
};


wxBEGIN_EVENT_TABLE(WindowSelectUser, wxFrame)
EVT_BUTTON(wxID_OK, WindowSelectUser::OnOk)
EVT_BUTTON(wxID_CANCEL, WindowSelectUser::OnCancel)
EVT_CLOSE(WindowSelectUser::OnCloseWindow)
EVT_TIMER(TIMER_ID, WindowSelectUser::OnTimer)
wxEND_EVENT_TABLE()

WindowSelectUser::WindowSelectUser(wxWindow* parent, Settings* settings) 
	: wxFrame(parent, wxID_ANY, wxT("Utenti disponibili"), wxDefaultPosition, wxDefaultSize, wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN)
{
	this->SetBackgroundColour(wxColour(240, 242, 245));
	this->SetFont(this->GetFont().Bold().Scale(0.9f));
	this->SetIcon(wxIcon(share_icon));
	m_settings = settings;
	m_frame = dynamic_cast<MainFrame*>(parent);
	m_ListaUtenti = std::list<UserSizer *>();  
	m_MappaInvio = std::map<std::string, utente>();  
	//Button ok
	wxBitmap* ok = new wxBitmap();
	ok->LoadFile(m_settings->getGeneralPath() + "bottoni\\invia.png", wxBITMAP_TYPE_ANY);
	wxBitmap* ok_hover = new wxBitmap();
	ok_hover->LoadFile(m_settings->getGeneralPath() + "bottoni\\invia_hover.png", wxBITMAP_TYPE_ANY);
	m_ok = new wxBitmapButton(this, wxID_OK, *ok,wxDefaultPosition, wxDefaultSize);
	m_ok->SetWindowStyle(wxNO_BORDER);
	m_ok->SetBackgroundColour(this->GetBackgroundColour());
	m_ok->SetBitmapHover(*ok_hover);
	m_ok->Disable();

	//Button cancel
	wxBitmap* cancel = new wxBitmap();
	cancel->LoadFile(m_settings->getGeneralPath() + "bottoni\\annulla.png", wxBITMAP_TYPE_ANY);
	wxBitmap* cancel_hover = new wxBitmap();
	cancel_hover->LoadFile(m_settings->getGeneralPath() + "bottoni\\annulla_hover.png", wxBITMAP_TYPE_ANY);
	m_cancel = new wxBitmapButton(this, wxID_CANCEL, *cancel, wxDefaultPosition, wxDefaultSize);
	m_cancel->SetWindowStyle(wxNO_BORDER);
	m_cancel->SetBackgroundColour(this->GetBackgroundColour());
	m_cancel->SetBitmapHover(*cancel_hover);
	m_timer = new wxTimer(this, TIMER_ID);
	int n=0;
	
	//inizializzazione barra stato
	CreateStatusBar(1); //TODO barra stato non funziona

	wxSizerFlags flags;
	flags.Border(wxALL, 5);

	wxSizer* sizerTop = new wxBoxSizer(wxVERTICAL);
	
	m_sizerUsers = new wxGridSizer(4, 5, 5);
	std::vector<utente> lista = m_settings->getUtentiOnline();
	if (lista.size() == 0) {
		n = 0;
		m_sizerUsers->AddSpacer(90);
	}
	else {
		//carico gli utenti connessi
		for (auto it : (m_settings->getUtentiOnline())) {
			UserSizer *u = new UserSizer(this, m_settings, it);
			m_ListaUtenti.push_back(u);
			m_sizerUsers->Add(u, 1, wxALIGN_CENTER);

		}
		n = m_ListaUtenti.size();
	}
	
	sizerTop->Add(m_sizerUsers);

	sizerTop->AddStretchSpacer()->SetMinSize(200, 20);

	wxSizer* const sizerBtns = new wxBoxSizer(wxHORIZONTAL);
	sizerBtns->Add(m_cancel, flags);
	sizerBtns->Add(m_ok, flags);

	sizerTop->Add(sizerBtns, flags.Align(wxALIGN_CENTER_HORIZONTAL));

	this->SetSizerAndFit(sizerTop);
	this->Show();
	this->Centre();

	if (n == 0)
		SetStatusText("In attesa di altri utenti...");
	else 
		SetStatusText("Seleziona uno o più utenti");

	m_timer->Start(UPDATE_WINDOW);

}

//distruttore non necessario, tutte le finestre figlie sono cancellate automaticamente
//WindowSelectUser::~WindowSelectUser() {
//	m_ListaUtenti.clear();
//}

//fa partire l'invio del file
void WindowSelectUser::OnOk(wxCommandEvent& event) {
	m_timer->Stop();
	WindowProgressBar *window = new WindowProgressBar(m_frame, m_settings, this->getListaInvio(), true);
	window->StartSending();
	this->Close();
}

void WindowSelectUser::OnCancel(wxCommandEvent& event) {
	wxMessageBox(wxT("Invio annullato."), wxT("Info"), wxOK | wxICON_INFORMATION, this);
	m_frame->Show();
	this->Close();
}

void WindowSelectUser::OnCloseWindow(wxCloseEvent& event) {
	//usare Destroy(), sennò va in loop (close chiama questo evento)!
	m_timer->Stop();
	this->Destroy();
}

void WindowSelectUser::OnTimer(wxTimerEvent& event) {
	UpdateUI();
}

void WindowSelectUser::UpdateUI() {
	std::vector<utente> lista = m_settings->getUtentiOnline();
	int n;
	if (lista.size() == 0) {
		n = 0;
		m_sizerUsers->Clear();
		int i = 0;
		for (auto it : m_ListaUtenti)
			it->Destroy();

		m_sizerUsers->AddSpacer(90);
		m_ListaUtenti.clear();
		m_MappaInvio.clear();
		m_ok->Disable();
	}
	else {
		if (m_ListaUtenti.size() == 0)
			m_sizerUsers->Remove(0); //tolgo lo spacer se presente
		std::vector<utente> tmp = this->getListaInvio();
		m_MappaInvio.clear();
		std::vector<UserSizer *> tmp2(m_ListaUtenti.begin(), m_ListaUtenti.end());
		m_ListaUtenti.clear();

		//ciclo per eliminare gli utenti non più presenti
		for (auto it : tmp2) {
			bool found = false;
			for (auto it2 = lista.begin(); it2 != lista.end(); it2++) {
				if (it->getIpAddr() == it2->getIpAddr()) { //TODO convertire a ip
					found = true;
					m_ListaUtenti.push_back(it);
					//"rieseguo" il click sugli utenti selezionati prima dell'aggiornamento
					for (auto it3 : tmp) {
						if (it3.getIpAddr() == it->getIpAddr()) {
							insertUtenteLista(it->getUser());
							break;
						}
					}
					lista.erase(it2);
					break;
				}
			}
			if (!found)
				it->Destroy();
		}

		//tutti gli utenti rimasti in lista vanno aggiunti
		for (auto it : lista) {	
			UserSizer *u = new UserSizer(this, m_settings, it);
			m_ListaUtenti.push_back(u);
			m_sizerUsers->Add(u, 1, wxALIGN_CENTER);
		}

		n = m_ListaUtenti.size();
	}

	this->Layout();
	this->Fit();
	if (n == 0)
		SetStatusText("In attesa di altri utenti...");
	else
		SetStatusText("Seleziona uno o più utenti");
	this->Show();
}

//aggiunge un utente alla lista di invio, da usare in UserSizer::OnUserClick
void WindowSelectUser::insertUtenteLista(utente user) {
	m_MappaInvio.insert(std::pair<std::string, utente>(user.getIpAddr(), user));
	m_ok->Enable();
}

//rimuove un utente alla lista di invio, da usare in UserSizer::OnUserClick
void WindowSelectUser::deleteUtenteLista(utente user) {
	m_MappaInvio.erase(user.getIpAddr());
	if (m_MappaInvio.size() == 0)
		m_ok->Disable();
}

//aggiunge un nuovo utente alla finestra
void WindowSelectUser::addUtente(utente user) {
	UserSizer *u = new UserSizer(this, m_settings, user);
	m_ListaUtenti.push_back(u);
	m_sizerUsers->Add(u, 1, wxALIGN_CENTER);
	
	this->Update();
}

//rimuove un utente dalla finsetra
void WindowSelectUser::removeUtente(utente user) {
	int i = 0;
	std::string ip = user.getIpAddr();

	for (auto it=m_ListaUtenti.begin(); it != m_ListaUtenti.end(); it++ ) {
		if ((*it)->getIpAddr() == ip) {
			this->m_ListaUtenti.erase(it);
			m_sizerUsers->Remove(i);
			break;
		}
		i++;
	}
	
	this->Update();
}

//da usare con il metodo WindowSelectUser::OnOk per passare la lista di utenti selezionati per l'invio
std::vector<utente> WindowSelectUser::getListaInvio()
{
	std::vector<utente> lista;
	for (auto it : m_MappaInvio)
		lista.push_back(it.second);

	return lista;
}


