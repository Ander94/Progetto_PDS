//COMMENTATO TUTTO

#include "sender.h"

void sendUDPMessage(std::string& username, status& current_status, std::atomic<bool>& exit_app) {

	std::string stato; //Stato dell'user sotto forma di stringa
	boost::asio::io_service io_service;   //Procedura di servizio boost
	boost::asio::ip::udp::socket socket(io_service);  //Socket su cui verranno inviati i pacchetti UDP
	boost::asio::ip::udp::endpoint sender_endpoint;   //Destinazione dei pacchetti UDP

	socket.open(boost::asio::ip::udp::v4()); //Inizializzo il socket per l'utilizzo di pacchetti IPv4
	socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
	socket.set_option(boost::asio::socket_base::broadcast(true));
	//Dico che il "sender_endpoint", ovvero colui che ricever� i pacchetti UDP, � tutta la LAN
	//Inviando cosi i pacchetti IPv4 all'indirizzo 255.255.255.255
	sender_endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::broadcast(), PORT_UDP);

	while (!exit_app.load()) {
		//Salvo lo stato sotto forma di stringa
		if (current_status == STAT_ONLINE) {
			stato = "ONLINE";
		}
		else {
			stato = "OFFLINE";
		}

		//Invio sulla LAN un pacchetto contentente la stringa nella forma username\r\nstato\r\n, cos� che tutti gli altri
		//utenti che utilizzano l'applicazione possano registrare gli altri utenti.
		socket.send_to(boost::asio::buffer(username + "\r\n" + stato + "\r\n"), sender_endpoint);
		
		//I pacchetti vengonoo inviati ongi TIME_SEND_MESSAGE_UDP ms
		Sleep(TIME_SEND_MESSAGE_UDP);

	}
	socket.close();
	io_service.stop();
	return;
}