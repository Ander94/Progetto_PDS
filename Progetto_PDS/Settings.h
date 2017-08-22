#pragma once
#include <iostream>
#include <cstdlib>
#include <memory>
#include <ctime>

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>

#include "utente.h"
#include "sender.h"

#include "MainApp.h"

using boost::asio::ip::udp;
using boost::asio::ip::tcp;

class Settings
{
private:
	utente* m_utenteProprietario;
	std::string m_GeneralPath; //AGGIUNTA DA SERGIO PER RENDERE GENERALE IL PATH
	std::string m_ImagePath;
	std::string m_DefaultImagePath;
	std::string m_SavePath; 
	std::string m_SendPath;
	bool m_isDir; //si sta inviando cartella o file
	status m_stato; //on-line(0) o off-line(1)
	
public:
	Settings() {}
	//Settings(std::string nomeUtente) { m_utenteProprietario = new utente(nomeUtente); }
	~Settings() { delete(m_utenteProprietario); }

	/*
	Da chiamare subito dopo la creazione di una nuovo oggetto settings
	*/
	void Init(std::string path, std::string nomeUtente)
	{
		m_stato = status::STAT_ONLINE;
		NewUtenteProprietario(nomeUtente, getOwnIP());
		m_SavePath = "C:\\Users\\" + nomeUtente + "\\Download\\";
		m_DefaultImagePath = path + "user_default.png";
		m_ImagePath = path + "profilo.png";
		if (!boost::filesystem::is_regular_file(m_ImagePath))
			boost::filesystem::copy_file(m_DefaultImagePath, m_ImagePath);
	}

	void NewUtenteProprietario(std::string nomeUtente, std::string ip) { m_utenteProprietario = new utente(nomeUtente, ip); }
	//void NewUtenteProprietario() { m_utenteProprietario = new utente(); }
	utente& getUtenteProprietario() { return *m_utenteProprietario; }

	void setImagePath(std::string imagePath) { m_ImagePath = imagePath; }
	std::string getImagePath() { return m_ImagePath; }
	void setSavePath(std::string savePath) { m_SavePath = savePath; }
	std::string getSavePath() { return  m_SavePath; };
	//AGGIUNTA DA SERGIO
	void setGeneralPath(std::string generalPath) { m_GeneralPath = generalPath; }
	std::string getGeneralPath() {return  m_GeneralPath; };

	std::string getUserName() { return m_utenteProprietario->getUsername(); }

	std::vector<utente>& getUtentiConnessi() { return m_utenteProprietario->getUtentiConnessi(); }

	void setSendPath(std::string sendPath) { m_SendPath = sendPath; }
	std::string getSendPath() { return m_SendPath; }
	
	void setIsDir(bool isDir) { m_isDir = isDir; }
	bool getIsDir() { return m_isDir; }

	void setStatoOn() {
		//wxMessageBox("Setto Online!", wxT("INFO"), wxOK | wxICON_INFORMATION);
		m_stato = status::STAT_ONLINE; }
	void setStatoOff() {
		//wxMessageBox("Setto Offline!", wxT("INFO"), wxOK | wxICON_INFORMATION); 
		m_stato = status::STAT_OFFLINE; }
	status& getStato() { 
		
		return m_stato; }

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
		local_endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::any(), 1500);
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
		sender_endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::broadcast(), 1500);

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

};