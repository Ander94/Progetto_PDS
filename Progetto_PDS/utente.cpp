//COMMENTATO TUTTO

#include <boost/asio.hpp>
#include "utente.h"
#include <iterator>
#include <cstdlib>
#include <iostream>
#include <future>
#include <memory>
#include <algorithm>
#include "Settings.h"

//Sono presenti diversi tipi di costruttori.(versioni overload)
//Costruttre 
utente::utente()
{
}

//Costruttore 
utente::utente(std::string username, std::string ipAddr, status state)
{
	std::lock_guard<std::mutex> lk_username(m_username);
	std::lock_guard<std::mutex> lk_ipAddr(m_ipAddr);
	std::lock_guard<std::mutex> lk_state(m_state);
	this->username = username;
	this->ipAddr = ipAddr;
	this->state = state;
}

//Costruttore 
utente::utente(std::string username, std::string ipAddr)
{
	std::lock_guard<std::mutex> lk_username(m_username);
	std::lock_guard<std::mutex> lk_ipAddr(m_ipAddr);
	this->username = username;
	this->ipAddr = ipAddr;
}

//Distruttore
utente::~utente()
{

}

std::string& utente::getUsername()
{
	std::lock_guard<std::mutex> lk_username(m_username);
	return this->username;
}

std::string& utente::getUsernamePc()
{
	std::lock_guard<std::mutex> lk_usernamePc(m_usernamePc);
	return this->usernamePc;
}

void utente::setUsername(std::string username)
{
	std::lock_guard<std::mutex> lk_username(m_username);
	this->username = username;
}

void utente::setUsernamePc(std::string usernamePc)
{
	std::lock_guard<std::mutex> lk_usernamePc(m_usernamePc);
	this->usernamePc = usernamePc;
}

std::vector<utente>& utente::getUtentiConnessi()
{
	std::lock_guard<std::mutex> lk_utentiConnessi(m_utentiConnessi);
	return utentiConnessi;
}

std::vector<utente> utente::getUtentiOnline() {
	std::lock_guard<std::mutex> lk_state(m_state);
	std::lock_guard<std::mutex> lk_utentiConnessi(m_utentiConnessi);
	std::vector<utente> utentiOnline;
	//Inserisco nel vettore utentiOnline solamente gli utenti che hanno stato STAT_ONLINE,
	//scandendo il vettore getUtentiConnessi
	for (auto it : this->utentiConnessi) {
		if (it.state == STAT_ONLINE) {
			utentiOnline.push_back(it);
		}
	}
	return utentiOnline;
}

bool utente::contieneUtente(std::string ipAddr) {
	std::lock_guard<std::mutex> lk_ipAddr(m_ipAddr);
	std::lock_guard<std::mutex> lk_utentiConnessi(m_utentiConnessi);
	//Scandisce il vettore utentiConnessi alla ricerca dell'utente con indirizzo ip ipAddr.
	//Torna un valore booleano a seconda del risultato della ricerca.
	for (auto it : this->utentiConnessi) {
		if (it.ipAddr == ipAddr)
			return true;
	}
	return false;
}

utente& utente::getUtente(std::string ipAddr) {
	std::lock_guard<std::mutex> lk_ipAddr(m_ipAddr);
	std::lock_guard<std::mutex> lk_utentiConnessi(m_utentiConnessi);
	//ricerca l'utente con indirizzo ip ipAddr, e ne torna il riferimento.
	//Se l'utente non esiste, lancia un eccezione.
	for (auto& it : this->utentiConnessi) {
		if (it.ipAddr == ipAddr) {
			return it;
		}
	}
	throw std::invalid_argument("Utente mancante.");
}

std::string utente::getUsernameFromIp(std::string ipAddr) {
	std::lock_guard<std::mutex> lk_username(m_username);
	std::lock_guard<std::mutex> lk_ipAddr(m_ipAddr);
	std::lock_guard<std::mutex> lk_utentiConnessi(m_utentiConnessi);
	//Tora il nome dell'utente a partire dall'ip ipAddr.
	for (auto it : this->utentiConnessi) {
		if (it.ipAddr == ipAddr) {
			return it.username;
		}
	}
	return "Errore: Nome non riconosciuto.";
}


void utente::addUtente(std::string username, std::string ipAddr, status state, boost::posix_time::ptime currentTime) {

	std::lock_guard<std::mutex> lk_ipAddr(m_ipAddr);
	std::lock_guard<std::mutex> lk_state(m_state);
	std::lock_guard<std::mutex> lk_currentTime(m_currentTime);
	std::lock_guard<std::mutex> lk_utentiConnessi(m_utentiConnessi);

	//Aggiunge un nuovo utente alla lista degli utenti connessi.
	utente nuovoUtente(username, ipAddr, state);
	nuovoUtente.currentTime = currentTime;
	nuovoUtente.ipAddr = ipAddr;
	nuovoUtente.state = state;
	this->utentiConnessi.push_back(nuovoUtente);
}



int utente::removeUtente(std::string ipAddr) {
	std::lock_guard<std::mutex> lk_ipAddr(m_ipAddr);
	std::lock_guard<std::mutex> lk_utentiConnessi(m_utentiConnessi);
	//Elimina l'utente con l'ip specificato.
	unsigned int i;
	for (i = 0; i<this->utentiConnessi.size(); i++) {
		if (this->utentiConnessi[i].ipAddr == ipAddr) {
			this->utentiConnessi.erase(utentiConnessi.begin() + i);
		}
	}
	return 0;

}

void utente::setCurrentTime(boost::posix_time::ptime currentTime) {
	std::lock_guard<std::mutex> lk_currentTime(m_currentTime);
	this->currentTime = currentTime;
}

void utente::setIpAddr(std::string ipAddr) {
	std::lock_guard<std::mutex> lk_ipAddr(m_ipAddr);
	this->ipAddr = ipAddr;
}

boost::posix_time::ptime utente::getTime() {
	std::lock_guard<std::mutex> lk_currentTime(m_currentTime);
	return this->currentTime;
}

std::string utente::getIpAddr() {
	std::lock_guard<std::mutex> lk_ipAddr(m_ipAddr);
	return this->ipAddr;
}

void utente::setState(status state) {
	std::lock_guard<std::mutex> lk_m_state(m_state);
	this->state = state;
}

status utente::getState() {
	std::lock_guard<std::mutex> lk_m_state(m_state);
	return this->state;
}




//Costruttore di copia
utente::utente(const utente& source) {
	std::lock_guard<std::mutex> lk_username(m_username);
	std::lock_guard<std::mutex> lk_ipAddr(m_ipAddr);
	std::lock_guard<std::mutex> lk_state(m_state);
	std::lock_guard<std::mutex> lk_currentTime(m_currentTime);
	std::lock_guard<std::mutex> lk_utentiConnessi(m_utentiConnessi);
	std::lock_guard<std::mutex> lk_usernamePc(m_usernamePc);
	this->ipAddr = source.ipAddr;
	this->state = source.state;
	this->currentTime = source.currentTime;
	this->usernamePc = source.usernamePc;
	this->username = source.username;
	for (auto it : source.utentiConnessi) {
		this->utentiConnessi.push_back(it);
	}
}

//Operatore di assegnazione
utente &utente::operator =(const utente & source) {
	std::lock_guard<std::mutex> lk_username(m_username);
	std::lock_guard<std::mutex> lk_ipAddr(m_ipAddr);
	std::lock_guard<std::mutex> lk_state(m_state);
	std::lock_guard<std::mutex> lk_currentTime(m_currentTime);
	std::lock_guard<std::mutex> lk_utentiConnessi(m_utentiConnessi);
	std::lock_guard<std::mutex> lk_usernamePc(m_usernamePc);
	if (this != &source) {
		this->ipAddr = source.ipAddr;
		this->state = source.state;
		this->currentTime = source.currentTime;
		this->username = source.username;
		this->usernamePc = source.usernamePc;
		for (auto it : source.utentiConnessi) {
			this->utentiConnessi.push_back(it);
		}
	}
	return *this;
}

