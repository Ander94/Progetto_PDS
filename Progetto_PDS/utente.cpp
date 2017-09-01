#include <boost/asio.hpp>
#include "utente.h"
#include <iterator>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <algorithm>



utente::utente()
{
}

utente::utente(std::string username, std::string ipAddr, status state)
{
	std::lock_guard<std::recursive_mutex> lk_username(m_username);
	std::lock_guard<std::recursive_mutex> lk_ipAddr(m_ipAddr);
	std::lock_guard<std::recursive_mutex> lk_state(m_state);
	this->username = username;
	this->ipAddr = ipAddr;
	this->state = state;
}

utente::utente(std::string username, std::string ipAddr)
{
	std::lock_guard<std::recursive_mutex> lk_username(m_username);
	std::lock_guard<std::recursive_mutex> lk_ipAddr(m_ipAddr);
	this->username = username;
	this->ipAddr = ipAddr;
}

utente::~utente()
{
}

std::string utente::getUsername()
{
	std::lock_guard<std::recursive_mutex> lk_username(m_username);
	return this->username;
}

void utente::setUsername(std::string username)
{
	std::lock_guard<std::recursive_mutex> lk_username(m_username);
	this->username = username;
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
	for (auto it : this->getUtentiConnessi()) {
		if (it.getState()==STAT_ONLINE) {
			utentiOnline.push_back(it);
		}
	}
	return utentiOnline;
}

bool utente::contieneUtente(std::string username) {
	std::lock_guard<std::recursive_mutex> lk_username(m_username);
	std::lock_guard<std::recursive_mutex> lk_utentiConnessi(m_utentiConnessi);
	for (auto it : this->getUtentiConnessi()) {
		if (it.getUsername() == username)
			return true;
	}
	return false;
}

utente& utente::getUtente(std::string username){
	std::lock_guard<std::recursive_mutex> lk_username(m_username);
	std::lock_guard<std::recursive_mutex> lk_utentiConnessi(m_utentiConnessi);
	for (auto& it : this->getUtentiConnessi()) {
		if (it.getUsername() == username) {
			return it;
		}	
	}
	throw std::invalid_argument("Utente mancante.");
}

std::string utente::getUsernameFromIp(std::string ipAddr) {
	std::lock_guard<std::recursive_mutex> lk_username(m_username);
	std::lock_guard<std::recursive_mutex> lk_ipAddr(m_ipAddr);
	std::lock_guard<std::recursive_mutex> lk_utentiConnessi(m_utentiConnessi);
	for (auto it : this->getUtentiConnessi()) {
		if (it.getIpAddr() == ipAddr) {
			return it.getUsername();
		}
	}
	return NULL;
}


void utente::addUtente(std::string username, std::string ipAddr, status state,boost::posix_time::ptime currentTime) {
	std::lock_guard<std::recursive_mutex> lk_username(m_username);
	std::lock_guard<std::recursive_mutex> lk_ipAddr(m_ipAddr);
	std::lock_guard<std::recursive_mutex> lk_state(m_state);
	std::lock_guard<std::recursive_mutex> lk_utentiConnessi(m_utentiConnessi);

	utente nuovoUtente(username, ipAddr, state);
	nuovoUtente.setCurrentTime(currentTime);
	nuovoUtente.setIpAddr(ipAddr);
	nuovoUtente.setState(state);
	this->getUtentiConnessi().push_back(nuovoUtente);
}



int utente::removeUtente(std::string username) {
	std::lock_guard<std::recursive_mutex> lk_username(m_username);
	std::lock_guard<std::recursive_mutex> lk_state(m_state);
	std::lock_guard<std::recursive_mutex> lk_currentTime(m_currentTime);
	std::lock_guard<std::recursive_mutex> lk_utentiConnessi(m_utentiConnessi);

	//sistemare meglio ciclo per eliminare
	unsigned int i, j;
	for (i = 0; i<this->getUtentiConnessi().size(); i++) {
		if (this->getUtentiConnessi()[i].getUsername() == username) {
			for (j = i; j<this->getUtentiConnessi().size() - 1; j++) {
				this->getUtentiConnessi()[j].setUsername(this->getUtentiConnessi()[j + 1].getUsername());
				this->getUtentiConnessi()[j].setCurrentTime(this->getUtentiConnessi()[j + 1].getTime());
			}
			this->getUtentiConnessi().pop_back();
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



void utente::checkTime(utente& utenteProprietario, std::atomic<bool>& exit_app) {

	boost::posix_time::ptime currentTime;
	while (!exit_app.load()) {
		unsigned int i, j;
		//std::cout << "Controllo " << std::endl;
		for (i = 0; i < utenteProprietario.getUtentiConnessi().size(); i++) {
			currentTime = boost::posix_time::second_clock::local_time();
			if ((currentTime - utenteProprietario.getUtentiConnessi()[i].getTime()).total_seconds() > 3) {
				for (j = i; j < utenteProprietario.getUtentiConnessi().size() - 1; j++) {
					utenteProprietario.getUtentiConnessi()[j].setUsername(utenteProprietario.getUtentiConnessi()[j + 1].getUsername());
					utenteProprietario.getUtentiConnessi()[j].setCurrentTime(utenteProprietario.getUtentiConnessi()[j + 1].getTime());
					utenteProprietario.getUtentiConnessi()[j].setIpAddr(utenteProprietario.getUtentiConnessi()[j + 1].getIpAddr());
				}
				utenteProprietario.getUtentiConnessi().pop_back();
			}
		}
		Sleep(1000);
	}
}

void utente::setState(status state) {
	std::lock_guard<std::recursive_mutex> lk_m_state(m_state);
	this->state = state;
}

status utente::getState() {
	std::lock_guard<std::recursive_mutex> lk_m_state(m_state);
	return this->state;
}


utente::utente(const utente& source) {
	std::lock_guard<std::recursive_mutex> lk_username(m_username);
	std::lock_guard<std::recursive_mutex> lk_ipAddr(m_ipAddr);
	std::lock_guard<std::recursive_mutex> lk_state(m_state);
	std::lock_guard<std::recursive_mutex> lk_currentTime(m_currentTime);
	std::lock_guard<std::recursive_mutex> lk_utentiConnessi(m_utentiConnessi);

	this->ipAddr = source.ipAddr;
	this->state = source.state;
	this->currentTime = source.currentTime;
	this->username = source.username;
	for (auto it : source.utentiConnessi) {
		this->getUtentiConnessi().push_back(it);
	}
}

utente &utente::operator =(const utente & source) {
	std::lock_guard<std::recursive_mutex> lk_username(m_username);
	std::lock_guard<std::recursive_mutex> lk_ipAddr(m_ipAddr);
	std::lock_guard<std::recursive_mutex> lk_state(m_state);
	std::lock_guard<std::recursive_mutex> lk_currentTime(m_currentTime);
	std::lock_guard<std::recursive_mutex> lk_utentiConnessi(m_utentiConnessi);

	if (this != &source) {
		this->ipAddr = source.ipAddr;
		this->state = source.state;
		this->currentTime = source.currentTime;
		this->username = source.username;
		for (auto it : source.utentiConnessi) {
			this->getUtentiConnessi().push_back(it);
		}
	}
	return *this;
}