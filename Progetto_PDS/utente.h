#pragma once

#include "boost\date_time\posix_time\posix_time.hpp"

#include <vector>
#include <string>
#include <mutex>
#include "protoType.h"
#include "wx/wx.h"

/***********************************
La classe utente rappresenta l'utente che sta utilizzando l'applicazione.
Essa contiene:
-L'username 
-L'indirizzo IPv4 
-Lo stato
-Il riferimento a tutti gli utenti connessi 
-L'ulltima volta(in ms) che un l'utente ha inviato un pacchetto UDP per notificare la sua presenza sulla LAN

L'accesso concorrente a tale classe è gestito attraverso l'utilizzo di mutex.
*********************************/

enum status {
	STAT_ONLINE = 0,
	STAT_OFFLINE = 1
};

class utente
{
	std::string username;
	std::string usernamePc;
	std::string ipAddr;
	status state;
	std::vector<utente> utentiConnessi;
	boost::posix_time::ptime currentTime;

	//Mutex per gestire l'accesso concorrente tra i diversi thread che utilizzano utenteProprietario
	std::recursive_mutex m_username; //(1)
	std::recursive_mutex m_ipAddr; //(2)
	std::recursive_mutex m_state; //(3)
	std::recursive_mutex m_currentTime; //(4)
	std::recursive_mutex m_utentiConnessi; //(5)
	std::recursive_mutex m_usernamePc; //(6)
	
	
public:
	utente(); //Costruttore 
	utente(std::string username, std::string ipAddr);  //Costruttore 
	utente(std::string username, std::string ipAddr, status state); //Costruttore
	~utente();   //Distruttore
	std::string& utente::getUsername(); //Torna l'username dell'utente
	std::string& utente::getUsernamePc();
	void utente::setIpAddr(std::string ipAddr); //
	void utente::setUsername(std::string username);
	void utente::setUsernamePc(std::string usernamePc);
	std::vector<utente>& utente::getUtentiConnessi(); //Torna il riferimento a tutti gli utenti in LAN, sia Online che Offline
	std::vector<utente> utente::getUtentiOnline(); //Torna il riferimento ai soli utenti online
	void utente::addUtente(std::string username, std::string ipAddr, status state,boost::posix_time::ptime currentTime);  //Aggiunge un nuovo utente alla lista degli utenti connessi
	bool utente::contieneUtente(std::string ipAddr); //Verifica se un utente invia pacchetti in LAN o meno.
	void utente::setCurrentTime(boost::posix_time::ptime currentTime);   //Aggiorna il tempo dell'ultima ricezione di un pacchetto UDP da parte di un utente
	utente& utente::getUtente(std::string ipAddr); //Torna il riferimento all'utente username, lancia un eccezione se l'utente non è contenuto.
	std::string utente::getIpAddr();   //Torna l'indirizzo ip dell'utente 
	boost::posix_time::ptime utente::getTime(); // Torna il tempo dell'ultima ricezione di un pacchetto IP da parte di un utente
	int utente::removeUtente(std::string username);   //Rimuove un utenta dal vettore degli utenti connessi 
	std::string getUsernameFromIp(std::string ipAddr);   //Torna l'username di un utente a partire da un IP
	static void checkTime(utente& utenteProprietario, std::atomic<bool>& exit_app);   //Tale funzione elimina gli utenti che non sono più attivi sulla LAN dopo un tempo definito in protoType.g
	void utente::setState(status state);   //Aggiorna lo stato di un utente
	status utente::getState();   //Ritorna lo stato di un utente
	utente::utente(const utente& source);  //Costruttore di copia
	utente &utente::operator =(const utente & source);   //Operatore di assegnazione
	
};