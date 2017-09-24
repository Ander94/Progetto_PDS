//COMMENTATO TUTTO

#pragma once
#include "reciver.h"
#include "Settings.h"
#include <mutex>
#include <atomic>
using boost::asio::ip::udp;
using boost::asio::ip::tcp;

//Mutex utile per iscrivere un utente alla volta.
//Ciò è necessario per prevenire il caso in cui un utente che sta effettuando l'iscrizione si iscriva nuovamente,
//figurando cosi più volte nella lista degli utenti connessi.
std::mutex iscrizione;

//Booleano utile a mostrare lo stato della propria connessione una volta sola.
std::atomic<bool> first_time;
/********************************************************************************
Iscrive o aggiorna i parametri riguardanti l'utente con username "username", indirizzio ip "ipAddr" e stato status
Riceve come parametri:
-username: l'username da iscrivere
-ipAddr: ip dell'utente da iscrivere
-status: stato online/offline dell'utente da iscrivere
 -utenteProprietario: contiene tutti gli utenti iscritti
-generalPath: path da cui prelevare l'immagine del profilo.
**********************************************************************************/
void iscriviUtente(std::string username, std::string ipAddr, enum status, utente& utenteProprietario, std::string generalPath);

void reciveUDPMessage(utente& utenteProprietario, std::string generalPath, std::atomic<bool>& exit_app) {
    
	char buf[PROTOCOL_PACKET], buf_username[PROTOCOL_PACKET], buf_state[PROTOCOL_PACKET]; //Buffer utili a gestire i pacchetti proveniento dalla rete
	size_t length, found;
	std::string ipAddr, reciveMessage, username, s_state;
	status state;
	int n;
    
	//Inizializzo il socket ad accettare pacchetti su IPv4 in boradcast.
	boost::asio::io_service io_service;
	udp::socket s(io_service);
	//Lancio il thread che controlla elimina gli utenti che non inviano pi˘ pacchetti UDP
	boost::thread check(utente::checkTime, boost::ref(utenteProprietario), generalPath, boost::ref(exit_app));
    
    
    //Per quale motivo ho due cicli?
    //-Il ciclo itnterno serve per ricevere la stringa proveniente dalla LAN
    //-In caso di eccezione, per cui recive_from dovesse fallire, catch chiuderà il socket, che però verrà re-inizializzato grazie all'uso
    //del ciclo esterno.
    //=>Ciò comporta che l'applicazione riceva sempre pacchetti, anche in caso di eccezioni.
	while (!exit_app.load()) {
		boost::asio::ip::udp::endpoint local_endpoint;  //endpoint locale
		boost::asio::ip::udp::endpoint reciver_endpoint; //endpoint di chi invia il pacchetto udp
		s.open(boost::asio::ip::udp::v4());  //Apro il socket
		s.set_option(boost::asio::ip::udp::socket::reuse_address(true));
		s.set_option(boost::asio::socket_base::broadcast(true));  //Inizializzo l'opzione che mi consente l'invio in LAN
		local_endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::any(), PORT_UDP);
		s.bind(local_endpoint);
		bool exit_internal_loop = false;
		first_time.store(true);
        
		while (!exit_app.load() && exit_internal_loop == false) {
			//Ricevo un messaggio
			try {
				length = s.receive_from(boost::asio::buffer(buf, PROTOCOL_PACKET), reciver_endpoint);
				//Estraggo l'ip di chi mi ha inviato il mesasggio
				ipAddr = reciver_endpoint.address().to_string();
				buf[length] = '\0';
				reciveMessage = buf;
				//Mi accerto che la richesta che ho ricevuto non sia utile a determinare il proprio IP
				found = reciveMessage.find("+GETADDR");
				if (found == std::string::npos) {
					//Estraggo username e stato dal messaggio.
					//In particolare leggo la stringa nel formato username\r\nstato\r\n
					//Qui sotto si implementa una read until "\r\n" che legge prima l'username e poi lo stato.
					n = 0;
					do {
						buf_username[n] = buf[n];
						n++;
					} while (buf[n] != '\r' && buf[n + 1] != '\n');
					buf_username[n] = '\0';
					username = buf_username;
					n += 2;
					do {
						buf_state[n - username.length() - 2] = buf[n];
						n++;
					} while (reciveMessage[n] != '\r' && reciveMessage[n + 1] != '\n');
					buf_state[n - username.length() - 2] = '\0';
					s_state = buf_state;
					if (s_state == "ONLINE") {
						state = status::STAT_ONLINE;
					}
					else {
						state = status::STAT_OFFLINE;
					}
					//Iscrivo l'utente.
					boost::thread(iscriviUtente, username, ipAddr, state, boost::ref(utenteProprietario), generalPath).detach();
				}
			}
			catch (...) {
				iscrizione.unlock();
				if (s.is_open()) {
					s.close();
				}
				io_service.stop();
				exit_internal_loop = true;
			}
		}
	}
	//Chiudo il controllo sugli utenti connessi
	check.join();
	if (s.is_open()) {
		s.close();
	}
	io_service.stop();
}

void iscriviUtente(std::string username, std::string ipAddr, enum status state, utente& utenteProprietario, std::string generalPath) {

	std::lock_guard<std::mutex> lg(iscrizione); //Acquisisco il mutex
	int counter = 0;  //Conta il numero di tentativi utili per l'acquisizione dell'immagine.
	//Evita di registrare se stessi.
    //getOwnIP torna l'ip del nostro PC
    std::string myIp(Settings::getOwnIP());
	if (true) {
		if (myIp ==ipAddr || myIp == "127.0.0.1") {
			if (myIp == "127.0.0.1") {
				if (first_time.load()) {
					std::thread([]() {
						Sleep(5000);
						if ( Settings::getOwnIP() == "127.0.0.1")
							wxMessageBox("Attenzione: la connessione internet èË assente.\nControllare lo stato della propria connessione per \nutilizzare correttamente l'applicazione.", wxT("INFO"), wxOK | wxICON_INFORMATION);
					}).detach();
					first_time.store(false);
				}
			}
			else {
				first_time.store(true);
			}
			return;
		}
	}
	boost::posix_time::ptime currentTime = boost::posix_time::second_clock::local_time();
	//Controllo se l'utente Ë gi‡ iscritto
	if (utenteProprietario.contieneUtente(ipAddr) == true) {
		//Se l'utente Ë gi‡ iscritto, setto un nuovo tempo (cioË vuol dire che l'utente Ë ancora online)
		utenteProprietario.getUtente(ipAddr).setCurrentTime(currentTime);
		//il nuovo stato
		utenteProprietario.getUtente(ipAddr).setState(state);
		//e l'username
		utenteProprietario.getUtente(ipAddr).setUsername(username);
		return;
	}

	//Se l'utente non Ë ancora iscritto, invio la mia immagine di profilo al nuovo utente iscritto
	//Se l'utente ha settato gi‡ un immagine, essa viene presa da profilo.png, altrimenti su usa un immagine di default.
	std::string filePath(generalPath + "profilo.png");
	if (boost::filesystem::is_regular_file(filePath) != true) {
		filePath = generalPath + "user_default.png";
		if (boost::filesystem::is_regular_file(filePath) != true) {
			wxMessageBox("Immagne del profilo non trovata", wxT("Errore"), wxOK | wxICON_ERROR);
		}
	}

	//Invio la mia immagine del profilo all'utente che sto registrando.
	sendImage(filePath, ipAddr);
	
	//Controllo che sia stata ricevuta correttamente l'immagine del profilo.
	//Se Ë avvenuto qualche errore in ricezione.
	//Cerco di ricevere l'immagine del profilo per un secondo. Se qualcosa Ë andato storto(ad esempio l'immagine non Ë stata ricevuta),
	//non aggiungo l'utente.
	//L'utente verr‡ aggiungo successivamente con l'arrivo di un nuovo pacchetto UDP
	while (utenteProprietario.immagineRicevuta(ipAddr) == false) {
		Sleep(200);
		if (counter>5) {
			break;
		}
	}
	//Aggiungo il nuovo utente e il suo stato.
	if (counter <= 5) {
		utenteProprietario.addUtente(username, ipAddr, state, currentTime);
	}
}
