#pragma once
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <memory>
#include <ctime>
#include <atomic>

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>

#include "utente.h"
#include "sender.h"
#include "ipcsetup.h"
#include "IPCclient.h"

#include "MainApp.h"

using boost::asio::ip::udp;
using boost::asio::ip::tcp;

enum save_request {
	SAVE_REQUEST_NO = 0,
	SAVE_REQUEST_YES = 1
};

class Settings
{
private:
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



	utente* m_utenteProprietario;
	std::string m_GeneralPath; //AGGIUNTA DA SERGIO PER RENDERE GENERALE IL PATH
	std::string m_ImagePath;
	std::string m_DefaultImagePath;
	std::string m_SavePath;
	std::string m_SendPath;
	MyClient* m_client;
	bool m_isDir; //si sta inviando cartella o file
	status m_stato; //on-line(0) o off-line(1)
	save_request m_save_request;  //Richiesta quando si riceve un file
	std::atomic<bool> exit_send_udp, exit_recive_udp;
	boost::asio::io_service io_service_tcp;
public:
	boost::thread sendUdpMessageThread, reciveUdpMessageThread, reciveTCPfileThread;






	Settings() {}
	//Settings(std::string nomeUtente) { m_utenteProprietario = new utente(nomeUtente); }
	~Settings() {
		std::lock_guard<std::recursive_mutex> lk_utenteProprietario(rm_utenteProprietario);
		delete(m_utenteProprietario);
	}

	void setExitSend(bool value) {
		std::lock_guard<std::recursive_mutex> lk_exit_send_udp(rm_exit_send_udp);
		this->exit_send_udp.store(value);
	}
	std::atomic<bool>& getExitSend() {
		std::lock_guard<std::recursive_mutex> lk_exit_send_udp(rm_exit_send_udp);
		return this->exit_send_udp;
	}
	void setExitRecive(bool value) {
		std::lock_guard<std::recursive_mutex> lk_exit_recive_udp(rm_exit_send_udp);
		this->exit_recive_udp.store(value);
	}
	std::atomic<bool>& getExitRecive() {
		std::lock_guard<std::recursive_mutex> lk_exit_recive_udp(rm_exit_send_udp);
		return this->exit_recive_udp;
	}


	boost::asio::io_service& getIoService() {
		std::lock_guard<std::recursive_mutex> lk_io_service(rm_io_service_tcp);
		return this->io_service_tcp;
	}

	void updateState() {
		std::fstream save_path_file;
		std::lock_guard<std::recursive_mutex> lk_GeneralPath(rm_GeneralPath);
		std::lock_guard<std::recursive_mutex> lk_SavePath(rm_SavePath);
		std::lock_guard<std::recursive_mutex> lk_stato(rm_stato);
		std::lock_guard<std::recursive_mutex> lk_save_request(rm_save_request);

		save_path_file.open(this->getGeneralPath() + "stato.txt", std::fstream::out);
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
		save_path_file.close();
	}

	/*
	Da chiamare subito dopo la creazione di una nuovo oggetto settings
	*/
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

		wxInitAllImageHandlers();

		NewUtenteProprietario(nomeUtente, getOwnIP());
		m_GeneralPath = path;

		if (!boost::filesystem::is_regular_file(m_GeneralPath + "stato.txt")) {
			m_SavePath = "C:\\Users\\" + nomeUtente + "\\Downloads\\";
			m_stato = status::STAT_ONLINE;
			m_save_request = save_request::SAVE_REQUEST_NO;
		}
		else {
			//Leggo il path dal file
			std::fstream save_path_file;
			std::string stato; //online o offline
			std::string save;
			save_path_file.open(m_GeneralPath + "stato.txt", std::fstream::in);
			std::getline(save_path_file, m_SavePath);
			std::getline(save_path_file, stato);
			std::getline(save_path_file, save);
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

			save_path_file.close();
		}
		m_DefaultImagePath = path + "user_default.png";
		m_ImagePath = path + "profilo.png";
		if (!boost::filesystem::is_regular_file(m_ImagePath))
			boost::filesystem::copy_file(m_DefaultImagePath, m_ImagePath);
	}

	void NewUtenteProprietario(std::string nomeUtente, std::string ip) {
		std::lock_guard<std::recursive_mutex> lk_utenteProprietario(rm_utenteProprietario);
		m_utenteProprietario = new utente(nomeUtente, ip);
	}

	utente& getUtenteProprietario() {
		std::lock_guard<std::recursive_mutex> lk_utenteProprietario(rm_utenteProprietario);
		return *m_utenteProprietario;
	}

	void setImagePath(std::string imagePath) {
		std::lock_guard<std::recursive_mutex> lk_ImagePath(rm_ImagePath);
		m_ImagePath = imagePath;
	}

	std::string getImagePath() {
		std::lock_guard<std::recursive_mutex> lk_ImagePath(rm_ImagePath);
		return m_ImagePath;
	}

	void resizeImage(std::string path) {
		//wxPNGHandler *handler = new wxPNGHandler();
		//wxImage::AddHandler(handler);
		wxImage *img = new wxImage();

		if (!boost::filesystem::is_regular_file(path)) {
			boost::filesystem::copy_file(getGeneralPath() + "user_default.png", path, boost::filesystem::copy_option::overwrite_if_exists);
			return;
		}
		

		if (!img->LoadFile(path, wxBITMAP_TYPE_ANY, -1)) {
			wxMessageBox("Errore caricamento immagine", wxT("ERRORE"), wxOK | wxICON_ERROR);
			return;
		}
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

	void setSavePath(std::string savePath) {
		std::lock_guard<std::recursive_mutex> lk_SavePath(rm_SavePath);
		m_SavePath = savePath;
		this->updateState();
	}
	std::string getSavePath() {
		std::lock_guard<std::recursive_mutex> lk_SavePath(rm_SavePath);
		return  m_SavePath;
	};

	//AGGIUNTA DA SERGIO
	void setGeneralPath(std::string generalPath) {
		std::lock_guard<std::recursive_mutex> lk_GeneralPath(rm_GeneralPath);
		m_GeneralPath = generalPath;
	}
	std::string getGeneralPath() {
		std::lock_guard<std::recursive_mutex> lk_GeneralPath(rm_GeneralPath);
		return  m_GeneralPath;
	};

	std::string getUserName() {
		std::lock_guard<std::recursive_mutex> lk_utenteProprietario(rm_utenteProprietario);
		return m_utenteProprietario->getUsername();
	}

	std::vector<utente>& getUtentiConnessi() {
		std::lock_guard<std::recursive_mutex> lk_utenteProprietario(rm_utenteProprietario);
		return m_utenteProprietario->getUtentiConnessi();
	}

	std::vector<utente> getUtentiOnline() {
		std::lock_guard<std::recursive_mutex> lk_utenteProprietario(rm_utenteProprietario);
		return m_utenteProprietario->getUtentiOnline();
	}

	void setSendPath(std::string sendPath) {
		std::lock_guard<std::recursive_mutex> lk_SendPath(rm_SendPath);
		m_SendPath = sendPath;
	}

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

	void setStatoOn() {
		std::lock_guard<std::recursive_mutex> lk_stato(rm_stato);
		m_stato = status::STAT_ONLINE;
		this->updateState();
	}
	void setStatoOff() {
		std::lock_guard<std::recursive_mutex> lk_stato(rm_stato);
		m_stato = status::STAT_OFFLINE;
		this->updateState();
	}
	status& getStato() {
		std::lock_guard<std::recursive_mutex> lk_stato(rm_stato);
		return m_stato;
	}

	void setAutoSavedOn() {
		std::lock_guard<std::recursive_mutex> lk_save_request(rm_save_request);
		m_save_request = save_request::SAVE_REQUEST_YES;
		this->updateState();
	}
	void setAutoSavedOff() {
		std::lock_guard<std::recursive_mutex> lk_save_request(rm_save_request);
		m_save_request = save_request::SAVE_REQUEST_NO;
		this->updateState();
	}
	save_request & getAutoSaved() {
		std::lock_guard<std::recursive_mutex> lk_save_request(rm_save_request);
		return m_save_request;
	}

	static void string_rand(std::string& unique_str) {
		srand(time(NULL));
		for (int i = 0; i < 10; ++i) {
			unique_str[i] = (char)((rand() % 26) + 65);
		}
		for (int i = 0; i < 10; ++i) {
			unique_str[i] = (char)((rand() % 26) + 65);
		}
	}

	//Prende il proprio ip e lo inserisce in ipAddr
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
		local_endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::any(), PORT_UDP);
		s.bind(local_endpoint);

		char buf[1024];


		size_t length = s.receive_from(boost::asio::buffer(buf, 1024), reciver_endpoint);
		buf[length] = '\0';
		std::string response(buf);
		if (response == ("+GETADDR" + unique_str)) {
			ownIpAddr = reciver_endpoint.address().to_string();
		}
		s.close();
		return;
	}

	//Invia in broadcast la stringa univoca +GETADDR_unique_str.
	//Essa potrà essere interpretata
	static void sendUDPMessageGETIP(std::string& unique_str) {

		boost::asio::io_service io_service;
		boost::asio::ip::udp::socket socket(io_service);
		boost::asio::ip::udp::endpoint local_endpoint;
		boost::asio::ip::udp::endpoint sender_endpoint;

		socket.open(boost::asio::ip::udp::v4());
		socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
		socket.set_option(boost::asio::socket_base::broadcast(true));
		sender_endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::broadcast(), PORT_UDP);

		try {
			//std::cout << "entro\n";
			socket.send_to(boost::asio::buffer("+GETADDR" + unique_str), sender_endpoint);
		}
		catch (boost::system::system_error e) {
			std::cout << e.what() << std::endl;
		}

		socket.close();
		return;
	}

	bool StartClient()
	{
		m_client = new MyClient();
		if (!m_client->Connect(IPC_HOST, IPC_SERVICE, IPC_TOPIC)) {
			wxDELETE(m_client);
			return false;
		}
		return true;
	}

	MyClient *GetClient() {
		std::lock_guard<std::recursive_mutex> lk_client(rm_client);
		return m_client;
	}

	void DeleteClient() {
		std::lock_guard<std::recursive_mutex> lk_client(rm_client);
		wxDELETE(m_client);
	}

	void CreateRegFiles() {
		std::string path = m_GeneralPath + "RegKey";
		if (!boost::filesystem::is_directory(path)) {
			if (!boost::filesystem::create_directories(path)) {
				wxMessageBox("Impossibile creare directory: " + path, wxT("INFO"), wxOK | wxICON_INFORMATION);
				return;
			}
		}
		std::fstream add_key, rem_key;
		add_key.open(path + "\\Add.reg", std::fstream::out);
		std::string str1 = "\"Icon\"=\"" + m_GeneralPath + "\icon1.ico\"";
		std::string str2 = "@=" + m_GeneralPath + "\Progetto_PDS.exe %1\"";
		boost::replace_all(str1, "\\", "\\\\");
		boost::replace_all(str2, "\\", "\\\\");
		add_key << "[HKEY_CLASSES_ROOT\\*\\shell\\Share]\n";
		add_key << str1;
		add_key << "\n\n";
		add_key << "[HKEY_CLASSES_ROOT\\*\\shell\\Share\\command]\n";
		add_key << str2;
		add_key.close();

		rem_key.open(path + "\\Rem.reg", std::fstream::out);
		add_key << "[-HKEY_CLASSES_ROOT\\*\\shell\\Share]";
		add_key << "\n\n";
		add_key << "[-HKEY_CLASSES_ROOT\\Directory\\shell\\Share]";
		rem_key.close();

	}
};