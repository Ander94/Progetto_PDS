#pragma once

#include <wx/wx.h>
#include <wx/taskbar.h>

#include "WindowProgressBar.h"
#include "WindowSaveFile.h"
#include "WindowSelectUser.h"
#include "ipcsetup.h"
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
	
	~TaskBarIcon() {
		RemoveIcon();
	}

	//Ripristina finestra con doppio click
	void OnLeftButtonDClick(wxTaskBarIconEvent&);

	//Ripristina finestra
	void OnMenuRestore(wxCommandEvent&);

	//Chiude l'applicazione
	void OnMenuExit(wxCommandEvent&);

	//Cambia stato (online/offline)
	void OnMenuStato(wxCommandEvent&);

	//Aggiorna la grafica nella finestra principale
	void OnMenuUIStato(wxUpdateUIEvent&);

	//Crea menu popup
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
	wxBitmapButton* m_changeImage;
	wxBitmapButton* m_changeSavePath;
	wxBitmapButton* m_contextMenu;
	wxStaticBitmap* m_userImage;
	wxRadioBox* m_status;
	wxRadioBox* m_saved;
	wxStaticText* m_textStato;
	wxStaticText* m_textSavePath;
	wxTextCtrl* m_nome;
	TaskBarIcon *m_taskBarIcon;
	
	//Metodi collegati ai vari eventi possibili
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
	void OnLoseFocus(wxFocusEvent&);
	void UpdateIcon();

	wxDECLARE_EVENT_TABLE();
public:
	MainFrame(const wxString& title, class Settings* settings);
	MainFrame();
	~MainFrame();

	bool StartServer();
	class MyServer *GetServer() { return m_server; }
	Settings *GetSettings() { return m_settings; };

	void SendFile(std::string path);
};

