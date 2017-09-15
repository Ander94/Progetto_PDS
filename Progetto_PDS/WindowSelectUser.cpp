#include <wx/wx.h>
#include <list>

#include "Settings.h"
#include "UserSizer.h"

#include "share_icon.xpm"

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
	this->SetIcon(wxIcon(share_icon));
	m_settings = settings;
	m_frame = dynamic_cast<MainFrame*>(parent);
	m_ListaUtenti = std::list<UserSizer *>();  
	m_MappaInvio = std::map<std::string, utente>();  
	m_ok = new wxButton(this, wxID_OK, "OK");
	m_ok->SetDefault();
	m_ok->Disable();
	m_cancel = new wxButton(this, wxID_CANCEL, "CANCEL");
	m_timer = new wxTimer(this, TIMER_ID);
	int n=0;
	
	CreateStatusBar(1);
	
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
	

	/*
	GERARCHIA SIZER E CONTROLLI
		
	+sizerTop
		+m_sizerUsers
			(0..*) UserSizer 
		spazio
		+sizerBtns
			m_cancel
			m_flags
	*/

	wxSizerFlags flags;
	flags.Border(wxALL, 5);
	wxSizer* sizerTop = new wxBoxSizer(wxVERTICAL);
	wxSizer* const sizerBtns = new wxBoxSizer(wxHORIZONTAL);

	sizerBtns->Add(m_cancel, flags);
	sizerBtns->Add(m_ok, flags);

	sizerTop->Add(m_sizerUsers);
	sizerTop->AddStretchSpacer()->SetMinSize(200, 20);
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
	//usare Destroy(), non Close(), sennò va in loop (close chiama questo evento)!
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
				if (it->getIpAddr() == it2->getIpAddr()) {
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

void WindowSelectUser::insertUtenteLista(utente user) {
	m_MappaInvio.insert(std::pair<std::string, utente>(user.getIpAddr(), user));
	m_ok->Enable();
}

void WindowSelectUser::deleteUtenteLista(utente user) {
	m_MappaInvio.erase(user.getIpAddr());
	if (m_MappaInvio.size() == 0)
		m_ok->Disable();
}

void WindowSelectUser::addUtente(utente user) {
	UserSizer *u = new UserSizer(this, m_settings, user);
	m_ListaUtenti.push_back(u);
	m_sizerUsers->Add(u, 1, wxALIGN_CENTER);
	
	Update();
}

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
	
	Update();
}

std::vector<utente> WindowSelectUser::getListaInvio()
{
	std::vector<utente> lista;
	for (auto it : m_MappaInvio)
		lista.push_back(it.second);

	return lista;
}


