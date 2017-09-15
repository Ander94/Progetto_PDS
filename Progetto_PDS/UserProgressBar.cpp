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

UserProgressBar::UserProgressBar(wxWindow* parent, wxWindowID id, std::string user, std::string ipAddr, bool isDir, std::string generalPath) : wxWindow(parent, id, wxDefaultPosition, wxDefaultSize, wxNO_FULL_REPAINT_ON_RESIZE)
{
	flagAbort.store(false);
	m_parentWindow = dynamic_cast<WindowProgressBar*>(parent);
	m_utente = user;
	m_ipAddr = ipAddr;
	m_isDir = isDir;
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	
	wxStaticText *username = new wxStaticText(this, wxID_ANY, m_utente, wxDefaultPosition, wxDefaultSize);
	username->SetFont((username->GetFont()).Bold().Larger());
	topSizer->Add(username, 0, wxALIGN_LEFT | wxALL, 3);

	wxBitmap* cancel = new wxBitmap();
	cancel->LoadFile(generalPath + "bottoni\\annulla.png", wxBITMAP_TYPE_ANY);
	wxBitmap* cancel_hover = new wxBitmap();
	cancel_hover->LoadFile(generalPath + "bottoni\\annulla_hover.png", wxBITMAP_TYPE_ANY);
	m_abort = new wxBitmapButton(this, wxID_CANCEL, *cancel, wxDefaultPosition, wxDefaultSize);
	m_abort->SetWindowStyle(wxNO_BORDER);
	m_abort->SetBackgroundColour(this->GetBackgroundColour());
	m_abort->SetBitmapHover(*cancel_hover);

	if (isDir) {
		//parte della cartella
		wxStaticBoxSizer* vDirSizer = new wxStaticBoxSizer(wxVERTICAL, this);
		wxFlexGridSizer* hDirSizer = new wxFlexGridSizer(3);
		m_percDir = new wxStaticText(this, wxID_ANY, "0   ", wxDefaultPosition, wxDefaultSize);
		wxStaticText* m_Dir = new wxStaticText(this, wxID_ANY, "Avanzamento complessivo: ", wxDefaultPosition, wxDefaultSize);
		m_percDir->SetFont((m_percDir->GetFont()));
		hDirSizer->Add(m_Dir, 1, wxALIGN_LEFT | wxRIGHT, 35);
		hDirSizer->Add(m_percDir, 1, wxALIGN_RIGHT);
		hDirSizer->Add(
			new wxStaticText(this, wxID_ANY, "%", wxDefaultPosition, wxDefaultSize),
			0,
			wxALIGN_RIGHT | wxLEFT,
			5);

		vDirSizer->Add(hDirSizer, 1, wxEXPAND);
		m_progDir = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxSize(400, 8), wxGA_HORIZONTAL | wxGA_SMOOTH);

		vDirSizer->Add(m_progDir, 1, wxALIGN_LEFT);

		//parte del file
		wxStaticBoxSizer* vFileSizer = new wxStaticBoxSizer(wxVERTICAL, this);
		wxFlexGridSizer* hFileSizer = new wxFlexGridSizer(4);
		m_percFile = new wxStaticText(this, wxID_ANY, "    0", wxDefaultPosition, wxDefaultSize);
		m_nameFile = new wxStaticText(this, wxID_ANY, "File: ", wxDefaultPosition, wxDefaultSize);
		m_timeFile = new wxStaticText(this, wxID_ANY, "calcolo in corso", wxDefaultPosition, wxDefaultSize);
		m_progFile = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxSize(400, 8), wxGA_HORIZONTAL | wxGA_SMOOTH);
		vFileSizer->Add(m_nameFile, 1, wxALIGN_LEFT);
		vFileSizer->Add(m_progFile, 1, wxALIGN_LEFT);
		hFileSizer->Add(
			new wxStaticText(this, wxID_ANY, "Tempo rimanente: ", wxDefaultPosition, wxDefaultSize),
			0,
			wxALIGN_LEFT);
		hFileSizer->Add(m_timeFile, 1, wxALIGN_LEFT);
		hFileSizer->Add(m_percFile, 1, wxALIGN_RIGHT);
		hFileSizer->Add(
			new wxStaticText(this, wxID_ANY, "%", wxDefaultPosition, wxDefaultSize),
			0,
			wxALIGN_RIGHT | wxLEFT,
			5);
		vFileSizer->Add(hFileSizer, 1, wxEXPAND);

		wxBoxSizer* vSizer = new wxBoxSizer(wxVERTICAL);
		vSizer->Add(vDirSizer);
		vSizer->Add(vFileSizer);

		wxBoxSizer* hSizer = new wxBoxSizer(wxHORIZONTAL);
		hSizer->Add(vSizer, 1);
		hSizer->Add(m_abort, 0, wxALIGN_BOTTOM | wxLEFT | wxRIGHT, 3);

		topSizer->Add(hSizer, 1, wxALL, 3);

	}
	else {
		wxStaticBoxSizer* vFileSizer = new wxStaticBoxSizer(wxVERTICAL, this);
		wxFlexGridSizer* hFileSizer = new wxFlexGridSizer(4);
		m_percFile = new wxStaticText(this, wxID_ANY, "    0", wxDefaultPosition, wxDefaultSize);
		m_timeFile = new wxStaticText(this, wxID_ANY, "calcolo in corso", wxDefaultPosition, wxDefaultSize);
		m_progFile = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxSize(400, 8), wxGA_HORIZONTAL | wxGA_SMOOTH);
		hFileSizer->Add(
				new wxStaticText(this, wxID_ANY, "Tempo rimanente: ", wxDefaultPosition, wxDefaultSize),
				0,
				wxALIGN_LEFT);
		hFileSizer->Add(m_timeFile, 1, wxALIGN_LEFT);
		hFileSizer->Add(m_percFile, 1, wxALIGN_RIGHT);	
		hFileSizer->Add(
			new wxStaticText(this, wxID_ANY, "%", wxDefaultPosition, wxDefaultSize),
			0,
			wxALIGN_RIGHT | wxLEFT,
			5);
		vFileSizer->Add(hFileSizer, 1, wxEXPAND);
		vFileSizer->Add(m_progFile, 1, wxALIGN_LEFT);
		
		wxBoxSizer* hSizer = new wxBoxSizer(wxHORIZONTAL);
		hSizer->Add(vFileSizer, 1);
		hSizer->Add(m_abort, 0, wxALIGN_BOTTOM | wxLEFT | wxRIGHT, 3);

		topSizer->Add(hSizer, 1, wxALL, 3);
	}
	
	m_abort->Bind(wxEVT_BUTTON, &UserProgressBar::OnAbortClick, this);	

	this->SetSizerAndFit(topSizer);
}

//interrompe trasferimento
void UserProgressBar::OnAbortClick(wxCommandEvent& event) {
	flagAbort.store(true);
}

void UserProgressBar::OnClientEvent(wxThreadEvent & event) {
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

	std::string time(s_min + ":" + s_sec);
	if (m_timeFile->GetLabelText() != time)
		m_timeFile->SetLabelText(time);	
}

void UserProgressBar::SetMaxDir(double long dim) {
	m_totDir = dim;
	m_parzialeDir = 0;
	m_progDir->SetValue(0);
	m_percDir->SetLabelText("0");
}

void UserProgressBar::SetNewFile(std::string path) {
	m_nameFile->SetLabelText("File: " + path);
	m_progFile->SetValue(0);
	this->Update();
}

void UserProgressBar::SetMaxFile(double long dim) {
	m_totFile = dim;
	m_parzialeFile = 0;
	m_percFile->SetLabelText("0");
	this->Update();
}

void UserProgressBar::IncFile(double long dim) {
	double long diff = dim - m_parzialeFile;
	m_parzialeFile = dim;
	double long value = m_parzialeFile * 100 / m_totFile;
	int intval = (int)floor(value + 0.5);
	m_progFile->SetValue(intval);
	if (m_percFile->GetLabelText() != std::to_string(intval))
		m_percFile->SetLabelText(std::to_string(intval));

	if (m_isDir) {
		m_parzialeDir += diff;
		value = m_parzialeDir * 100 / m_totDir;
		intval = (int)floor(value + 0.5);
		m_progDir->SetValue(intval);
		if (m_percDir->GetLabelText() != std::to_string(intval))
			m_percDir->SetLabelText(std::to_string(intval));
	}

	this->Update();
}