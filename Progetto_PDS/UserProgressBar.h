#pragma once

#include <atomic>

#include "utente.h"

#include <wx/wx.h>

#include "WindowProgressBar.h"


enum {
	StartDir_EVENT = 5000,
	StartFile_EVENT,
	End_EVENT,
	SetTimeFile_EVENT,
	SetNewDir_EVENT,
	SetMaxDir_EVENT,
	SetNewFile_EVENT,
	SetMaxFile_EVENT,
	IncFile_EVENT
};



class UserProgressBar : public wxWindow
{
private:
	class WindowProgressBar *m_parentWindow;
	wxBitmapButton *m_abort;
	wxStaticText* m_nameFile;	//nome file
	wxStaticText* m_nameDir;	//nome cartella
	wxStaticText *m_timeDir;	//tempo mancante per trasferimento cartella
	wxStaticText *m_percDir;	//percentuale progresso invio cartella
	wxStaticText *m_timeFile;	//tempo mancante per trasferimento file
	wxStaticText *m_percFile;	//percentuale progresso invio cartella
	wxGauge *m_progDir;
	wxGauge *m_progFile;
	boost::posix_time::ptime m_startTime;	//tempo inizio invio

	std::string m_utente;
	std::string m_ipAddr;
	std::mutex mut;

	bool m_isDir;
	long long m_totFile=0, m_parzialeFile=0, m_totDir = 0, m_parzialeDir = 0;
	long m_min=0, m_sec=0;
	std::atomic<bool> flagAbort;
	long calcola_tempo;

	//l'utente ha interrotto il trasferimento
	void OnAbortClick(wxCommandEvent& event);

	//il thread segnala l'inizio effettivo del trasferimento
	void OnStartDir(wxThreadEvent& event) { 
		m_startTime = boost::posix_time::second_clock::local_time();
		m_abort->Enable();
	}

	void OnStartFile(wxThreadEvent& event){
		m_abort->Enable();
	}

	//il thread ha segnalato la fine del trasferimento
	void OnEndEvent(wxThreadEvent& event);	

	//setta il tempo mancante alla fine del trasferimento
	void OnSetTimeFile(wxThreadEvent& event) { SetTimeFile(event.GetPayload<long>()); };
	
	void OnSetNewDir(wxThreadEvent& event) { SetNewDir(event.GetString().ToStdString()); };

	void OnSetMaxDir(wxThreadEvent& event) { SetMaxDir(event.GetPayload<long long>()); };
	
	void OnSetNewFile(wxThreadEvent& event) { SetNewFile(event.GetString().ToStdString()); };
	
	void OnSetMaxFile(wxThreadEvent& event) { SetMaxFile(event.GetPayload<long long>()); };
	
	void OnIncFile(wxThreadEvent& event) { IncFile(event.GetPayload<long long>()); };

	void OnCloseWindow(wxCloseEvent&);

	wxDECLARE_EVENT_TABLE();
public:
	UserProgressBar(wxWindow* parent, wxWindowID id, std::string user, std::string ipAddr, bool isDir, std::string generalPath);

	std::string GetUsername() { return m_utente; }
	std::string GetIpAddr() { return m_ipAddr; }
	WindowProgressBar* GetParent() { return m_parentWindow; }

	//setta il tempo rimasto del file
	void SetTimeFile(long sec);

	//passare la dimensione del direttorio
	void SetMaxDir(long long dim);

	//passare il file attualmente in trasferimento (da usare solo in modalità direttorio!)
	void SetNewFile(std::string path);

	//passare la cartella attualmente in trasferimento (da usare solo in modalità direttorio!)
	void SetNewDir(std::string path);

	//passare la dimensione del file
	void SetMaxFile(long long dim);

	//passare la quantità di byte inviati
	void IncFile(long long dim);

	//per testare se bisogna interrompere l'invio
	bool testAbort() { return flagAbort.load(); }
};