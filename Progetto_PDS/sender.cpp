#include "sender.h"

void sendUDPMessage(std::string& username, status& current_status, std::atomic<bool>& exit_app) {

    //Per quale motivo ho due cicli?
    //-Il ciclo itnterno serve per inviare la stringa con username/stato nella LAN
    //-In caso di eccezione, per cui send_to dovesse fallire, catch chiuderà il socket, che però verrà re-inizializzato grazie all'uso
    //del ciclo esterno.
    //=>Ciò comporta che l'applicazione riceva sempre pacchetti, anche in caso di eccezioni.
	while (!exit_app.load()) {
		std::string stato; //Stato dell'user sotto forma di stringa
		boost::asio::io_service io_service;   //Procedura di servizio boost
		boost::asio::ip::udp::socket socket(io_service);  //Socket su cui verranno inviati i pacchetti UDP
		boost::asio::ip::udp::endpoint sender_endpoint;   //Destinazione dei pacchetti UDP

		socket.open(boost::asio::ip::udp::v4()); //Inizializzo il socket per l'utilizzo di pacchetti IPv4
		socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
		socket.set_option(boost::asio::socket_base::broadcast(true));
		//Dico che il "sender_endpoint", ovvero colui che riceverà i pacchetti UDP, è tutta la LAN
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

			//Invio sulla LAN un pacchetto contentente la stringa nella forma username\r\nstato\r\n, così che tutti gli altri
			//utenti che utilizzano l'applicazione possano registrare gli altri utenti.
			try {
				socket.send_to(boost::asio::buffer(username + "\r\n" + stato + "\r\n"), sender_endpoint);
			}
			catch (...) {
                if (socket.is_open()) {
                    socket.close();
                }
				break;
			}

			//I pacchetti vengonono inviati ongi TIME_SEND_MESSAGE_UDP ms
			Sleep(TIME_SEND_MESSAGE_UDP);

		}
		if (socket.is_open()) {
			socket.close();
		}
	}
	
	return;
}
