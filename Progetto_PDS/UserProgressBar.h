#pragma once

#include <atomic>

#include "utente.h"

#include <wx/wx.h>

#include "WindowProgressBar.h"


enum {
	CLIENT_EVENT = 5000,
	SERVER_EVENT,
	SetTimeFile_EVENT,
	SetMaxDir_EVENT,
	SetNewFile_EVENT,
	SetMaxFile_EVENT,
	IncFile_EVENT
};

class UserProgressBar : public wxWindow
{
private:
	class WindowProgressBar *m_parentWindow;
	wxButton *m_abort;
	wxStaticText* m_nameFile;
	//tempo e percentuali
	wxStaticText *m_percDir;
	wxStaticText *m_timeFile;
	wxStaticText *m_percFile;
	//barre avanzamento
	wxGauge *m_progDir;
	wxGauge *m_progFile;

	std::string m_utente;

	std::mutex mut;

	bool m_isDir;
	std::string m_File;
	long m_totDir=0, m_parzialeDir=0;
	long m_totFile=0, m_parzialeFile=0;
	long m_min=0, m_sec=0;
	std::atomic<bool> flagAbort;

	void OnAbortClick(wxCommandEvent& event);

	void OnSetTimeFile(wxThreadEvent& event) { SetTimeFile(event.GetExtraLong()); };
	
	void OnSetMaxDir(wxThreadEvent& event) { SetMaxDir(event.GetExtraLong()); };
	
	void OnSetNewFile(wxThreadEvent& event) { SetNewFile(event.GetString().ToStdString()); };
	
	void OnSetMaxFile(wxThreadEvent& event) { SetMaxFile(event.GetExtraLong()); };
	
	void OnIncFile(wxThreadEvent& event) { IncFile(event.GetExtraLong()); };
	
	void OnClientEvent(wxThreadEvent& event);
	wxDECLARE_EVENT_TABLE();
public:
	UserProgressBar(wxWindow* parent, wxWindowID id, std::string user, bool isDir);

	std::string GetUsername() { return m_utente; }
	WindowProgressBar* GetParent() { return m_parentWindow; }

	//void UpdateUI();

	//setta il tempo rimasto del file
	void SetTimeFile(long sec);

	//passare la dimensione del direttorio
	void SetMaxDir(long dim);

	//passare il file attualmente in trasferimento (da usare solo in modalità direttorio!)
	void SetNewFile(std::string path);

	//passare la dimensione del file
	void SetMaxFile(long dim);

	//passare la quantità di byte inviati
	void IncFile(long dim);

	//per testare se bisogna interrompere l'invio
	bool testAbort() { return flagAbort.load(); }
};