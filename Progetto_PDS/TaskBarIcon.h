#pragma once


//#include "utente.h"
#include "Settings.h"

#include <wx/wx.h>
#include <wx/taskbar.h>

#include "WindowProgressBar.h"
#include "WindowSaveFile.h"
#include "WindowSelectUser.h"
#include "ipcsetup.h"
//#include "IPCserver.h"
#include "IPCclient.h"
#include "Settings.h"
#include "client.h"
// ----------------------------------------------------------------------------
// Icona TaskBar
// ----------------------------------------------------------------------------

class TaskBarIcon : public wxTaskBarIcon
{
private:
	class Settings* m_settings;
public:
	TaskBarIcon(class Settings* settings) {
		m_settings = settings;
	}

	void OnLeftButtonDClick(wxTaskBarIconEvent&);
	void OnMenuRestore(wxCommandEvent&);
	void OnMenuExit(wxCommandEvent&);
	void OnMenuCheckmark(wxCommandEvent&);
	void OnMenuUICheckmark(wxUpdateUIEvent&);
	//MIO
	void OnMenuCheckmarkOnline(wxCommandEvent&);
	void OnMenuUICheckmarkOnline(wxUpdateUIEvent&);
	void OnMenuCheckmarkOffline(wxCommandEvent&);
	void OnMenuUICheckmarkOffline(wxUpdateUIEvent&);
	//
	void OnMenuSub(wxCommandEvent&);
	
	virtual wxMenu *CreatePopupMenu() wxOVERRIDE;

	wxDECLARE_EVENT_TABLE();
};


// ----------------------------------------------------------------------------
// Frame principale
// ----------------------------------------------------------------------------

class MainFrame : public wxFrame
{
private:
	Settings* m_settings;
	class WindowSelectUser* m_selectUser;
	MyClient* m_client;
	class MyServer* m_server;
	wxTimer* m_timer;
	wxTextCtrl* m_elencoUser;
	wxButton* m_changeImage;
	wxButton* m_changeSavePath;
	wxStaticBitmap* m_userImage;
	wxRadioBox* m_status;
	wxRadioBox* m_saved;
	wxStaticText* m_textStato;
	wxStaticText* m_textSavePath;

	void OnTimer(wxTimerEvent&);
	void OnImage(wxCommandEvent&);
	void OnChangeSavePath(wxCommandEvent&);
	void OnRadioBox(wxCommandEvent&);
	void OnMenuUICheckmark(wxUpdateUIEvent&);
public:
	MainFrame(const wxString& title, class Settings* settings);
	virtual ~MainFrame();
	bool StartServer();
	bool StartClient();
	MyServer *GetServer() { return m_server; }
	MyClient *GetClient() { return m_client; }
	Settings *GetSettings() { return m_settings; };

	void SendFile(std::string path);
	void MainFrame::showBal(std::string title, std::string message);
protected:
	void OnAbout(wxCommandEvent& event);
	void OnOK(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnCloseWindow(wxCloseEvent& event);


	TaskBarIcon *m_taskBarIcon;

	wxDECLARE_EVENT_TABLE();
};

