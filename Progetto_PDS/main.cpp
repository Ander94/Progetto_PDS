#include <iostream>
#include <cstdlib>
#include <memory>
#include <ctime>
#include "utente.h"
#include "sender.h"
#include "reciver.h"
#include "server.h"
#include "client.h"
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#define BUFLEN 256

using boost::asio::ip::udp;
using boost::asio::ip::tcp;
void srand(std::string& unique_str);

enum menu_command {
	INVIA_FILE = 1, STAMPA = 2, ESCI = 3, NON_VALIDO = 4, ONLINE = 5, OFFLINE = 6
};

void menu();
menu_command insert_command();
void getOwnIP(std::string& ipAddr);
void getOwnUsername(std::string&username);
void reciveUDPMessageGETIP(std::string& ownIpAddr, std::string& unique_str);
void sendUDPMessageGETIP(std::string& unique_str);




int main(int argc, char **argv) {
	
	std::string usernameToSend;
	std::string initialAbsolutePath;
	std::string username;
	std::string ipAddr;
	menu_command command;
	status current_status;
	//Questa funzione prende il nome utente da sola, se non la si vuole usare utilizzare la std::cin commentata
	getOwnUsername(username);
	//std::cout << "Inserisci il nome dell'utente " << std::endl;
	//std::cin >> username;

	//Acquisisco il mio ip
	getOwnIP(ipAddr);

	//Creo un ogetto utente, che incapsula lo username e l'ip dell'utente, e tutti gli utenti a lui connessi
	utente utenteProprietario(username, ipAddr);
	
	//Lancio i thread che girano in background
	//Dico che lo stato iniziale è online, ma questo potrà essere poi cambiato
	current_status = STAT_ONLINE;
	boost::thread sendUdpMessageThread(sendUDPMessage, boost::ref(username), boost::ref(current_status));
	boost::thread reciveUdpMessageThread(reciveUDPMessage, boost::ref(utenteProprietario));
	boost::thread reciveTCPfileThread(reciveTCPfile, boost::ref(utenteProprietario));
	
	//Stamp il menu
	menu();

	//Gestisco l'inserimento dei comandi da parte dell'utente
	while (1) {
		while ((command = insert_command()) == NON_VALIDO) {
			std::cout << "Comando non valido. Re-";
		}
		if (command == INVIA_FILE) {
			std::cout << "Inserire il nome dell'utente a cui inviare il file" << std::endl;
			std::cin >> usernameToSend;
			std::cout << "Inserire il nome del file/directory da inviare" << std::endl;
			std::cin >> initialAbsolutePath;
			sendTCPfile(utenteProprietario, usernameToSend, initialAbsolutePath);
			//Questa sleep serve per non stampare subito nel prompt e far casino, dovrà essere eliminata
			Sleep(1000); //**ELIMINARE**//
		}
		if (command == STAMPA) {
			if (utenteProprietario.getUtentiConnessi().empty()) {
				std::cout << "Nessun utente connesso!" << std::endl;
			}
			else {
				std::cout << "Utenti connessi:" << std::endl;
				for (auto it : utenteProprietario.getUtentiConnessi()) {
					std::cout << it.getUsername() << std::endl;
				}
			}
		}

		if (command == ESCI) {
			//ricordare di forzare la chiusuradei thread
			sendUdpMessageThread.detach();
			reciveUdpMessageThread.detach();
			reciveTCPfileThread.detach();
			return 0;
		}

		if (command == ONLINE) {
			if (current_status != STAT_ONLINE) {
				current_status = STAT_ONLINE;
			//	std::cout << "Lo stato corrente e' cambiato a " << current_status << std::endl;
			}
			else {
				std::cout << "Lo stato corrente e' gia' Online." << std::endl;
			}
		}
		if (command == OFFLINE) {
			if (current_status != STAT_OFFLINE) {
				current_status = STAT_OFFLINE;
				//std::cout << "Lo stato corrente e' cambiato a " << current_status << std::endl;
			}
			else {
				std::cout << "Lo stato corrente e' gia' Offnline." << std::endl;
			}
		}
	}
}

//Questa funzione stampa il menu iniziale
void menu() {
	std::cout << "***MENU***" << std::endl;
	std::cout << "Inserisci:" << std::endl;
	std::cout << "1) invia: per inviare un file" << std::endl;
	std::cout << "2) stampa: per stampare gli utenti connessi" << std::endl;
	std::cout << "3) online/offline: per cambiare lo stato" << std::endl;
	std::cout << "4) esci: per uscire" << std::endl;
}

//Prende un comando e lo trasmette al main
menu_command insert_command() {
	std::string command;
	std::cout << "Inserisci un comando..." << std::endl;
	std::cin >> command;
	if (command == "esci") {
		return ESCI;
	}
	if (command == "stampa") {
		return STAMPA;
	}

	if (command == "invia") {
		return INVIA_FILE;
	}
	if (command == "online")
	{
		return ONLINE;
	}
	if (command == "offline") {
		return OFFLINE;
	}
	return NON_VALIDO;
}

//Prende il proprio ip e lo inserisce in ipAddr
void getOwnIP(std::string& ipAddr) {
	ipAddr = "NULL";
	//Finche non acquisisco l'ip (serve ad essere sicuri che t2 non invi il messaggio prima che esso sia ricevuto da t1)
	std::string unique_str("FFFFFFFFFF");
	srand(unique_str);
	while (ipAddr == "NULL") {
		boost::thread t1(reciveUDPMessageGETIP, boost::ref(ipAddr), unique_str);
		boost::thread t2(sendUDPMessageGETIP, unique_str);
		t1.join();
		t2.join();
	}	
}

//Riceve un messaggio di tipo "+GETIPADDRESS" al fine di capire il proprio ip
void reciveUDPMessageGETIP(std::string& ownIpAddr, std::string& unique_str) {

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
	if (response == unique_str) {
		ownIpAddr = reciver_endpoint.address().to_string();	
	}
	s.close();
	return;
}

void sendUDPMessageGETIP(std::string& unique_str) {

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
		socket.send_to(boost::asio::buffer(unique_str), sender_endpoint);
	}
	catch (boost::system::system_error e) {
			std::cout << e.what() << std::endl;
	}
	
	socket.close();
	return;
}

void getOwnUsername(std::string& username) {
	char usernameBuf[BUFLEN + 1];
	DWORD username_len = BUFLEN + 1;
	GetUserName(usernameBuf, &username_len);
	username = usernameBuf;
}

void srand(std::string& unique_str) {
	srand(time(NULL));
	for (int i = 0; i < 10; ++i) {
		unique_str[i] = (char)(( rand()  % 26) + 65);
	}
	for (int i = 0; i < 10; ++i) {
		unique_str[i] = (char)((rand() % 26) + 65);
	}
}