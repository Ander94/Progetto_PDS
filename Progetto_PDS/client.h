#pragma once

#include <boost/thread.hpp>
#include <iostream>
#include <vector>
#include <string>
#include "utente.h"

#include "UserProgressBar.h"
#include "WindowProgressBar.h"

void sendTCPfile(utente& utenteProprietario, std::string username, 
	std::string initialAbsolutePath, UserProgressBar* progBar);

//IP, path dell'immagine
void sendImage(std::string filePath, std::string ipAddr);
