#pragma once


//#include "utente.h"

#include <wx/wx.h>
#include <wx/taskbar.h>

#include "WindowProgressBar.h"
#include "WindowSaveFile.h"
#include "WindowSelectUser.h"
#include "ipcsetup.h"
//#include "IPCserver.h"
#include "IPCclient.h"
#include "Settings.h"

// ----------------------------------------------------------------------------
// Icona TaskBar
// ----------------------------------------------------------------------------

class TaskBarIcon : public wxTaskBarIcon
{
public:
	TaskBarIcon() {}

	void OnLeftButtonDClick(wxTaskBarIconEvent&);
	void OnMenuRestore(wxCommandEvent&);
	void OnMenuExit(wxCommandEvent&);
	void OnMenuCheckmark(wxCommandEvent&);
	void OnMenuUICheckmark(wxUpdateUIEvent&);
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

public:
	MainFrame(const wxString& title, class Settings* settings);
	virtual ~MainFrame();
	
	bool StartServer();
	bool StartClient();
	MyServer *GetServer() { return m_server; }
	MyClient *GetClient() { return m_client; }
	
	void SendFile(std::string path);
	void ReciveFile();
protected:
	void OnAbout(wxCommandEvent& event);
	void OnOK(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnCloseWindow(wxCloseEvent& event);
	//void OnServerEvent(wxThreadEvent& event);
	TaskBarIcon *m_taskBarIcon;

	wxDECLARE_EVENT_TABLE();
};

