#include <wx/wx.h>
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
	: wxFrame(parent, wxID_ANY, wxT("Utenti disponibili"), wxDefaultPosition, wxDefaultSize)
{
	m_settings = settings;
	m_frame = dynamic_cast<MainFrame*>(parent);
	m_ListaUtenti = std::list<UserSizer *>();  //utenti attualmente presenti nella finestra
	m_MappaInvio = std::map<std::string, utente>();  //utenti attualmente selezionati
	m_sizer = new wxFlexGridSizer(1);
	m_topSizer = new wxGridSizer(4, 5, 5);
	m_botSizer = new wxBoxSizer(wxVERTICAL);
	m_botSizerH = new wxBoxSizer(wxHORIZONTAL);
	m_ok = new wxButton(this, wxID_OK, "OK");
	m_cancel = new wxButton(this, wxID_CANCEL, "CANCEL");
	m_timer = new wxTimer(this, TIMER_ID);
	int n=0;
	
	//inizializzazione barra stato
	//int l[] = { 200 };
	CreateStatusBar(1); //TODO barra stato non funziona
	//SetStatusWidths(1, l);

	m_botSizerH->Add(m_cancel, 1);
	m_botSizerH->Add(m_ok, 0, wxLEFT, 5);
	m_ok->SetDefault();
	m_botSizer->Add(m_botSizerH, 1, wxALIGN_RIGHT);

	m_sizer->Add(m_topSizer, 1, wxALL, 5);
	m_sizer->Add(m_botSizer, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 5);


	/*utente u("leonardo", "");
	this->addUtente(u);*/

	std::vector<utente> lista = m_settings->getUtentiOnline();
	if (lista.size() == 0) {
		n = 0;
		m_topSizer->AddSpacer(100);
		m_ok->Disable();
	}
	else {
		//carico gli utenti connessi
		for (auto it : (m_settings->getUtentiOnline())) {
				UserSizer *u = new UserSizer(this, m_settings, it);
				m_ListaUtenti.push_back(u);
				m_topSizer->Add(u, 1, wxALIGN_CENTER);
			
		}
		n = m_ListaUtenti.size();
	}
	

	this->SetSizerAndFit(m_sizer);
	this->Show();
	this->Centre();

	if (n == 0)
		SetStatusText("Nessun utente è attualmente connesso.");
	else
		SetStatusText("Utenti connessi: " + n);

	m_timer->Start(1000); //2 sec

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
		m_topSizer->Clear();
		int i = 0;
		for (auto it : m_ListaUtenti)
			it->Destroy();
		
		m_topSizer->AddSpacer(100);
		m_ListaUtenti.clear();
		m_MappaInvio.clear();
		m_ok->Disable();
	}
	else {
		if (m_ListaUtenti.size() == 0)
			m_topSizer->Remove(0); //tolgo lo spacer se presente
		std::unique_ptr<std::vector<utente>> tmp = this->getListaInvio();
		m_MappaInvio.clear();
		std::vector<UserSizer *> tmp2(m_ListaUtenti.begin(), m_ListaUtenti.end());
		m_ListaUtenti.clear();

		//ciclo per eliminare gli utenti non più presenti
		for (auto it : tmp2) {
			bool found = false;
			for (auto it2 = lista.begin(); it2 != lista.end(); it2++) {
				if (it->getUsername() == it2->getUsername()) { //TODO convertire a ip
					found = true;
					m_ListaUtenti.push_back(it);
					//"rieseguo" il click sugli utenti selezionati prima dell'aggiornamento
					for (auto it3 : *tmp) {
						if (it3.getUsername() == it->getUsername()) {
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
			m_topSizer->Add(u, 1, wxALIGN_CENTER);
			////rieseguo il click sugli utenti selezionati prima dell'aggiornamento
			//for (auto it2 : *tmp) {
			//	if (it2.getUsername() == it.getUsername()) {
			//		u->PerformClick();
			//		break;
			//	}
			//}
		}
		tmp.reset();
		n = m_ListaUtenti.size();
		m_ok->Enable();
	}

	this->Layout();
	this->Fit();
	if (n==0)
		SetStatusText("Nessun utente è attualmente connesso.");
	else
		SetStatusText("Utenti connessi: " + n);
	this->Show();
}

//aggiunge un utente alla lista di invio, da usare in UserSizer::OnUserClick
void WindowSelectUser::insertUtenteLista(utente user) {
	m_MappaInvio.insert(std::pair<std::string, utente>(user.getUsername(), user));
}

//rimuove un utente alla lista di invio, da usare in UserSizer::OnUserClick
void WindowSelectUser::deleteUtenteLista(utente user) {
	m_MappaInvio.erase(user.getUsername());
}

//aggiunge un nuovo utente alla finestra
void WindowSelectUser::addUtente(utente user) {
	UserSizer *u = new UserSizer(this, m_settings, user);
	m_ListaUtenti.push_back(u);
	m_topSizer->Add(u, 1, wxALIGN_CENTER);
	
	//da rimuovere
	this->Layout();
	this->Fit();
	this->Show();
}

//rimuove un utente dalla finsetra
void WindowSelectUser::removeUtente(utente user) {
	int i = 0;
	//std::string ip = user.getIpAddr();
	std::string name = user.getUsername();

	for (auto it=m_ListaUtenti.begin(); it != m_ListaUtenti.end(); it++ ) {
		//if ((*it)->getIpAddr() == ip) {
		if ((*it)->getUsername() == name) {
			this->m_ListaUtenti.erase(it);
			m_topSizer->Remove(i);
			break;
		}
		i++;
	}
	
	//da rimuovere
	this->Layout();
	this->Fit();
	this->Show();
}

//da usare con il metodo WindowSelectUser::OnOk per passare la lista di utenti 
//selezionati per l'invio
std::unique_ptr<std::vector<utente>> WindowSelectUser::getListaInvio()
{
	std::unique_ptr<std::vector<utente>> lista(new std::vector<utente>());
	for (auto it : m_MappaInvio)
		lista->push_back(it.second);

	return std::move(lista);
}


