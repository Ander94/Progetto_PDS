#include <wx/wx.h>
#include <wx/statline.h>
#include <list>

#include "client.h"

#include "WindowProgressBar.h"
#include "UserProgressBar.h"
#include "TaskBarIcon.h"


wxBEGIN_EVENT_TABLE(WindowProgressBar, wxFrame)
EVT_CLOSE(WindowProgressBar::OnCloseWindow)
wxEND_EVENT_TABLE()




WindowProgressBar::WindowProgressBar(wxWindow* parent, Settings* settings, std::unique_ptr<std::vector<utente>> listaUtenti, bool isSending)
	: wxFrame(parent, wxID_ANY, wxT("Trasferimenti in corso"), wxDefaultPosition, wxDefaultSize)
{
	m_frame = dynamic_cast<MainFrame*>(parent);
	m_settings = settings;
	wxBoxSizer *m_sizer = new wxBoxSizer(wxVERTICAL);
	m_CountUtenti = 0;
	m_isSending = isSending;

	/*UserProgressBar *mario = new UserProgressBar(this, wxID_ANY, "Mario", false);
	m_sizer->Add(mario, 1, wxEXPAND);
	UserProgressBar *luca = new UserProgressBar(this, wxID_ANY, "Luca", false);
	m_sizer->Add(mario, 1, wxEXPAND);
	UserProgressBar *maria = new UserProgressBar(this, wxID_ANY, "Maria", false);
	m_sizer->Add(mario, 1, wxEXPAND);*/
	
	//creo le barre di avanzamento
	for (auto it : *listaUtenti) {
		if (m_CountUtenti!=0)
			m_sizer->Add(new wxStaticLine(this));

		UserProgressBar *u = new UserProgressBar(this, wxID_ANY, it.getUsername(), it.getIpAddr(),m_settings->getIsDir());
		m_ListaUtenti.push_back(u);
		m_sizer->Add(u, 1, wxEXPAND);
		this->m_CountUtenti++;
	}

	//botSizerH->Add(m_cancel);
	//botSizerH->Add(m_ok, 0, wxLEFT, 5);
	//m_ok->SetDefault();
	//botSizer->Add(botSizerH, 0, wxALIGN_RIGHT);

	//m_sizer->Add(m_topSizer, 1, wxALL, 5);
	//m_sizer->Add(botSizer, 0, wxEXPAND | wxRIGHT | wxBOTTOM, 5);

	this->SetSizerAndFit(m_sizer);
	this->Centre();
	this->Show();
}

void WindowProgressBar::StartSending() {
	for (auto it : m_ListaUtenti) {
		sendTCPfile(m_settings->getUtenteProprietario(), it->GetIpAddr(), m_settings->getSendPath(), it);
	}
		
}

void WindowProgressBar::decreseCountUtenti() {
	//Cosi è sbagliato, bisogna chiudere solamente la finestra dell'utente di riferimento
	this->m_CountUtenti--;
	if (this->m_CountUtenti == 0) {
		//wxMessageBox(wxT("Tutti i trasferimenti terminati!"), wxT("Info"), wxOK | wxICON_INFORMATION, this);
		this->Close();
	}
}

void WindowProgressBar::OnCloseWindow(wxCloseEvent& event) {
	//usare Destroy(), sennò va in loop (close chiama questo evento)!
	this->Destroy();
}


