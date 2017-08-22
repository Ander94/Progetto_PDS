#include "reciver.h"
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost\filesystem.hpp>
#include <iterator>
#include <fstream>

#define BUFLEN 65536

using boost::asio::ip::udp;
using boost::asio::ip::tcp;

void iscriviUtente(std::string username, std::string ipAddr,utente& utenteProprietario, std::string generalPath);

void reciveUDPMessage(utente& utenteProprietario, std::string generalPath) {
	

	boost::asio::io_service io_service;
	udp::socket s(io_service);
	
	boost::asio::ip::udp::endpoint local_endpoint;
	boost::asio::ip::udp::endpoint reciver_endpoint;

	s.open(boost::asio::ip::udp::v4());
	s.set_option(boost::asio::ip::udp::socket::reuse_address(true));
	s.set_option(boost::asio::socket_base::broadcast(true));
	local_endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::any(), 1500);
	s.bind(local_endpoint);

	//Lancio il thread che fa il check sul vettore
	boost::thread check(utente::checkTime, boost::ref(utenteProprietario));

	while(1){
		
		char buf[1024];
		size_t length = s.receive_from( boost::asio::buffer(buf, 1024), reciver_endpoint);
		std::string ipAddr;
		std::string reciveMessage;
		size_t found;

		ipAddr = reciver_endpoint.address().to_string();
		buf[length] = '\0';
		reciveMessage = buf;

		//Mi accerto che la richesta che ho ricevuto non sia utile a determinare il proprio IP
		found = reciveMessage.find("+GETADDR");
		if (found == std::string::npos)
			iscriviUtente(reciveMessage, ipAddr, utenteProprietario, generalPath);
	}
	
	s.close();
	return;
}

void iscriviUtente(std::string username, std::string ipAddr, utente& utenteProprietario, std::string generalPath) {

	//se attivato a true, evita di ricevere i pacchetti per l'iscrizione in loopback
	if (false) {
		if (utenteProprietario.getIpAddr() == ipAddr) {
			return;
		}
	}

	boost::posix_time::ptime currentTime = boost::posix_time::second_clock::local_time();

	//Controllo se l'utente è già iscritto
	if (utenteProprietario.contieneUtente(username) == true) {
		//Se l'utente è già iscritto, setto un nuovo tempo (cioè vuol dire che l'utente è ancora online)
		utenteProprietario.getUtente(username).setCurrentTime(currentTime);
		return;
	}

	//Invio la mia immagine di profilo al nuovo utente iscritto
	//Il protocollo che utilizzerò sarà
	//Invia +IM
	//Attendi +OK
	//Invia dim del file
	//Attendi +OK
	//Invia file
	//Chiudi tutto

	//Qua dovrà andare il path della mia immagine di default
	std::string filePath(generalPath + "profilo.png");
	
	if (boost::filesystem::is_regular_file(filePath)!=true) {
		filePath = generalPath + "user_default.png";
		if (boost::filesystem::is_regular_file(filePath) != true) {
			wxMessageBox("Immagne per profilo non trovata.", wxT("Errore"), wxOK | wxICON_ERROR);
			//USCIRE IN MODO CORRETTO!
			exit(-1);
		}
	}

	sendImage(filePath, ipAddr);

	//IMPORTANTE CHE QUESTO STIA DOPO!
	//Aggiungo l'utente
	utenteProprietario.addUtente(username, ipAddr, currentTime);
}

