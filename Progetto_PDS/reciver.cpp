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
		ipAddr = reciver_endpoint.address().to_string();
		buf[length] = '\0';
		std::string reciveMessage(buf);

		//Mi accerto che la richesta che ho ricevuto non sia utile a determinare il proprio IP
		size_t found = reciveMessage.find("+GETADDR");
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

	if (utenteProprietario.contieneUtente(username) == true) {
		utenteProprietario.getUtente(username).setCurrentTime(currentTime);
		return;
	}

	//Se aggiungo l'utente, gli richiedo l'immagine
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
	}

	boost::asio::io_service io_service;
	tcp::resolver resolver(io_service);
	tcp::resolver::query query(tcp::v4(), ipAddr, "1400");
	tcp::resolver::iterator iterator = resolver.resolve(query);
	tcp::socket s(io_service);

	//Mi connetto al server, se qualcosa non va a buon fine, ritorno al main
	try {
		boost::asio::connect(s, iterator);
	}
	catch (std::exception e) {
		std::cerr << "Non sono riuscito a connettermi con l'utente. Controllare che l'utente sia attivo" << std::endl;
		return;
	}


	//DA QUI
	try {
		std::ifstream file_in(filePath, std::ios::in | std::ios::binary);
		std::ifstream file_dim(filePath, std::ios::in | std::ios::binary);
		//std::string buf;
		std::streampos begin, end_pos;
		begin = file_dim.tellg();
		file_dim.seekg(0, std::ios::end);
		end_pos = file_dim.tellg();
		file_dim.close();
		long int size = (long)(end_pos - begin); //dim file

		std::string fileSize;
		std::ostringstream convert;

		convert << size;
		fileSize = convert.str();
		char buf[256];
		std::string buf_send;
		size_t length;
		if (file_in.is_open())
		{
			//Invio +IM per dire che è un file
			buf_send = "+IM\0";
			boost::asio::write(s, boost::asio::buffer(buf_send));
			//Vedo se il server mi ha detto che va ok
			length = s.read_some(boost::asio::buffer(buf, 256));
			buf[length] = '\0';
			std::string response(buf);
			if (response != "+OK") {
				std::cerr << "Il server ha dato risposta negativa per la recezione del file." << std::endl;
				return;
			}

			//Invio qui la dimensione del file
			boost::asio::write(s, boost::asio::buffer(fileSize));

			length = s.read_some(boost::asio::buffer(buf, 256));
			buf[length] = '\0';
			if (response != "+OK") {
				std::cerr << "Il server ha dato risposta negativa per la recezione del file." << std::endl;
				return;
			}

			char c;
			double dim_write;
			double dim_send = 0;
			char buf_to_send[BUFLEN];
			std::cout << "Client: devo scrivere " << size << " byte" << std::endl;
			while (dim_send < size) {
				dim_write = 0;
				while (dim_write < BUFLEN && dim_send < size) {
					file_in.get(c);
					buf_to_send[(int)dim_write] = c;
					dim_write++;
					dim_send++;

				}
				std::cout << "Client: scrivo " << (int)dim_write << " byte" << std::endl;
				boost::asio::write(s, boost::asio::buffer(buf_to_send, (int)dim_write));
			}
			length = s.read_some(boost::asio::buffer(buf, 256));
			buf[length] = '\0';
			if (response != "+OK") {
				std::cerr << "Il server ha dato risposta negativa per la recezione del file." << std::endl;
				return;
			}
			file_in.close();
		}
		else {
			std::cerr << "File inesistente al client." << std::endl;
			return;
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	//A QUI

	s.close();
	io_service.stop();

	//IMPORTANTE CHE QUESTO STIA DOPO!
	utenteProprietario.addUtente(username, ipAddr, currentTime);

}


