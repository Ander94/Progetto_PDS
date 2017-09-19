#pragma once

#include <atomic>

#include "utente.h"

#include <wx/wx.h>

#include "WindowProgressBar.h"


enum {
	SERVER_EVENT = 5100,
	SetMaxDim_EVENT,
	Inc_EVENT
};

class FileInDownload : public wxWindow
{
private:
	class WindowDownload *m_parentWindow;
	wxBitmapButton *m_abort;
	wxStaticText* m_nameFile;
	wxStaticText *m_perc;

	std::string m_utente;
	std::string m_ipAddr;

	double long m_tot = 0, m_parziale;
	std::atomic<bool> flagAbort;

	void OnAbortClick(wxCommandEvent& event);	//l'utente ha interrotto il trasferimento

	void OnSetMaxDim(wxThreadEvent& event) { SetMaxDim(event.GetPayload<int>()); };

	void OnIncFile(wxThreadEvent& event) { IncFile(event.GetPayload<int>()); };

	wxDECLARE_EVENT_TABLE();
public:
	FileInDownload(wxWindow* parent, wxWindowID id, std::string user, std::string file, std::string generalPath);

	std::string GetUsername() { return m_utente; }
	std::string GetIpAddr() { return m_ipAddr; }
	WindowDownload* GetParent() { return m_parentWindow; }

	//passare la dimensione del direttorio
	void SetMaxDim(int dim);

	//passare la quantità di byte inviati
	void IncFile(int dim);

	//per testare se bisogna interrompere l'invio
	bool testAbort() { return flagAbort.load(); }
};





/*--------------------------------------------------------------------------------------------------------*/




class WindowDownload : public wxFrame
{
private:
	class Settings* m_settings;
	class MainFrame* m_frame;
	std::map <std::string, FileInDownload*> m_MappaDownload;	//utenti attualmente selezionati
	wxBitmapButton* m_ok;
	wxSizer* m_sizerDownload;
	int m_counter = 0;

	void OnOk(wxCommandEvent&);
	void OnMinimize(wxIconizeEvent&);
	void OnEndDownload(wxThreadEvent&);

	wxDECLARE_EVENT_TABLE();

public:
	WindowDownload(wxWindow* parent, Settings* settings);


	//void OnNewDownload(wxThreadEvent&);
	FileInDownload* newDownload(std::string user, std::string file);
	////aggiunge un nuovo utente alla finestra
	//void addUtente(utente user);
	////rimuove un utente dalla finsetra
	//void removeUtente(utente user);

};
