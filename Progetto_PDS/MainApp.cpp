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
	//wxMessageBox(cwd, wxT("INFO"), wxOK | wxICON_INFORMATION);

	//-------------------------------------------------------------------------------------------------
	//           MAIN
	//-------------------------------------------------------------------------------------------------

	try
	{
		setSettings(new Settings());
		std::wstring program = argv[0];
		std::string path = argv[0];
		std::string str = "Progetto_PDS.exe";
		path.replace(path.end() - str.length(), path.end(), "");
	
		std::string sendpath;
		if (argc > 1)
			for (int i = 1; i < argc; i++) {
				if (i==1) {
					sendpath.append(argv[i]);
				}
				else {
					sendpath.append(" " + argv[i]);
				}
				
			}

		if (m_settings->StartClient()) {
			if (argc == 1) {
				//è stata aperta una nuova istanza, ma non è stato passato nessun argomento
				//si apre la finestra principare dell'istanza già in esecuzione
				m_settings->GetClient()->GetConnection()->Execute("");
			}
			else {
				//è stato ricevuto un path, si notifica l'istanza in esecuzione
				m_settings->GetClient()->GetConnection()->Poke(sendpath, "path", 5);
			}
			m_settings->GetClient()->Disconnect();
			m_settings->DeleteClient();
		}
		else {
			//avvio l'applicazione per la prima volta
			m_settings->Init(path, wxGetUserName().ToStdString());
			
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
			
			if (argc > 1) {
				frame->SendFile(sendpath);
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
