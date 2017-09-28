//COMMENTATO TUTTO

#include <boost/asio.hpp>
#include "utente.h"
#include <iterator>
#include <cstdlib>
#include <iostream>
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
	std::lock_guard<std::recursive_mutex> lk_username(m_username);
	std::lock_guard<std::recursive_mutex> lk_ipAddr(m_ipAddr);
	std::lock_guard<std::recursive_mutex> lk_state(m_state);
	this->username = username;
	this->ipAddr = ipAddr;
	this->state = state;
}

//Costruttore 
utente::utente(std::string username, std::string ipAddr)
{
	std::lock_guard<std::recursive_mutex> lk_username(m_username);
	std::lock_guard<std::recursive_mutex> lk_ipAddr(m_ipAddr);
	this->username = username;
	this->ipAddr = ipAddr;
}

//Distruttore
utente::~utente()
{

}

/*bool utente::immagineRicevuta(std::string ipAddr) {
	std::lock_guard<std::recursive_mutex> lk_(m_immaginiRegistrate);
	//Utilizzo un ciclo per scandire il vettore immaginiRegistrate, e ritorno un booleano che mi indica la presenza di quell'indirizzo IP
	for (unsigned int i = 0; i < this->immaginiRegistrate.size(); i++) {
		if (ipAddr == immaginiRegistrate[i])
			return true;
	}
	return false;
}
void utente::registraImmagine(std::string ipAddr) {
	//Inserisco un nuovo IP in immaginiRegistrate
	std::lock_guard<std::recursive_mutex> lk_(m_immaginiRegistrate);
	this->immaginiRegistrate.push_back(ipAddr);
}

void utente::rimuoviImmagine(std::string ipAddr) {
	std::lock_guard<std::recursive_mutex> lk_(m_immaginiRegistrate);
	//Elimino dal vettore l'elemento contenete la stringa specificata in ipAddr
	for (unsigned int i = 0; i < immaginiRegistrate.size(); i++) {
		if (ipAddr == immaginiRegistrate[i])
			this->immaginiRegistrate.erase(immaginiRegistrate.begin()+i);
	}
}*/


std::string& utente::getUsername()
{
	std::lock_guard<std::recursive_mutex> lk_username(m_username);
	return this->username;
}

std::string& utente::getUsernamePc()
{
	std::lock_guard<std::recursive_mutex> lk_usernamePc(m_usernamePc);
	return this->usernamePc;
}

void utente::setUsername(std::string username)
{
	std::lock_guard<std::recursive_mutex> lk_username(m_username);
	this->username = username;
}

void utente::setUsernamePc(std::string usernamePc)
{
	std::lock_guard<std::recursive_mutex> lk_usernamePc(m_usernamePc);
	this->usernamePc = usernamePc;
}

std::vector<utente>& utente::getUtentiConnessi()
{
	std::lock_guard<std::recursive_mutex> lk_utentiConnessi(m_utentiConnessi);
	return utentiConnessi;
}

std::vector<utente> utente::getUtentiOnline() {
	std::lock_guard<std::recursive_mutex> lk_state(m_state);
	std::lock_guard<std::recursive_mutex> lk_utentiConnessi(m_utentiConnessi);
	std::vector<utente> utentiOnline;
	//Inserisco nel vettore utentiOnline solamente gli utenti che hanno stato STAT_ONLINE,
	//scandendo il vettore getUtentiConnessi
	for (auto it : this->getUtentiConnessi()) {
		if (it.getState() == STAT_ONLINE) {
			utentiOnline.push_back(it);
		}
	}
	return utentiOnline;
}

bool utente::contieneUtente(std::string ipAddr) {
	std::lock_guard<std::recursive_mutex> lk_username(m_ipAddr);
	std::lock_guard<std::recursive_mutex> lk_utentiConnessi(m_utentiConnessi);
	//Scandisce il vettore utentiConnessi alla ricerca dell'utente con indirizzo ip ipAddr.
	//Torna un valore booleano a seconda del risultato della ricerca.
	for (auto it : this->getUtentiConnessi()) {
		if (it.getIpAddr() == ipAddr)
			return true;
	}
	return false;
}

utente& utente::getUtente(std::string ipAddr) {
	std::lock_guard<std::recursive_mutex> lk_username(m_ipAddr);
	std::lock_guard<std::recursive_mutex> lk_utentiConnessi(m_utentiConnessi);
	//ricerca l'utente con indirizzo ip ipAddr, e ne torna il riferimento.
	//Se l'utente non esiste, lancia un eccezione.
	for (auto& it : this->getUtentiConnessi()) {
		if (it.getIpAddr() == ipAddr) {
			return it;
		}
	}
	throw std::invalid_argument("Utente mancante.");
}

std::string utente::getUsernameFromIp(std::string ipAddr) {
	std::lock_guard<std::recursive_mutex> lk_username(m_username);
	std::lock_guard<std::recursive_mutex> lk_ipAddr(m_ipAddr);
	std::lock_guard<std::recursive_mutex> lk_utentiConnessi(m_utentiConnessi);
	//Tora il nome dell'utente a partire dall'ip ipAddr.
	for (auto it : this->getUtentiConnessi()) {
		if (it.getIpAddr() == ipAddr) {
			return it.getUsername();
		}
	}
	return "Errore: Nome non riconosciuto.";
}


void utente::addUtente(std::string username, std::string ipAddr, status state, boost::posix_time::ptime currentTime) {
	std::lock_guard<std::recursive_mutex> lk_username(m_username);
	std::lock_guard<std::recursive_mutex> lk_ipAddr(m_ipAddr);
	std::lock_guard<std::recursive_mutex> lk_state(m_state);
	std::lock_guard<std::recursive_mutex> lk_utentiConnessi(m_utentiConnessi);

	//Aggiunge un nuovo utente alla lista degli utenti connessi.
	utente nuovoUtente(username, ipAddr, state);
	nuovoUtente.setCurrentTime(currentTime);
	nuovoUtente.setIpAddr(ipAddr);
	nuovoUtente.setState(state);
	this->getUtentiConnessi().push_back(nuovoUtente);
}



int utente::removeUtente(std::string ipAddr) {
	std::lock_guard<std::recursive_mutex> lk_username(m_ipAddr);
	std::lock_guard<std::recursive_mutex> lk_utentiConnessi(m_utentiConnessi);

	//Elimina l'utente con l'ip specificato.
	unsigned int i;
	for (i = 0; i<this->getUtentiConnessi().size(); i++) {
		if (this->getUtentiConnessi()[i].getIpAddr() == ipAddr) {
			this->getUtentiConnessi().erase(getUtentiConnessi().begin() + i);
		}
	}
	return 0;

}

void utente::setCurrentTime(boost::posix_time::ptime currentTime) {
	std::lock_guard<std::recursive_mutex> lk_currentTime(m_currentTime);
	this->currentTime = currentTime;
}

void utente::setIpAddr(std::string ipAddr) {
	std::lock_guard<std::recursive_mutex> lk_ipAddr(m_ipAddr);
	this->ipAddr = ipAddr;
}

boost::posix_time::ptime utente::getTime() {
	std::lock_guard<std::recursive_mutex> lk_currentTime(m_currentTime);
	return this->currentTime;
}

std::string utente::getIpAddr() {
	std::lock_guard<std::recursive_mutex> lk_ipAddr(m_ipAddr);
	return this->ipAddr;
}

void utente::setState(status state) {
	std::lock_guard<std::recursive_mutex> lk_m_state(m_state);
	this->state = state;
}

status utente::getState() {
	std::lock_guard<std::recursive_mutex> lk_m_state(m_state);
	return this->state;
}

//Costruttore di copia
utente::utente(const utente& source) {
	std::lock_guard<std::recursive_mutex> lk_username(m_username);
	std::lock_guard<std::recursive_mutex> lk_ipAddr(m_ipAddr);
	std::lock_guard<std::recursive_mutex> lk_state(m_state);
	std::lock_guard<std::recursive_mutex> lk_currentTime(m_currentTime);
	std::lock_guard<std::recursive_mutex> lk_utentiConnessi(m_utentiConnessi);
	//std::lock_guard<std::recursive_mutex> lk_(m_immaginiRegistrate);
	this->ipAddr = source.ipAddr;
	this->state = source.state;
	this->currentTime = source.currentTime;
	this->username = source.username;
	for (auto it : source.utentiConnessi) {
		this->getUtentiConnessi().push_back(it);
	}
	/*for (auto it : source.immaginiRegistrate) {
		this->immaginiRegistrate.push_back(it);
	}*/
}

//Operatore di assegnazione
utente &utente::operator =(const utente & source) {
	std::lock_guard<std::recursive_mutex> lk_username(m_username);
	std::lock_guard<std::recursive_mutex> lk_ipAddr(m_ipAddr);
	std::lock_guard<std::recursive_mutex> lk_state(m_state);
	std::lock_guard<std::recursive_mutex> lk_currentTime(m_currentTime);
	std::lock_guard<std::recursive_mutex> lk_utentiConnessi(m_utentiConnessi);
	//std::lock_guard<std::recursive_mutex> lk_(m_immaginiRegistrate);
	if (this != &source) {
		this->ipAddr = source.ipAddr;
		this->state = source.state;
		this->currentTime = source.currentTime;
		this->username = source.username;
		for (auto it : source.utentiConnessi) {
			this->getUtentiConnessi().push_back(it);
		}
		/*for (auto it : source.immaginiRegistrate) {
			this->immaginiRegistrate.push_back(it);
		}*/
	}
	return *this;
}