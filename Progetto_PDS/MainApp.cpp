// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "share_icon.xpm"

#include "MainApp.h"

#include "reciver.h"
#include "server.h"

#include <wx/cmdline.h>
#include <wx/taskbar.h>

#include "boost\asio.hpp"


IMPLEMENT_APP(MainApp)

bool MainApp::OnInit() 
{
	//wxMessageBox(cwd, wxT("INFO"), wxOK | wxICON_INFORMATION);

	//-------------------------------------------------------------------------------------------------
	//           MAIN
	//-------------------------------------------------------------------------------------------------

	try
	{
		Settings* m_settings = new Settings();
		std::string path = argv[0];
		std::string str = "Progetto_PDS.exe";
		path.replace(path.end() - str.length(), path.end(), "");
		
		//Concateno gli argomenti in una sola stringa
		//Necessario per gestire eventuali spazi nei nomi di file o cartelle
		std::string argument;
		if (argc > 1)
			for (int i = 1; i < argc; i++) {
				if (i==1) {
					argument.append(argv[i]);
				}
				else {
					argument.append(" " + argv[i]);
				}
				
			}

		if (m_settings->StartClient()) {
			//Se entro qua, significa che c'è già un processo aperto. 
			//Se ho ricevuto un path, lo invio al processo in esecuzione e termino senza fare altro.

			//creo una finestra principale vuota, così può essere distrutta per terminare l'applicazione correttamente
			MainFrame* frame = new MainFrame();	
			SetTopWindow(frame);
			
			if (argc == 1) {
				m_settings->GetClient()->GetConnection()->Execute("");
			}
			else {
				m_settings->GetClient()->GetConnection()->Poke(argument, "path", 5);
			}
			m_settings->GetClient()->Disconnect();
			m_settings->DeleteClient();
			frame->Destroy();
		}
		else {
			//se entro qua, significa che non ci sono altri processi aperti
			m_settings->Init(path, wxGetUserName().ToStdString());

			//i privilegi di amministratore servono per modificare le chiavi di registro
			if (argument == "_ADMIN_PRIVILEGES") {
				m_settings->setAdmin();
				if (m_settings->getScorciatoia() == scorciatoia::SCORCIATOIA_ASSENTE)
					m_settings->AddRegKey();
				else
					m_settings->RemRegKey();
				argument = "_NO_ARGUMENT";
			}
				

			MainFrame* frame = new MainFrame("LAN Sharing Service", m_settings);
			frame->SetIcon(wxIcon(share_icon));
			SetTopWindow(frame);

			frame->Show();

			//Inizializzazione dei vari threads
			m_settings->setExitRecive(false);
			m_settings->setExitSend(false);
			m_settings->reciveTCPfileThread = boost::thread(reciveTCPfile, boost::ref(m_settings->getUtenteProprietario()), m_settings->getGeneralPath(), m_settings, boost::ref(m_settings->getIoService()));
			m_settings->reciveUdpMessageThread = boost::thread(reciveUDPMessage, boost::ref(m_settings->getUtenteProprietario()), m_settings->getGeneralPath(), boost::ref(m_settings->getExitRecive()));
			m_settings->sendUdpMessageThread = boost::thread(sendUDPMessage, boost::ref(m_settings->getUserName()), boost::ref(m_settings->getStato()), boost::ref(m_settings->getExitSend()));
			m_settings->sendAliveThread = boost::thread(m_settings->SendAlive, boost::ref(m_settings->getUtenteProprietario()), boost::ref(m_settings->getExitSend()));
			m_settings->reciveAliveThread = boost::thread(m_settings->ReciveAlive, boost::ref(m_settings->getUtenteProprietario()), boost::ref(m_settings->getExitRecive()));
			frame->StartServer();
			
			if (argc > 1 && argument != "_NO_ARGUMENT") {
				frame->SendFile(argument);
			}
		}
	}
	catch (const std::exception& e)
	{
		wxMessageBox(e.what(), wxT("Errore"), wxOK | wxICON_ERROR);	//TODO gestione errori
	}
	
	return true;
}

void MainApp::OnInitCmdLine(wxCmdLineParser& parser)
{
	static const wxCmdLineEntryDesc cmdLineDesc[] =
	{
		{ wxCMD_LINE_PARAM, NULL, NULL, "", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_MULTIPLE | wxCMD_LINE_PARAM_OPTIONAL},
		{ wxCMD_LINE_NONE }
	};
	parser.SetDesc(cmdLineDesc);
}

int MainApp::OnExit()
{
	//la chiusura dei thread si può inserire qua
	//distruggere settings

	return 0;
}
