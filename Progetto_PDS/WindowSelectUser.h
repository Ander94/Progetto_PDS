#pragma once


#include "Settings.h"
#include "UserSizer.h"
#include "TaskBarIcon.h"

#include <wx/wx.h>
#include <vector>
#include <map>
#include <memory>

class WindowSelectUser : public wxFrame 
{
private:
	Settings* m_settings;
	class MainFrame* m_frame;
	std::map <std::string, utente> m_MappaInvio;
	std::list <class UserSizer *> m_ListaUtenti;
	wxGridSizer* m_topSizer;
	wxFlexGridSizer* m_sizer;
	wxBoxSizer* m_botSizer;
	wxBoxSizer* m_botSizerH;
	wxButton* m_ok;
	wxButton* m_cancel;	
	wxTimer* m_timer;

	void OnOk(wxCommandEvent& event);
	void OnCancel(wxCommandEvent& event);
	void OnCloseWindow(wxCloseEvent& event);
	void OnTimer(wxTimerEvent& event);
	wxDECLARE_EVENT_TABLE();

public:
	WindowSelectUser(wxWindow* parent, Settings* settings);
	//~WindowSelectUser();

	//da usare per aggiornare la finestra (inserire o togliere utenti)
	void UpdateUI();
	void addUtente(utente user);
	void removeUtente(utente user);
	
	std::unique_ptr<std::vector<utente>> getListaInvio();

	//per m_MappaInvio, da usare in UserSizer
	void insertUtenteLista(utente user);
	void deleteUtenteLista(utente user);
};

