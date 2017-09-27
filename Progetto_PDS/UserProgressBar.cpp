//NON COMMENTATO

#include <wx/statbmp.h>

#include "UserProgressBar.h"
#include "utente.h"

wxBEGIN_EVENT_TABLE(UserProgressBar, wxWindow)
EVT_THREAD(CLIENT_EVENT, UserProgressBar::OnClientEvent)
EVT_THREAD(SetTimeFile_EVENT, UserProgressBar::OnSetTimeFile)
EVT_THREAD(SetNewDir_EVENT, UserProgressBar::OnSetNewDir)
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
	m_startTime = boost::posix_time::second_clock::local_time();
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
		wxFlexGridSizer* hDirSizer = new wxFlexGridSizer(4);
		m_percDir = new wxStaticText(this, wxID_ANY, "    0", wxDefaultPosition, wxDefaultSize);
		m_nameDir = new wxStaticText
		(
			this,
			wxID_ANY,
			"Cartella: ",
			wxDefaultPosition,
			wxSize(400, 14),
			wxST_NO_AUTORESIZE | wxST_ELLIPSIZE_MIDDLE
		);
		//m_nameDir->SetMaxSize(wxSize(400, 50));
		m_timeDir = new wxStaticText(this, wxID_ANY, "calcolo in corso    ", wxDefaultPosition, wxDefaultSize);
		m_progDir = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxSize(400, 8), wxGA_HORIZONTAL | wxGA_SMOOTH);
		m_percDir->SetFont((m_percDir->GetFont()));
		hDirSizer->Add(
			new wxStaticText(this, wxID_ANY, "Tempo rimanente: ", wxDefaultPosition, wxDefaultSize),
			0,
			wxALIGN_LEFT);
		hDirSizer->Add(m_timeDir, 1, wxALIGN_LEFT);
		hDirSizer->Add(m_percDir, 1, wxALIGN_RIGHT);
		hDirSizer->Add(
			new wxStaticText(this, wxID_ANY, "%", wxDefaultPosition, wxDefaultSize),
			0,
			wxALIGN_RIGHT | wxLEFT,
			5);
		vDirSizer->Add(m_nameDir, 1, wxALIGN_LEFT);
		vDirSizer->Add(m_progDir, 1, wxALIGN_LEFT);
		vDirSizer->Add(hDirSizer, 1, wxEXPAND);

		//parte del file
		wxStaticBoxSizer* vFileSizer = new wxStaticBoxSizer(wxVERTICAL, this);
		wxFlexGridSizer* hFileSizer = new wxFlexGridSizer(4);
		m_percFile = new wxStaticText(this, wxID_ANY, "    0", wxDefaultPosition, wxDefaultSize);
		m_nameFile = new wxStaticText(this, wxID_ANY, "File: ", wxDefaultPosition, wxDefaultSize);
		m_timeFile = new wxStaticText(this, wxID_ANY, "calcolo in corso    ", wxDefaultPosition, wxDefaultSize);
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

		//metto insieme entrambe le parti
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
		m_timeFile = new wxStaticText(this, wxID_ANY, "calcolo in corso    ", wxDefaultPosition, wxDefaultSize);
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
	std::string s_hour;
	std::string s_min;
	std::string s_sec;

	long min = sec / 60;
	long hour = min / 60;
	min = min % 60;
	sec = sec % 60;

	//if (sec <= 9)
	//	s_sec = "0" +std::to_string(sec);
	//else
	//	s_sec = std::to_string(sec);

	//if (min <= 9)
	//	s_min = "0" + std::to_string(min);
	//else
	//	s_min = std::to_string(min);

	s_hour = std::to_string(hour);
	s_min = std::to_string(min);
	s_sec = std::to_string(sec);

	std::string time("");
	if (hour > 0) {
		time = s_hour + "h " + s_min + "min " + s_sec + "sec";
	}
	else if (min > 0) {
		time = s_min + "min " + s_sec + "sec";
	}
	else {
		time = s_sec + "sec";
	}

	if (m_timeFile->GetLabelText() != time)
		m_timeFile->SetLabel(time);
}

void UserProgressBar::SetNewDir(std::string path) {
	m_nameDir->SetLabelText("Cartella: " + path);
	m_nameDir->SetToolTip(path);
	Update();
}

void UserProgressBar::SetMaxDir(long long dim) {
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

void UserProgressBar::SetMaxFile(long long dim) {
	m_totFile = dim;
	m_parzialeFile = 0;
	m_percFile->SetLabelText("0");
	this->Update();
}

void UserProgressBar::IncFile(long long dim) {
	this->calcola_tempo++;
	long long diff = dim - m_parzialeFile;
	m_parzialeFile = dim;
	long long value = m_parzialeFile * 100 / m_totFile;
	int intval = (int)floor(value + 0.5);
	m_progFile->SetValue(intval);
	if (m_percFile->GetLabelText() != std::to_string(intval))
		m_percFile->SetLabelText(std::to_string(intval));

	if (m_isDir) {
		m_parzialeDir += diff;
		if (this->calcola_tempo%EVALUATE_TIME == 0){
		//tempo rimanente
		boost::posix_time::ptime curTime = boost::posix_time::second_clock::local_time();
		long dif = (curTime - m_startTime).total_seconds();
		long sec = (long int)((((m_totDir - m_parzialeDir) / (long double)((m_parzialeDir)))*dif));
		long min = sec / 60;
		long hour = min / 60;
		min = min % 60;
		sec = sec % 60;
		std::string s_min;
		std::string s_sec;
		std::string s_hour;
		
		//if (sec <= 9)
		//	s_sec = "0" + std::to_string(sec);
		//else
		//	s_sec = std::to_string(sec);

		//if (min <= 9)
		//	s_min = "0" + std::to_string(min);
		//else
		//	s_min = std::to_string(min);
		
		s_hour = std::to_string(hour);
		s_min = std::to_string(min);
		s_sec = std::to_string(sec);

		std::string time("");
		if (hour > 0) {
			time = s_hour + "h " + s_min + "min " + s_sec + "sec";
		} 
		else if (min > 0) {
			time = s_min + "min " + s_sec + "sec";
		}
		else {
			time = s_sec + "sec";
		}
		
		if (m_timeDir->GetLabelText() != time)
			m_timeDir->SetLabel(time);
	}
		//percentuale avanzamento
		value = m_parzialeDir * 100 / m_totDir;
		intval = (int)floor(value + 0.5);
		m_progDir->SetValue(intval);
		if (m_percDir->GetLabelText() != std::to_string(intval))
			m_percDir->SetLabelText(std::to_string(intval));
	}

	this->Update();
}