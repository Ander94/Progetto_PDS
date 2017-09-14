#pragma once

#include "wx/wxprec.h"
#include "wx/wx.h"
#include <wx/cmdline.h>

#include "settings.h"

class MainApp : public wxApp
{
private:
	std::recursive_mutex rm_settings;//1

	class MainFrame* m_frame;
	class Settings* m_settings;

public:

	/********************************************************************************
	 Un'applicazione wxWidgets non ha un metodo "main". L'equivalente è il metodo wxApp::OnInit
	 definito in una classe derivata da wxApp.
	**********************************************************************************/
	bool OnInit();
	
	/********************************************************************************
	Chiamato da OnInit() e può essere utilizzato per inizializzare il parser con le opzioni
	della riga di comando per questa applicazione.
	**********************************************************************************/
	void OnInitCmdLine(wxCmdLineParser& parser);
	
	/********************************************************************************
	Viene chiamato quando l'applicazione termina, ma prima che le sue strutture siano distrutte.
	**********************************************************************************/
	int OnExit();

	void setSettings(class Settings* settings) {
		std::lock_guard<std::recursive_mutex> lk_settings(rm_settings);
		this->m_settings = settings;
	}

	class Settings* GetSettings() {
		std::lock_guard<std::recursive_mutex> lk_settings(rm_settings);
		return m_settings;
	}
};


DECLARE_APP(MainApp)
