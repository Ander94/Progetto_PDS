#pragma once

#include "boost\date_time\posix_time\posix_time.hpp"

#include <vector>
#include <string>
#include <mutex>

class utente
{
	//std::recursive_mutex mut;
	std::string username;
	std::string ipAddr;
	std::vector<utente> utentiConnessi;
	boost::posix_time::ptime currentTime;
	
public:
	utente();
	utente(std::string username, std::string ipAddr);
	~utente();
	std::string utente::getUsername();
	void utente::setIpAddr(std::string ipAddr);
	void utente::setUsername(std::string username);
	std::vector<utente>& utente::getUtentiConnessi();
	void utente::addUtente(std::string username, std::string ipAddr, boost::posix_time::ptime currentTime);
	bool utente::contieneUtente(std::string username);
	void utente::setCurrentTime(boost::posix_time::ptime currentTime);
	utente& utente::getUtente(std::string username);
	std::string utente::getIpAddr();
	boost::posix_time::ptime utente::getTime();
	int utente::removeUtente(std::string username);
	std::string getUsernameFromIp(std::string ipAddr);
	static void checkTime(utente& utenteProprietario);
};