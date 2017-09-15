#pragma once

#include "utente.h"

#include <wx/wx.h>
#include <memory>

#include "Settings.h"
#include "UserProgressBar.h"


class WindowProgressBar : public wxFrame
{
private:
	class MainFrame* m_frame;
	class Settings* m_settings;
	std::vector <class UserProgressBar *> m_ListaUtenti;	//lista delle barre di progresso create
	int m_CountUtenti;	//numero di utenti selezionati per l'invio
	bool m_isSending;	//indica se si sta mandando o ricevendo il file

	void OnCloseWindow(wxCloseEvent& event);

	wxDECLARE_EVENT_TABLE();
public:
	WindowProgressBar(wxWindow* parent, Settings* settings, std::vector<utente> listaUtenti, bool isSending);
	void StartSending();		//da usare dopo la creazione della finestra per chiamare i thread di invio
	void decreseCountUtenti();	//da usare quando termina (o viene abortito) un trasferimento
};