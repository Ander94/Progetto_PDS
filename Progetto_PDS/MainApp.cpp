// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "share_icon.xpm"

#include "reciver.h"
#include "server.h"

#include <wx/cmdline.h>
#include <wx/taskbar.h>

#include "MainApp.h"
#include "TaskBarIcon.h"
#include "boost\asio.hpp"


IMPLEMENT_APP(MainApp)

bool MainApp::OnInit() 
{
	//std::wstring str = argv[0];
	//LPCTSTR data = str.c_str();

	//ShellExecute(NULL,
	//	TEXT("runas"),
	//	data,
	//	TEXT("prova"),
	//	NULL,
	//	SW_SHOWNORMAL
	//);
	
	
	//wxMessageBox(cwd, wxT("INFO"), wxOK | wxICON_INFORMATION);

	//-------------------------------------------------------------------------------------------------
	//           MAIN
	//-------------------------------------------------------------------------------------------------

	try
	{
		setSettings(new Settings());
		//std::wstring program = argv[0];
		std::string path = argv[0];
		std::string str = "Progetto_PDS.exe";
		path.replace(path.end() - str.length(), path.end(), "");
		
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
			//creo una finestra principale vuota, così può essere distrutta per ternimare l'applicazione
			MainFrame* frame = new MainFrame();	
			setFrame(frame);
			SetTopWindow(frame);
			
			if (argc == 1) {
				//è stata aperta una nuova istanza, ma non è stato passato nessun argomento
				//si apre la finestra principare dell'istanza già in esecuzione
				m_settings->GetClient()->GetConnection()->Execute("");
			}
			else {
				//è stato ricevuto un path, si notifica l'istanza in esecuzione
				m_settings->GetClient()->GetConnection()->Poke(argument, "path", 5);
			}
			m_settings->GetClient()->Disconnect();
			m_settings->DeleteClient();
			frame->Destroy();  //necessario, altrimenti l'applicazione non termina correttamente
		}
		else {
			//avvio l'applicazione per la prima volta
			m_settings->Init(path, wxGetUserName().ToStdString());
			
			if (argument == "_ADMIN_PRIVILEGES") {
				m_settings->setAdmin();
				if (m_settings->getScorciatoia() == scorciatoia::SCORCIATOIA_ASSENTE)
					m_settings->AddRegKey();
				else
					m_settings->RemRegKey();
				argument = "_NO_ARGUMENT";
			}
				

			MainFrame* frame = new MainFrame("LAN Sharing Service", GetSettings());
			setFrame(frame);
			frame->SetIcon(wxIcon(share_icon));
			SetTopWindow(frame);

			frame->Show();

			m_settings->setExitRecive(false);
			m_settings->setExitSend(false);
			m_settings->reciveTCPfileThread = boost::thread(reciveTCPfile, boost::ref(m_settings->getUtenteProprietario()), m_settings->getGeneralPath(), GetFrame(), boost::ref(m_settings->getIoService()));
			m_settings->reciveUdpMessageThread = boost::thread(reciveUDPMessage, boost::ref(m_settings->getUtenteProprietario()), m_settings->getGeneralPath(), boost::ref(m_settings->getExitRecive()));
			m_settings->sendUdpMessageThread = boost::thread(sendUDPMessage, m_settings->getUserName(), boost::ref(m_settings->getStato()), boost::ref(m_settings->getExitSend()));
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
		wxMessageBox(e.what(), wxT("Errore"), wxOK | wxICON_ERROR);
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

	return 0;
}
