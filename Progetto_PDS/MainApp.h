#pragma once

#include "wx/wxprec.h"
#include "wx/wx.h"
#include <wx/cmdline.h>

#include "TaskBarIcon.h"
#include "settings.h"

class MainApp : public wxApp
{
private:
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
};


DECLARE_APP(MainApp)
