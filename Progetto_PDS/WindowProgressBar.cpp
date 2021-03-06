//NON COMMENTATO
#include <wx/wx.h>
#include <wx/statline.h>
#include <list>

#include "client.h"

#include "WindowProgressBar.h"
#include "UserProgressBar.h"
#include "TaskBarIcon.h"

#include "share_icon.xpm"


wxBEGIN_EVENT_TABLE(WindowProgressBar, wxFrame)
EVT_CLOSE(WindowProgressBar::OnCloseWindow)
wxEND_EVENT_TABLE()

WindowProgressBar::WindowProgressBar(wxWindow* parent, Settings* settings, std::vector<utente> listaUtenti, std::string sendPath, bool isDir)
	: wxFrame(parent, wxID_ANY, wxT("Trasferimenti in corso"), wxDefaultPosition, wxDefaultSize, wxMINIMIZE_BOX | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN)
{
	this->SetIcon(wxIcon(share_icon));
	this->SetBackgroundColour(wxColour(240, 242, 245));
	this->SetFont(this->GetFont().Bold().Scale(0.9f));
	m_frame = dynamic_cast<MainFrame*>(parent);
	m_settings = settings;
	m_CountUtenti = 0;
	m_sendPath = sendPath;

	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	
	/*
		Creazione barre di avanzamento e inserimento nel sizer
	*/
	for (auto it : listaUtenti) {
		if (m_CountUtenti!=0)
			topSizer->Add(new wxStaticLine(this));

		UserProgressBar *u = new UserProgressBar(this, wxID_ANY, it.getUsername(), it.getIpAddr(), isDir, m_settings->getGeneralPath());
		m_ListaUtenti.push_back(u);
		topSizer->Add(u, 1, wxEXPAND);
		this->m_CountUtenti++;
	}

	SetSizerAndFit(topSizer);
}

void WindowProgressBar::StartSending() {
	Show(true);
	Centre();
	SetFocus();
	for (auto it : m_ListaUtenti) {
		sendTCPfile(m_settings->getUtenteProprietario(), it->GetIpAddr(), m_sendPath, it);
	}
}

void WindowProgressBar::decreseCountUtenti() {
	this->m_CountUtenti--;
	if (this->m_CountUtenti == 0) {
		this->Close();
	}
}

void WindowProgressBar::OnCloseWindow(wxCloseEvent& event) {
	if (m_CountUtenti != 0) {
		for (auto it : m_ListaUtenti) {
			it->Abort();
		}
		event.Veto();
		Show(false);
		return;
	}
	Destroy();
}


