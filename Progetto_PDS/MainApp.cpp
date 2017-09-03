// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


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
	//std::string cwd = wxGetCwd().ToStdString();
	//wxMessageBox(cwd, wxT("INFO"), wxOK | wxICON_INFORMATION);
	
	//utente u1("Mario", ""), u2("Francescxo", "");
	//std::vector<utente> lista_utenti;
	//lista_utenti.push_back(u1);
	//lista_utenti.push_back(u2);
	//lista_utenti.push_back(utente("Gianluca", ""));
	//lista_utenti.push_back(utente("Leonardo", ""));

	//std::unique_ptr<std::vector<utente>> lista(new std::vector<utente>());
	//for (auto it : lista_utenti)
	//	lista->push_back(it);

	//WindowProgressBar* progress = new WindowProgressBar(NULL, NULL, std::move(lista));
	//progress->Show();

	/*WindowSelectUser *finestra = new WindowSelectUser(lista_utenti);

	finestra->addUtente(utente("Gianluca", ""));
	finestra->addUtente(utente("Leonardo", ""));
	finestra->addUtente(utente("Sergio", ""));
	finestra->addUtente(utente("Ugo", ""));
	finestra->removeUtente(u1);
	finestra->Show();*/


	/*WindowSaveFile *salvataggio = new WindowSaveFile();
	salvataggio->ShowModal();*/



	//-------------------------------------------------------------------------------------------------
	//           MAIN
	//-------------------------------------------------------------------------------------------------
	
	try
	{
		m_settings = new Settings();
		std::string path = argv[0];
		std::string str = "Progetto_PDS.exe";
		path.replace(path.end() - str.length(), path.end(), "");
		m_settings->Init(path, wxGetUserName().ToStdString());
		//m_settings->setGeneralPath(path);
		//m_settings->NewUtenteProprietario(wxGetUserName().ToStdString(), m_settings->getOwnIP());

		//Capire qui, ora funziona
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
				

		m_frame = new MainFrame("LAN Sharing Service", m_settings);
		SetTopWindow(m_frame);
		m_frame->Show();
		//path.replace(path.end() - str.length(), path.end(), "user_default.png");
		//wxMessageBox(path, wxT("INFO"), wxOK | wxICON_INFORMATION);
		//m_settings->setImagePath(path);
		//wxMessageBox(sendpath, wxT("INFO"), wxOK | wxICON_INFORMATION);


		//wxString filepath = wxT("C:\\Users\\ander\\Desktop\\Progetto_PDS\\user_default.png");
		//wxPNGHandler *handler = new wxPNGHandler();
		////wxJPEGHandler *handler = new wxJPEGHandler();
		//wxImage::AddHandler(handler);
		//wxImage *img = new wxImage();
		//wxImage img2;
		//img->LoadFile(filepath, wxBITMAP_TYPE_PNG, -1);
		//img2 = img->Scale(70, 70, wxIMAGE_QUALITY_HIGH);
		//img2.SaveFile("Prova.png", wxBITMAP_TYPE_PNG);

		//tramite file controllo se c'è già un processo in esecuzione
		/*std::fstream status_file;
		status_file.open("C:\\Users\\ander\\Desktop\\Progetto_PDS\\Progetto_PDS\\stato.txt", std::fstream::in);
		int n;
		status_file >> n;
		status_file.close();*/

		//if (n >= 1) {
			/*status_file.open("C:\\Users\\ander\\Desktop\\Progetto_PDS\\Progetto_PDS\\stato.txt", std::fstream::out);
			status_file << n + 1;
			status_file.close();*/

			//l'applicazione è già in esecuzione in background
			//wxMessageBox("Esiste processo già in esecuzione", wxT("INFO"), wxOK | wxICON_INFORMATION);
		if (m_frame->StartClient()) {
			//m_frame->StartClient();
			if (argc == 1) {
				//è stata aperta una nuova istanza, ma non è stato passato nessun argomento
				//si apre la finestra principare dell'istanza già in esecuzione
				m_frame->GetClient()->GetConnection()->Execute("");
			}
			else {
				//è stato ricevuto un path, si notifica l'istanza in esecuzione
				m_frame->GetClient()->GetConnection()->Poke(sendpath, "path", 5);
			}
			m_frame->GetClient()->Disconnect();
			m_frame->Destroy();
		}
		else {
			/*status_file.open("C:\\Users\\ander\\Desktop\\Progetto_PDS\\Progetto_PDS\\stato.txt", std::fstream::out);
			status_file << n + 1;
			status_file.close();*/
			//wxMessageBox("Stampato running", wxT("INFO"), wxOK | wxICON_INFORMATION);

			//avvio l'applicazione per la prima volta
			m_settings->exit_recive_udp.store(false);
			m_settings->exit_send_udp.store(false);
			m_settings->sendUdpMessageThread = boost::thread(sendUDPMessage, m_settings->getUserName(), boost::ref(m_settings->getStato()), boost::ref(m_settings->exit_send_udp));
			//Qua dovrei passare anche m_settings->getGeneralPath();
			m_settings->reciveUdpMessageThread = boost::thread(reciveUDPMessage, boost::ref(m_settings->getUtenteProprietario()), m_settings->getGeneralPath(), boost::ref(m_settings->exit_recive_udp));
			//Qua dovrei passare anche m_settings
			m_settings->reciveTCPfileThread = boost::thread(reciveTCPfile, boost::ref(m_settings->getUtenteProprietario()), m_settings->getGeneralPath(), m_frame, boost::ref(m_settings->io_service_tcp));
			
			m_frame->StartServer();
			
			if (argc > 1) {
				m_frame->SendFile(sendpath);
			}
		}
	}
	catch (const std::exception& e)
	{
		wxMessageBox(e.what(), wxT("Errore"), wxOK | wxICON_ERROR);
	}
	
	//-------------------------------------------------------------------------------------------------



	//if (!wxTaskBarIcon::IsAvailable())
	//{
	//	wxMessageBox (
	//		"There appears to be no system tray support in your current environment. This sample may not behave as expected.",
	//		"Warning", wxOK | wxICON_EXCLAMATION);
	//}

	//// Create the main window
	//MyDialog *gs_dialog = new MyDialog(wxT("wxTaskBarIcon Test Dialog"));
	//gs_dialog->Show(true);



	
	//wxMessageBox(argv[0], wxT("INFO"), wxOK | wxICON_INFORMATION);
	//if (argc > 1)
	//	wxMessageBox(argv[1], wxT("INFO"), wxOK | wxICON_INFORMATION);

	
	
	
	
	
	/*
	wxFrame *frame = new wxFrame(NULL, -1, wxT("Utenti disponibili"), wxDefaultPosition, wxDefaultSize);
	wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	UserSizer *u = new UserSizer(frame, wxID_ANY);
	sizer->Add(u);
	frame->SetSizerAndFit(sizer);
	frame->Show();
	*/


	//WindowProgressBar *finestra2 = new WindowProgressBar();
	//finestra2->Show();

	//per modifiche di registro
	//system("\"C:\\Users\\ander\\Desktop\\Open with Notepad Hacks\\Add.reg\"");
	//system("\"C:\\Users\\ander\\Desktop\\Open with Notepad Hacks\\Remove.reg\"");

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
	/*std::fstream status_file;
	status_file.open("C:\\Users\\ander\\Desktop\\Progetto_PDS\\Progetto_PDS\\stato.txt", std::fstream::in);
	int n;
	status_file >> n;
	status_file.close();
	status_file.open("C:\\Users\\ander\\Desktop\\Progetto_PDS\\Progetto_PDS\\stato.txt", std::fstream::out);
	status_file << n - 1;
	status_file.close();*/
		//wxMessageBox("Tutto ok!", wxT("INFO"), wxOK | wxICON_INFORMATION);

	return 0;
}
