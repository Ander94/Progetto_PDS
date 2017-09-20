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
	wxStaticText* m_perc;

	std::string m_utente;
	std::string m_ipAddr;

	long long m_tot = 0, m_parziale;
	std::atomic<bool> flagAbort;

	void OnAbortClick(wxCommandEvent& event);	//l'utente ha interrotto il trasferimento

	void OnSetMaxDim(wxThreadEvent& event) { SetMaxDim(event.GetPayload<long long>()); };

	void OnIncFile(wxThreadEvent& event) { IncFile(event.GetPayload<long long>()); };

	wxDECLARE_EVENT_TABLE();
public:
	FileInDownload(wxWindow* parent, wxWindowID id, std::string user, std::string file, std::string generalPath);

	std::string GetUsername() { return m_utente; }
	std::string GetIpAddr() { return m_ipAddr; }
	WindowDownload* GetParent() { return m_parentWindow; }

	//passare la dimensione del direttorio
	void SetMaxDim(long long dim);

	//passare la quantità di byte inviati
	void IncFile(long long dim);

	//per testare se bisogna interrompere l'invio
	bool testAbort() { return flagAbort.load(); }
};





/*--------------------------------------------------------------------------------------------------------*/




class WindowDownload : public wxFrame
{
private:
	class Settings* m_settings;
	class MainFrame* m_frame;
	wxBitmapButton* m_ok;
	wxSizer* m_sizerDownload;
	int m_counter = 0;
	std::mutex m_mutex;

	void OnOk(wxCommandEvent&);
	void OnMinimize(wxIconizeEvent&);
	void OnEndDownload(wxThreadEvent&);

	wxDECLARE_EVENT_TABLE();

public:
	WindowDownload(wxWindow* parent, Settings* settings);

	FileInDownload* newDownload(std::string user, std::string file);
};
