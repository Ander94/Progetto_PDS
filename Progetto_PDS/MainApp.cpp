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
			//Se entro qua, significa che c'� gi� un processo aperto. 
			//Se ho ricevuto un path, lo invio al processo in esecuzione, altrimenti segnalo di aprire la finestra principale.
			//Poi termino questo processo.

			//Creo una finestra principale vuota, cos� pu� essere distrutta per terminare l'applicazione correttamente
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

			//se l'argomento � admin_privileges, significa che un processo ha ricevuto la richiesta
			//di modificare le chiavi di registro e questo processo possiede gi� i diritti per farlo
			if (argument == "_ADMIN_PRIVILEGES") {
				m_settings->setAdmin();
				if (m_settings->getScorciatoia() == scorciatoia::SCORCIATOIA_ASSENTE) {
					if (m_settings->AddRegKey() == -1)
						wxLogError(wxT("Non � stato possibile creare la chiave di registro."));
				}
				else {
					if (m_settings->RemRegKey() == -1)
						wxLogError(wxT("Non � stato possibile rimuovere la chiave di registro."));
				}
				argument = "_NO_ARGUMENT";
			}
				

			MainFrame* frame = new MainFrame("LAN Sharing Service", m_settings);
			frame->SetIcon(wxIcon(share_icon));
			SetTopWindow(frame);

			frame->Show();

			//Inizializzazione dei vari threads
			m_settings->setExitRecive(false);
			m_settings->setExitSend(false);
			//Thread che mi consente di ricevere file.
			//Esso si pone in attesa di una nuova richiesta.
			m_settings->reciveTCPfileThread = std::thread(reciveTCPfile, std::ref(m_settings->getUtenteProprietario()), m_settings->getGeneralPath(), m_settings, std::ref(m_settings->getIoService()));
			//thread che riceve messaggi UDP da tutta la LAN, ed eventualmente registra un nuovo utente.
			m_settings->reciveUdpMessageThread = std::thread(reciveUDPMessage, std::ref(m_settings->getUtenteProprietario()), m_settings->getGeneralPath(), std::ref(m_settings->getExitRecive()), m_settings);
			//thrad che invia messaggi UDP segnalando il proprio stato e la propria prensenza in tutta la rete LAN.
			m_settings->sendUdpMessageThread = std::thread(sendUDPMessage, std::ref(m_settings->getUserName()), std::ref(m_settings->getStato()), std::ref(m_settings->getExitSend()));
			frame->StartServer();
			
			//il processo � stato avviato con degli argomenti, quindi procedo immediatamente all'invio del file/cartella
			if (argc > 1 && argument != "_NO_ARGUMENT") {
				frame->SendFile(argument);
			}
		}
	}
	catch (...)
	{
		wxLogError(wxT("Errore nell'avvio dell'applicazione.\nSi prega di riavviare."));
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
	return 0;
}
