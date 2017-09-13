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
	//void OnMenuCheckmarkOnline(wxCommandEvent&);
	//void OnMenuUICheckmarkOnline(wxUpdateUIEvent&);
	//void OnMenuCheckmarkOffline(wxCommandEvent&);
	//void OnMenuUICheckmarkOffline(wxUpdateUIEvent&);
	void OnMenuStato(wxCommandEvent&);
	void OnMenuUIStato(wxUpdateUIEvent&);

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
	wxButton* m_contextMenu;
	wxStaticBitmap* m_userImage;
	wxRadioBox* m_status;
	wxRadioBox* m_saved;
	wxStaticText* m_textStato;
	wxStaticText* m_textSavePath;
	TaskBarIcon *m_taskBarIcon;
	wxTextCtrl* m_nome;

	void OnInviaFile(wxCommandEvent&);
	void OnInviaDir(wxCommandEvent&);
	void OnOK(wxCommandEvent&);
	void OnExit(wxCommandEvent&);
	void OnCloseWindow(wxCloseEvent&);
	void OnTimer(wxTimerEvent&);
	void OnImage(wxCommandEvent&);
	void OnChangeSavePath(wxCommandEvent&);
	void OnContextMenu(wxCommandEvent&);
	void OnRadioBoxStato(wxCommandEvent&);
	void OnRadioBoxSalvataggio(wxCommandEvent&);	
	void OnMenuUICheckmark(wxCommandEvent&);
	void OnChangeUsername(wxCommandEvent&);
	void UpdateIcon();

	wxDECLARE_EVENT_TABLE();
public:
	MainFrame(const wxString& title, class Settings* settings);
	MainFrame();
	virtual ~MainFrame();
	bool StartServer();
	//bool StartClient();
	class MyServer *GetServer() { return m_server; }
	//MyClient *GetClient() { return m_client; }
	Settings *GetSettings() { return m_settings; };

	void SendFile(std::string path);
	void MainFrame::showBal(std::string title, std::string message);
};

