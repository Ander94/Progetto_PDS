//COMMENTATA PARTE SERGIO
#pragma once
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <memory>
#include <ctime>
#include <atomic>
#include <Shlwapi.h>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>

#include <wx/log.h>
#include <wx/notifmsg.h>

#include "TaskBarIcon.h"
#include "MainApp.h"
#include "ipcsetup.h"
#include "IPCclient.h"

#include "timeout.h"
#include "utente.h"
#include "sender.h"


//Indica se bisogna richiedere all'utente l'accettazione di un file.
enum save_request {
	SAVE_REQUEST_NO = 0,
	SAVE_REQUEST_YES = 1
};

//Indica se la l'opzione di avvio del programma tramite tasto destro è attivata o meno.
enum scorciatoia {
	SCORCIATOIA_PRESENTE = 0,
	SCORCIATOIA_ASSENTE = 1
};

//Indica la modalità di esecuzione dell'applicazione.
enum modalità {
	MOD_USER = 0,
	MOD_ADMIN = 1
};

class Settings
{
private:
	//Mutex per gestire l'accesso in concorrenza alla classe Settings.
	std::recursive_mutex rm_utenteProprietario;  //1
	std::recursive_mutex rm_GeneralPath;//2
	std::recursive_mutex rm_ImagePath;//3
	std::recursive_mutex rm_DefaultImagePath;//4
	std::recursive_mutex rm_SavePath;//5
	std::recursive_mutex rm_SendPath;//6
	std::recursive_mutex rm_client;//7
	std::recursive_mutex rm_isDir;//8
	std::recursive_mutex rm_stato;//9
	std::recursive_mutex rm_save_request;//10
	std::recursive_mutex rm_exit_send_udp;//11
	std::recursive_mutex rm_exit_recive_udp;//12
	std::recursive_mutex rm_io_service_tcp;//13
	std::recursive_mutex rm_scorciatoia;//14
	std::recursive_mutex rm_taskBarIcon;//14
	std::mutex m_socket;//15

	utente* m_utenteProprietario;   //Riferimento ad utente proprietario.
	wxTaskBarIcon* m_taskBarIcon;
	wxNotificationMessage* m_notification;
	std::string m_GeneralPath; 
	std::string m_ImagePath;
	std::string m_DefaultImagePath;
	std::string m_SavePath;
	std::string m_SendPath;
	MyClient* m_client;
	bool m_isDir; //si sta inviando cartella o file
	status m_stato; //on-line(0) o off-line(1)
	modalità m_mod; //se si hanno i privilegi amministratore o no
	save_request m_save_request;  //Richiesta quando si riceve un file
	scorciatoia m_scorciatoia;  
	std::atomic<bool> exit_send_udp, exit_recive_udp;  //Atomic che indica se i threads di invio/ricezione di pacchetti udp possono essere disattivati
	boost::asio::io_service io_service_tcp;   //io_service che mette in run/stop la procedura di accettazione dei file.
	boost::asio::io_service io_service_udp;   //io_service utile per inizializzare il socket di ricezione pacchetti udp
	boost::asio::ip::udp::socket socket_udp;  //Socket per la ricezione di pacchetti udp
	

public:
	//Tengo traccia di tutti i thread lanciati ed utilizzati dall'applicazione
	boost::thread sendUdpMessageThread, reciveUdpMessageThread, reciveTCPfileThread;
	
	//Costruttore
	Settings() : socket_udp(io_service_udp) {
	}

	//Distruttore
	~Settings() {
		std::lock_guard<std::recursive_mutex> lk_utenteProprietario(rm_utenteProprietario);
		delete(m_utenteProprietario);
		wxDELETE(m_notification);
	}

	//Torna il riferimento al socket
	boost::asio::ip::udp::socket& getSocket() {
		std::lock_guard<std::mutex> lg_socket(m_socket);
		return socket_udp;
	}


	//Se aperto, chiude il socket.
	void closeSocket() {
		std::lock_guard<std::mutex> lg_socket(m_socket);
		if (socket_udp.is_open()) {
			socket_udp.close();
		}
	}

	//Setta la variabile booleana exit_send_udp con valore "value"
	void setExitSend(bool value) {
		std::lock_guard<std::recursive_mutex> lk_exit_send_udp(rm_exit_send_udp);
		this->exit_send_udp.store(value);
	}

	
	//Torna il valore della variabile booleana exit_send_udp 
	std::atomic<bool>& getExitSend() {
		std::lock_guard<std::recursive_mutex> lk_exit_send_udp(rm_exit_send_udp);
		return this->exit_send_udp;
	}

	//Setta la variabile booleana exit_recive_udp con valore "value"
	void setExitRecive(bool value) {
		std::lock_guard<std::recursive_mutex> lk_exit_recive_udp(rm_exit_send_udp);
		this->exit_recive_udp.store(value);
	}

	//Torna il valore della variabile booleana exit_recive_udp
	std::atomic<bool>& getExitRecive() {
		std::lock_guard<std::recursive_mutex> lk_exit_recive_udp(rm_exit_send_udp);
		return this->exit_recive_udp;
	}

	//Ritorna l'io_serivce che fa riferimento all'accettazione di nuovi file da parte di altri utenti.
	boost::asio::io_service& getIoService() {
		std::lock_guard<std::recursive_mutex> lk_io_service(rm_io_service_tcp);
		return this->io_service_tcp;
	}

	//Aggiorna il file stato.txt con le impostazioni correnti dell'applicazione.
	//Nel file stato.txt vengono salvati:
	//-Username
	//-savePath
	//-Stato
	//-Richiesta di salvataggio
	//-Attivazione scorciatoia
	void updateState() {
		std::fstream save_path_file;
		std::lock_guard<std::recursive_mutex> lk_Username(rm_utenteProprietario);
		std::lock_guard<std::recursive_mutex> lk_GeneralPath(rm_GeneralPath);
		std::lock_guard<std::recursive_mutex> lk_SavePath(rm_SavePath);
		std::lock_guard<std::recursive_mutex> lk_stato(rm_stato);
		std::lock_guard<std::recursive_mutex> lk_save_request(rm_save_request);

		save_path_file.open(this->getGeneralPath() + "stato.txt", std::fstream::out);
		save_path_file << this->getUtenteProprietario().getUsername() << std::endl;
		save_path_file << this->getSavePath() << std::endl;
		if (this->getStato() == status::STAT_ONLINE) {
			save_path_file << "online\n";
		}
		else {
			save_path_file << "offline\n";
		}
		if (this->getAutoSaved() == save_request::SAVE_REQUEST_YES) {
			save_path_file << "autoSavedOn\n";
		}
		else {
			save_path_file << "autoSavedOff\n";
		}
		if (this->getScorciatoia() == scorciatoia::SCORCIATOIA_PRESENTE) {
			save_path_file << "scorciatoiaPresente\n";
		}
		else {
			save_path_file << "scorciatoiaAssente\n";
		}
		save_path_file.close();
	}

	//Inizializza la classe Settings in avvio.
	void Init(std::string path, std::string nomeUtente)
	{
		std::lock_guard<std::recursive_mutex> lk_utenteProprietario(rm_utenteProprietario);
		std::lock_guard<std::recursive_mutex> lk_GeneralPath(rm_GeneralPath);
		std::lock_guard<std::recursive_mutex> lk_ImagePath(rm_ImagePath);
		std::lock_guard<std::recursive_mutex> lk_DefaultImagePath(rm_DefaultImagePath);
		std::lock_guard<std::recursive_mutex> lk_SavePath(rm_SavePath);
		std::lock_guard<std::recursive_mutex> lk_SendPath(rm_SendPath);
		std::lock_guard<std::recursive_mutex> lk_client(rm_client);
		std::lock_guard<std::recursive_mutex> lk_isDir(rm_isDir);
		std::lock_guard<std::recursive_mutex> lk_stato(rm_stato);
		std::lock_guard<std::recursive_mutex> lk_save_request(rm_save_request);
		std::lock_guard<std::recursive_mutex> lk_exit_send_udp(rm_exit_send_udp);
		std::lock_guard<std::recursive_mutex> lk_exit_recive_udp(rm_exit_recive_udp);
		std::lock_guard<std::recursive_mutex> lk_io_service_tcp(rm_io_service_tcp);
		std::lock_guard<std::recursive_mutex> lk_scorciatoia(rm_scorciatoia);

		wxInitAllImageHandlers();

		//Construisco l'utente proprietario, ovvero colui che ha avviato l'applicazione.
		NewUtenteProprietario(nomeUtente, getOwnIP());
		m_GeneralPath = path;  //Path d'avvio del programma.
		m_mod = MOD_USER;     //Modalità d'avvio del programma.
		m_utenteProprietario->setUsernamePc(nomeUtente);   //Username del pc

		//Inizializzo le impostazioni con le impostazioni salvate nel file stato.txt.
		//Se il file non è presente, inizializzo il tutto con le impostazioni di default.
		if (!boost::filesystem::is_regular_file(m_GeneralPath + "stato.txt")) {
			m_SavePath = "C:\\Users\\" + nomeUtente + "\\Downloads\\";
			m_stato = status::STAT_ONLINE;
			m_save_request = save_request::SAVE_REQUEST_NO;
			m_scorciatoia = scorciatoia::SCORCIATOIA_ASSENTE;
		}
		else {
			std::fstream save_path_file;
			std::string stato; 
			std::string save;
			std::string scorc;
			std::string username;
			save_path_file.open(m_GeneralPath + "stato.txt", std::fstream::in);
			std::getline(save_path_file, username);
			std::getline(save_path_file, m_SavePath);
			std::getline(save_path_file, stato);
			std::getline(save_path_file, save);
			std::getline(save_path_file, scorc);
			this->getUtenteProprietario().setUsername(username);
			if (stato == "online") {
				m_stato = status::STAT_ONLINE;
			}
			else {
				m_stato = status::STAT_OFFLINE;
			}
			if (save == "autoSavedOn") {
				m_save_request = save_request::SAVE_REQUEST_YES;
			}
			else {
				m_save_request = save_request::SAVE_REQUEST_NO;
			}
			if (scorc == "scorciatoiaPresente") {
				m_scorciatoia = scorciatoia::SCORCIATOIA_PRESENTE;
			}
			else {
				m_scorciatoia = scorciatoia::SCORCIATOIA_ASSENTE;
			}
			save_path_file.close();
		}

		//Setto il path dell'immagine di profilo e dell'immagine di default.
		m_DefaultImagePath = path + "user_default.png";
		m_ImagePath = path + "profilo.png";
		if (!boost::filesystem::is_regular_file(m_ImagePath))
			boost::filesystem::copy_file(m_DefaultImagePath, m_ImagePath);
	}

	void setTaskBarIcon(wxTaskBarIcon* taskBarIcon) {
		std::lock_guard<std::recursive_mutex> lk_taskBarIcon(rm_taskBarIcon);
		m_taskBarIcon = taskBarIcon;
		m_notification = new wxNotificationMessage();
		m_notification->UseTaskBarIcon(taskBarIcon);
	}
	void showBal(std::string title, std::string message) {
		std::lock_guard<std::recursive_mutex> lk_taskBarIcon(rm_taskBarIcon);
		//m_taskBarIcon->ShowBalloon(title, message, 5000, wxICON_INFORMATION);
		m_notification->SetTitle(title);
		m_notification->SetMessage(message);
		m_notification->Show();
	}

	//Costruisce un nuovo utente proprietario con nome utente "nomeUtente" e indirizzio ip "ip"
	//Verrà salvato in m_utenteProprietario.
	void NewUtenteProprietario(std::string nomeUtente, std::string ip) {
		std::lock_guard<std::recursive_mutex> lk_utenteProprietario(rm_utenteProprietario);
		m_utenteProprietario = new utente(nomeUtente, ip);
	}

	//Restitusice il riferimento all'utente proprietario.
	utente& getUtenteProprietario() {
		std::lock_guard<std::recursive_mutex> lk_utenteProprietario(rm_utenteProprietario);
		return *m_utenteProprietario;
	}

	//Setta il path della propria immagine di profilo. Il path viene indicato in "imagePath"
	void setImagePath(std::string imagePath) {
		std::lock_guard<std::recursive_mutex> lk_ImagePath(rm_ImagePath);
		m_ImagePath = imagePath;
	}

	//Restituisce il path della propria immagine di profilo.
	std::string getImagePath() {
		std::lock_guard<std::recursive_mutex> lk_ImagePath(rm_ImagePath);
		return m_ImagePath;
	}

	//Ritaglia l'immagine per renderla quadrata, così da poterla inserire come immagine di profilo
	void resizeImage(std::string path) {
		wxImage *img = new wxImage();

		//Controlla che l'immagine esiste
		if (!boost::filesystem::is_regular_file(path)) {
			boost::filesystem::copy_file(getGeneralPath() + "user_default.png", path, boost::filesystem::copy_option::overwrite_if_exists);
			return;
		}
		

		//Controlla che l'immagine sia stata caricata correttamente.
		if (!img->LoadFile(path, wxBITMAP_TYPE_ANY, -1)) {
			wxMessageBox("Errore caricamento immagine di " + m_utenteProprietario->getUsernameFromIp(boost::filesystem::basename(path) + boost::filesystem::extension(path)), wxT("ERRORE"), wxOK | wxICON_ERROR);
			boost::filesystem::copy_file(getGeneralPath() + "user_default.png", path, boost::filesystem::copy_option::overwrite_if_exists);
			return;
		}

		//Ritaglia l'immagine
		int h = img->GetHeight();
		int w = img->GetWidth();
		int s, ph, pw;
		if (h < w) {
			s = h; ph = 0; pw = (w - h) / 2;
		}
		else {
			s = w; ph = (h - w) / 2; pw = 0;
		}
		wxImage tmp = img->GetSubImage(wxRect(pw, ph, s, s));
		tmp.Rescale(200, 200, wxIMAGE_QUALITY_HIGH);
		tmp.SaveFile("" + path);
	}

	//Setta il path di salvataggio relativo ai file ricevuti dall'applicazione.
	void setSavePath(std::string savePath) {
		std::lock_guard<std::recursive_mutex> lk_SavePath(rm_SavePath);
		m_SavePath = savePath;
		//Chiamo updateState per salvare le modifiche su stato.txt
		this->updateState();
	}

	//Torna il path di salvataggio dei file.
	std::string getSavePath() {
		std::lock_guard<std::recursive_mutex> lk_SavePath(rm_SavePath);
		return  m_SavePath;
	};

	//Setta il path generale, ovvero il path da cui viene lanciata l'applicazione.
	//Il path viene indicato in "GeneralPath"
	void setGeneralPath(std::string generalPath) {
		std::lock_guard<std::recursive_mutex> lk_GeneralPath(rm_GeneralPath);
		m_GeneralPath = generalPath;
	}

	//Tora 
	std::string getGeneralPath() {
		std::lock_guard<std::recursive_mutex> lk_GeneralPath(rm_GeneralPath);
		return  m_GeneralPath;
	};

	//Torna l'username dell'utente proprietario.
	std::string &getUserName() {
		std::lock_guard<std::recursive_mutex> lk_utenteProprietario(rm_utenteProprietario);
		return m_utenteProprietario->getUsername();
	}
	//Torna l'username del pc
	std::string &getUserNamePc() {
		std::lock_guard<std::recursive_mutex> lk_utenteProprietario(rm_utenteProprietario);
		return m_utenteProprietario->getUsernamePc();
	}
	//Torna il riferimento al vettore di tutti gli utenti connessi.
	std::vector<utente>& getUtentiConnessi() {
		std::lock_guard<std::recursive_mutex> lk_utenteProprietario(rm_utenteProprietario);
		return m_utenteProprietario->getUtentiConnessi();
	}

	//Torna un vettore contente tutti gli utenti online nel momento in cui getUtentiOnline viene chiamato.
	std::vector<utente> getUtentiOnline() {
		std::lock_guard<std::recursive_mutex> lk_utenteProprietario(rm_utenteProprietario);
		return m_utenteProprietario->getUtentiOnline();
	}

	//Setta il path relativo al file da inviare, specificato su sendPath.
	void setSendPath(std::string sendPath) {
		std::lock_guard<std::recursive_mutex> lk_SendPath(rm_SendPath);
		m_SendPath = sendPath;
	}

	//Ritorna il path da inviare.
	std::string getSendPath() {
		std::lock_guard<std::recursive_mutex> lk_SendPath(rm_SendPath);
		return m_SendPath;
	}

	void setIsDir(bool isDir) {
		std::lock_guard<std::recursive_mutex> lk_isDir(rm_isDir);
		m_isDir = isDir;
	}
	bool getIsDir() {
		std::lock_guard<std::recursive_mutex> lk_isDir(rm_isDir);
		return m_isDir;
	}

	//Setta lo stato dell'utente proprietario online.
	void setStatoOn() {
		std::lock_guard<std::recursive_mutex> lk_stato(rm_stato);
		m_stato = status::STAT_ONLINE;
		//Chiamo updateState per salvare le modifiche su stato.txt
		this->updateState();
	}
	//Setta lo stato dell'utente proprietario offline.
	void setStatoOff() {
		std::lock_guard<std::recursive_mutex> lk_stato(rm_stato);
		m_stato = status::STAT_OFFLINE;
		//Chiamo updateState per salvare le modifiche su stato.txt
		this->updateState();
	}

	//Torna lo stato(online/offline) dell'utente proprietario.
	status& getStato() {
		std::lock_guard<std::recursive_mutex> lk_stato(rm_stato);
		return m_stato;
	}

	//Setta l'utilizzo della scorciatoia
	void setScorciatoiaPresente() {
		std::lock_guard<std::recursive_mutex> lk_scorciatoia(rm_scorciatoia);
		m_scorciatoia = scorciatoia::SCORCIATOIA_PRESENTE;
		//Chiamo updateState per salvare le modifiche su stato.txt
		this->updateState();
	}

	//Setta il mancato utilizzo della socricatoia
	void setScorciatoiaAssente() {
		std::lock_guard<std::recursive_mutex> lk_scorciatoia(rm_scorciatoia);
		m_scorciatoia = scorciatoia::SCORCIATOIA_ASSENTE;
		//Chiamo updateState per salvare le modifiche su stato.txt
		this->updateState();
	}

	//Indica se la scorciatoia è utilizzata o meno.
	scorciatoia& getScorciatoia() {
		std::lock_guard<std::recursive_mutex> lk_scorciatoia(rm_scorciatoia);
		return m_scorciatoia;
	}
	
	//Setta il salvataggio automatico
	void setAutoSavedOn() {
		std::lock_guard<std::recursive_mutex> lk_save_request(rm_save_request);
		m_save_request = save_request::SAVE_REQUEST_YES;
		//Chiamo updateState per salvare le modifiche su stato.txt
		this->updateState();
	}

	//Setta la richesta di salvataggio
	void setAutoSavedOff() {
		std::lock_guard<std::recursive_mutex> lk_save_request(rm_save_request);
		m_save_request = save_request::SAVE_REQUEST_NO;
		//Chiamo updateState per salvare le modifiche su stato.txt
		this->updateState();
	}

	save_request & getAutoSaved() {
		std::lock_guard<std::recursive_mutex> lk_save_request(rm_save_request);
		return m_save_request;
	}

	//Setta la modalità di utilizzo dell'applicazione a livello amministratore.
	void setAdmin() {
		m_mod = MOD_ADMIN;
	}

	//Torna la modalità utilizzata
	modalità& getMod() {
		return m_mod;
	}

	//Crea una stringa random di 10 caratteri.
	//Questa è utile nella fase di determinazione del proprio IP.
	static void string_rand(std::string& unique_str) {
		srand(time(NULL));
		for (int i = 0; i < 10; ++i) {
			unique_str[i] = (char)((rand() % 26) + 65);
		}
	}

	//Torna il proprio ip utilizzato nella LAN.
	//Funzionamento:
	//-Invia in rete una stringa univoca(creata in modo casuale dall'applicazione stessa.)
	//-Riceve la stringa univoca e, nel momento della ricezione, riesce a deterinare il proprio indirizzo IP.
	//Ciò lo fa sganciando due thead(uno invia e l'altro riceve) i quali sono entrambi in conoscenza di tale stringa.
	static std::string getOwnIP() {
		std::string ipAddr("NULL");
		std::string unique_str("FFFFFFFFFF");
		//Genero una sringa univoca, al fine di essere sicuro che quello che sto ricevendo e' il mio ip
		string_rand(unique_str);
		//Finche non acquisisco l'ip (serve ad essere sicuri che t2 non invi il messaggio prima che esso sia ricevuto da t1)
		while (ipAddr == "NULL") {
			boost::thread t1(reciveUDPMessageGETIP, boost::ref(ipAddr), unique_str);
			boost::thread t2(sendUDPMessageGETIP, unique_str);
			t1.join();
			t2.join();
		}
		return ipAddr;
	}

	//Riceve un messaggio di tipo "+GETADDR_unique_str" al fine di capire il proprio ip
	//Salva in ownIpAddr il proprio ip poichè è passato per riferimento
	static void reciveUDPMessageGETIP(std::string& ownIpAddr, std::string& unique_str) {
		boost::asio::io_service io_service;
		udp::socket s(io_service);

		boost::asio::ip::udp::endpoint local_endpoint;
		boost::asio::ip::udp::endpoint reciver_endpoint;

		s.open(boost::asio::ip::udp::v4());
		s.set_option(boost::asio::ip::udp::socket::reuse_address(true));
		s.set_option(boost::asio::socket_base::broadcast(true));
		local_endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::any(), PORT_UDP_OWNIP);
		s.bind(local_endpoint);

		char buf[1024];
		size_t length = s.receive_from(boost::asio::buffer(buf, 1024), reciver_endpoint);
		buf[length] = '\0';
		std::string response(buf);
		//Se il messaggio ricevuto contiene la propria stringa unicvoca, allora posso settare il mio indirizzo ip.
		if (response == ("+GETADDR" + unique_str)) {
			ownIpAddr = reciver_endpoint.address().to_string();
		}
		if (s.is_open()) {
			s.close();
		}
		return;
	}

	//Invia in broadcast la stringa univoca +GETADDR_unique_str.
	static void sendUDPMessageGETIP(std::string& unique_str) {

		boost::asio::io_service io_service;
		boost::asio::ip::udp::socket socket(io_service);
		boost::asio::ip::udp::endpoint local_endpoint;
		boost::asio::ip::udp::endpoint sender_endpoint;

		socket.open(boost::asio::ip::udp::v4());
		socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
		socket.set_option(boost::asio::socket_base::broadcast(true));
		sender_endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::broadcast(), PORT_UDP_OWNIP);
		//Invio in rete una stringa del tipo +GETADDRunique_str, che mi aiuterà ad identificare il mio IP
		socket.send_to(boost::asio::buffer("+GETADDR" + unique_str), sender_endpoint);
		socket.close();
		return;
	}

	//Inizializza il client ICP
	bool StartClient()
	{
		m_client = new MyClient();
		if (!m_client->Connect(IPC_HOST, IPC_SERVICE, IPC_TOPIC)) {
			wxDELETE(m_client);
			return false;
		}
		return true;
	}
	//Ritorna il client
	MyClient *GetClient() {
		std::lock_guard<std::recursive_mutex> lk_client(rm_client); 
		return m_client;
	}
	//Rimuove il client
	void DeleteClient() {
		std::lock_guard<std::recursive_mutex> lk_client(rm_client);
		wxDELETE(m_client);
	}

	//Aggiunge la scorciatoia nel context-menu, così che il programma possa essere avviato da tasto destro.
	int AddRegKey() {
		std::string str = m_GeneralPath + "icon1.ico";
		std::wstring stemp = std::wstring(str.begin(), str.end());
		LPCTSTR data = stemp.c_str();
		std::string str2 = m_GeneralPath + "Progetto_PDS.exe %1";
		std::wstring stemp2 = std::wstring(str2.begin(), str2.end());
		LPCTSTR data2 = stemp2.c_str();
		HKEY hkey;
		DWORD dwDisposition;

		//PRIMA CHIAVE------------------------------------------------------
		if (RegCreateKeyEx(HKEY_CLASSES_ROOT, TEXT("*\\shell\\Share"),
			0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition) != ERROR_SUCCESS)
			return -1;

		if (RegSetValueEx(hkey, TEXT("Icon"), 0, REG_SZ, (LPBYTE)data, _tcslen(data) * sizeof(TCHAR)) != ERROR_SUCCESS)
			return -1;
		
		RegCloseKey(hkey);
		
		if (RegCreateKeyEx(HKEY_CLASSES_ROOT, TEXT("*\\shell\\Share\\command"),
			0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition) != ERROR_SUCCESS)
			return -1;
		
		if (RegSetValueEx(hkey, NULL, 0, REG_SZ, (LPBYTE)data2, _tcslen(data2) * sizeof(TCHAR)) != ERROR_SUCCESS)
			return -1;

		RegCloseKey(hkey);

		//--------------------------------------------------------------------

		//SECONDA CHIAVE------------------------------------------------------
		if (RegCreateKeyEx(HKEY_CLASSES_ROOT, TEXT("Directory\\shell\\Share"),
			0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition) != ERROR_SUCCESS)
			return -1;
		
		if (RegSetValueEx(hkey, TEXT("Icon"), 0, REG_SZ, (LPBYTE)data, _tcslen(data) * sizeof(TCHAR)) != ERROR_SUCCESS)
			return -1;
		
		RegCloseKey(hkey);
		
		if (RegCreateKeyEx(HKEY_CLASSES_ROOT, TEXT("Directory\\shell\\Share\\command"),
			0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition) != ERROR_SUCCESS)
			return -1;
		
		if (RegSetValueEx(hkey, NULL, 0, REG_SZ, (LPBYTE)data2, _tcslen(data2) * sizeof(TCHAR)) != ERROR_SUCCESS)
			return -1;
		
		RegCloseKey(hkey);
		
		//--------------------------------------------------------------------

		setScorciatoiaPresente();
		return 0;
	}

	//Rimuove la scoricatoia da tasto destro
	int RemRegKey() {

		if (SHDeleteKey(HKEY_CLASSES_ROOT, TEXT("*\\shell\\Share")) != ERROR_SUCCESS)
			return -1;

		if (SHDeleteKey(HKEY_CLASSES_ROOT, TEXT("Directory\\shell\\Share")) != ERROR_SUCCESS)
			return -1;

		setScorciatoiaAssente();
		return 0;
	}

};