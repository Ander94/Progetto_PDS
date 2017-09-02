#include <wx/statbmp.h>

#include "UserProgressBar.h"
#include "utente.h"

wxBEGIN_EVENT_TABLE(UserProgressBar, wxWindow)
EVT_THREAD(CLIENT_EVENT, UserProgressBar::OnClientEvent)
EVT_THREAD(SetTimeFile_EVENT, UserProgressBar::OnSetTimeFile)
EVT_THREAD(SetMaxDir_EVENT, UserProgressBar::OnSetMaxDir)
EVT_THREAD(SetNewFile_EVENT, UserProgressBar::OnSetNewFile)
EVT_THREAD(SetMaxFile_EVENT, UserProgressBar::OnSetMaxFile)
EVT_THREAD(IncFile_EVENT, UserProgressBar::OnIncFile)
wxEND_EVENT_TABLE()

UserProgressBar::UserProgressBar(wxWindow* parent, wxWindowID id, std::string user, bool isDir) : wxWindow(parent, id)
{
	flagAbort.store(false);
	m_parentWindow = dynamic_cast<WindowProgressBar*>(parent);
	m_utente = user;
	m_isDir = isDir;
	m_File = "File: ";
	wxBoxSizer* m_sizer = new wxBoxSizer(wxVERTICAL);
	
	wxStaticText *username = new wxStaticText(this, wxID_ANY, m_utente, wxDefaultPosition, wxDefaultSize);
	username->SetFont((username->GetFont()).Bold().Larger());
	m_sizer->Add(username, 0, wxALIGN_LEFT | wxALL, 3);
	m_abort = new wxButton(this, wxID_CANCEL);

	if (isDir) {
		//parte della cartella
		wxStaticBoxSizer* vDirSizer = new wxStaticBoxSizer(wxVERTICAL, this);
		wxGridSizer* hDirSizer = new wxGridSizer(2);
		m_percDir = new wxStaticText(this, wxID_ANY, "0 %", wxDefaultPosition, wxDefaultSize);
		wxStaticText* m_Dir = new wxStaticText(this, wxID_ANY, "Avanzamento complessivo: ", wxDefaultPosition, wxDefaultSize);
		m_percDir->SetFont((m_percDir->GetFont()));
		hDirSizer->Add(m_Dir, 1, wxALIGN_LEFT);
		hDirSizer->Add(m_percDir, 1, wxALIGN_RIGHT);
		vDirSizer->Add(hDirSizer, 1, wxEXPAND);
		m_progDir = new wxGauge(this, wxID_ANY, 1000, wxDefaultPosition, wxSize(400, 8), wxGA_HORIZONTAL | wxGA_SMOOTH);
		//m_progDir->Pulse();

		vDirSizer->Add(m_progDir, 1, wxALIGN_LEFT);

		//parte del file
		wxStaticBoxSizer* vFileSizer = new wxStaticBoxSizer(wxVERTICAL, this);
		wxGridSizer* hFileSizer = new wxGridSizer(2);
		m_percFile = new wxStaticText(this, wxID_ANY, "0 %", wxDefaultPosition, wxDefaultSize);
		//m_percFile->SetFont((m_percFile->GetFont()));
		m_nameFile = new wxStaticText(this, wxID_ANY, "File: ", wxDefaultPosition, wxDefaultSize);
		//m_nameFile->SetFont((m_percFile->GetFont()));
		m_timeFile = new wxStaticText(this, wxID_ANY, "Calcolo del tempo rimanente", wxDefaultPosition, wxDefaultSize);
		//m_timeFile->SetFont((m_timeFile->GetFont()));
		m_progFile = new wxGauge(this, wxID_ANY, 1000, wxDefaultPosition, wxSize(400, 8), wxGA_HORIZONTAL | wxGA_SMOOTH);
		//m_progFile->Pulse();
		vFileSizer->Add(m_nameFile, 1, wxALIGN_LEFT);
		vFileSizer->Add(m_progFile, 1, wxALIGN_LEFT);
		hFileSizer->Add(m_timeFile, 1, wxALIGN_LEFT);
		hFileSizer->Add(m_percFile, 1, wxALIGN_RIGHT);
		vFileSizer->Add(hFileSizer, 1, wxEXPAND);

		wxBoxSizer* vSizer = new wxBoxSizer(wxVERTICAL);
		vSizer->Add(vDirSizer);
		vSizer->Add(vFileSizer);

		wxBoxSizer* hSizer = new wxBoxSizer(wxHORIZONTAL);
		hSizer->Add(vSizer, 1);
		hSizer->Add(m_abort, 0, wxALIGN_BOTTOM | wxLEFT | wxRIGHT, 3);

		m_sizer->Add(hSizer, 1, wxALL, 3);

	}
	else {
		wxStaticBoxSizer* vFileSizer = new wxStaticBoxSizer(wxVERTICAL, this);
		wxGridSizer* hFileSizer = new wxGridSizer(2);
		m_percFile = new wxStaticText(this, wxID_ANY, "0 %", wxDefaultPosition, wxDefaultSize);
		//m_percFile->SetFont((m_percFile->GetFont()));
		m_timeFile = new wxStaticText(this, wxID_ANY, "Calcolo del tempo rimanente", wxDefaultPosition, wxDefaultSize);
		//m_timeFile->SetFont((m_percFile->GetFont()));
		m_progFile = new wxGauge(this, wxID_ANY, 1000, wxDefaultPosition, wxSize(400, 8), wxGA_HORIZONTAL | wxGA_SMOOTH);
		hFileSizer->Add(m_timeFile, 1, wxALIGN_LEFT);
		hFileSizer->Add(m_percFile, 1, wxALIGN_RIGHT);	
		vFileSizer->Add(hFileSizer, 1, wxEXPAND);
		vFileSizer->Add(m_progFile, 1, wxALIGN_LEFT);
		
		wxBoxSizer* hSizer = new wxBoxSizer(wxHORIZONTAL);
		hSizer->Add(vFileSizer, 1);
		hSizer->Add(m_abort, 0, wxALIGN_BOTTOM | wxLEFT | wxRIGHT, 3);

		m_sizer->Add(hSizer, 1, wxALL, 3);
	}
	
	m_abort->Bind(wxEVT_BUTTON, &UserProgressBar::OnAbortClick, this);	

	this->SetSizerAndFit(m_sizer);
}

//interrompe trasferimento
void UserProgressBar::OnAbortClick(wxCommandEvent& event) {
	//wxMessageBox(wxT("Invio interrotto qui!"), wxT("Attenzione"), wxOK | wxICON_EXCLAMATION, this);
	flagAbort.store(true);
}

//void UserProgressBar::UpdateUI() {
//	std::string time("Tempo rimanente: " + std::to_string(m_min) + ":" + std::to_string(m_sec));
//	m_timeFile->SetLabelText(time);
//
//	m_progFile->SetValue((int)(m_parzialeFile*1000/m_totFile));
//	m_percFile->SetLabelText("Progresso: " + std::to_string(m_parzialeFile/m_totFile*100) + "%");
//	if (m_isDir) {
//		m_progDir->SetValue((int)(m_parzialeDir*1000/m_totDir));
//		m_percDir->SetLabelText("Progresso: " + std::to_string(m_parzialeDir/m_totDir*100) + "%");
//		if (m_nameFile->GetLabelText().ToStdString() !=  "File: " + m_File)
//			m_nameFile->SetLabelText("File: " + m_File);
//	}
//}

//per la fine del trasferimento
void UserProgressBar::OnClientEvent(wxThreadEvent & event) {
	//wxMessageBox(wxT("Trasferimento terminato!"), wxT("Attenzione"), wxOK | wxICON_EXCLAMATION, this);
	m_abort->Disable();
	this->m_parentWindow->decreseCountUtenti();
}



//setta il tempo mancante alla fine del trasferimento
void UserProgressBar::SetTimeFile(long sec) {
	std::string s_min;
	std::string s_sec;

	long min = sec / 60;
	sec = sec % 60;

	if (sec <= 9)
		s_sec = "0" +std::to_string(sec);
	else
		s_sec = std::to_string(sec);

	if (min <= 9)
		s_min = "0" + std::to_string(min);
	else
		s_min = std::to_string(min);

	
		std::string time("Tempo rimanente: " + s_min + ":" + s_sec);
		m_timeFile->SetLabelText(time);
	
	
}

void UserProgressBar::SetMaxDir(long dim) {
	if (dim <= 0) {
		m_percDir->SetLabelText("??? %");
		m_progDir->Pulse();
		return;
	}
	m_totDir = dim;
	m_parzialeDir = 0;
	m_progDir->SetValue(0);
	m_percDir->SetLabelText("0 %");
}

void UserProgressBar::SetNewFile(std::string path) {
	m_File = path;
	m_nameFile->SetLabelText("File: " + path);
	m_progFile->SetValue(0);
	this->Layout();
}

void UserProgressBar::SetMaxFile(long dim) {
	m_totFile = dim;
	m_parzialeFile = 0;
	m_percFile->SetLabelText("0 %");
	this->Layout();
}

void UserProgressBar::IncFile(long dim) {
	long diff = dim - m_parzialeFile;
	m_parzialeFile = dim;
	float value = (float)m_parzialeFile * 1000 / (float)m_totFile;
	int intval = (int)floor(value + 0.5);
	m_progFile->SetValue(intval);
	m_percFile->SetLabelText(std::to_string(intval/10) + "%");
	if (m_isDir && m_totDir > 0) {
		m_parzialeDir += diff;
		value = (float)m_parzialeDir * 1000 / (float)m_totDir;
		intval = (int)floor(value + 0.5);
		m_progDir->SetValue(intval);
		m_percDir->SetLabelText(std::to_string(intval/10) + "%");
	}
	this->Layout();
}