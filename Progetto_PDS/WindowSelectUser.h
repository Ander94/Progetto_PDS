#pragma once


#include "Settings.h"
#include "UserSizer.h"
#include "TaskBarIcon.h"

#include <wx/wx.h>
#include <vector>
#include <map>

class WindowSelectUser : public wxFrame 
{
private:
	Settings* m_settings;
	class MainFrame* m_frame;
	std::map <std::string, utente> m_MappaInvio;	//utenti attualmente selezionati
	std::list <class UserSizer *> m_ListaUtenti;	//utenti attualmente presenti nella finestra
	wxBitmapButton* m_ok;
	wxBitmapButton* m_cancel;
	wxGridSizer* m_sizerUsers;
	wxTimer* m_timer;

	void OnOk(wxCommandEvent& event);	//fa partire l'invio del file
	void OnCancel(wxCommandEvent& event);
	void OnCloseWindow(wxCloseEvent& event);
	void OnTimer(wxTimerEvent& event);
	wxDECLARE_EVENT_TABLE();

public:
	WindowSelectUser(wxWindow* parent, Settings* settings);

	//ogni volta che scatta il timer, controllo lo stato degli utenti online
	void UpdateUI();	
	
	//da usare con il metodo WindowSelectUser::OnOk per passare la lista di utenti selezionati per l'invio
	std::vector<utente> getListaInvio();	

	//aggiunge un utente alla lista di invio, da usare in UserSizer::OnUserClick
	void insertUtenteLista(utente user);
	//rimuove un utente alla lista di invio, da usare in UserSizer::OnUserClick
	void deleteUtenteLista(utente user);
};

