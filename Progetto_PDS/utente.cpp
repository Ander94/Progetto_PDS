#include <boost/asio.hpp>
#include "utente.h"
#include <mutex>
#include <string>
#include <iterator>
#include <cstdlib>
#include <iostream>
#include <memory>



utente::utente()
{
}

utente::utente(std::string username, std::string ipAddr)
{
	//std::lock_guard<std::recursive_mutex> lk(mut);
	this->username = username;
	this->ipAddr = ipAddr;
}

utente::~utente()
{
}

std::string utente::getUsername()
{
	//std::lock_guard<std::recursive_mutex> lk(mut);
	return this->username;
}

void utente::setUsername(std::string username)
{
	//std::lock_guard<std::recursive_mutex> lk(mut);
	this->username = username;
}

std::vector<utente>& utente::getUtentiConnessi()
{
	//std::lock_guard<std::recursive_mutex> lk(mut);
	return utentiConnessi;
}

bool utente::contieneUtente(std::string username) {
	//std::lock_guard<std::recursive_mutex> lk(mut);
	for (auto it : this->utentiConnessi) {
		if (it.getUsername() == username)
			return true;
	}
	return false;
}

utente& utente::getUtente(std::string username){
	for (auto& it : this->utentiConnessi) {
		if (it.getUsername() == username) {
			return it;
		}	
	}
	throw std::invalid_argument("Utente mancante.");
}

std::string utente::getUsernameFromIp(std::string ipAddr) {
	for (auto it : this->utentiConnessi) {
		if (it.getIpAddr() == ipAddr) {
			return it.getUsername();
		}
	}
	return NULL;
}


void utente::addUtente(std::string username, std::string ipAddr,boost::posix_time::ptime currentTime) {
	utente nuovoUtente(username, ipAddr);
	nuovoUtente.setCurrentTime(currentTime);
	nuovoUtente.setIpAddr(ipAddr);
	this->utentiConnessi.push_back(nuovoUtente);
}



int utente::removeUtente(std::string username) {
	//sistemare meglio ciclo per eliminare
	unsigned int i, j;
	for (i = 0; i<this->utentiConnessi.size(); i++) {
		if (this->utentiConnessi[i].getUsername() == username) {
			for (j = i; j<this->utentiConnessi.size() - 1; j++) {
				this->utentiConnessi[j].setUsername(this->utentiConnessi[j + 1].getUsername());
				this->utentiConnessi[j].setCurrentTime(this->utentiConnessi[j + 1].getTime());
			}
			this->utentiConnessi.pop_back();
		}
	}
	return 0;
	
}

void utente::setCurrentTime(boost::posix_time::ptime currentTime) {
	//std::cout << "vorrei mettere " << currentTime << std::endl;
	this->currentTime = currentTime;
	//std::cout << "Ho messo " << this->currentTime << std::endl;
}

void utente::setIpAddr(std::string ipAddr) {
	this->ipAddr = ipAddr;
}

/*void utente::setCurrentTime(std::string username, boost::posix_time::ptime currentTime) {
	for (auto& it : this->utentiConnessi) {
		if (it.getUsername() == username) {
			it.currentTime = currentTime;
		}
	}
}*/

boost::posix_time::ptime utente::getTime() {
	return this->currentTime;
}

std::string utente::getIpAddr() {
	return this->ipAddr;
}



void utente::checkTime(utente& utenteProprietario) {
	boost::posix_time::ptime currentTime;
	while (1) {
		unsigned int i, j;
		//std::cout << "Controllo " << std::endl;
		for (i = 0; i < utenteProprietario.getUtentiConnessi().size(); i++) {
			currentTime = boost::posix_time::second_clock::local_time();
			//std::cout << "il tempo passato per" <<  utenteProprietario.getUtentiConnessi()[i].getUsername() << "e' " << (currentTime - utenteProprietario.getUtentiConnessi()[i].getTime()).total_seconds() << std::endl;
			if ((currentTime - utenteProprietario.getUtentiConnessi()[i].getTime()).total_seconds() > 5) {
				for (j = i; j < utenteProprietario.getUtentiConnessi().size() - 1; j++) {
					utenteProprietario.getUtentiConnessi()[j].setUsername(utenteProprietario.getUtentiConnessi()[j + 1].getUsername());
					utenteProprietario.getUtentiConnessi()[j].setCurrentTime(utenteProprietario.getUtentiConnessi()[j + 1].getTime());
					utenteProprietario.getUtentiConnessi()[j].setIpAddr(utenteProprietario.getUtentiConnessi()[j + 1].getIpAddr());
				}
				utenteProprietario.getUtentiConnessi().pop_back();
			}
		}
		Sleep(5000);
	}
}

