#pragma once
#include <boost/thread.hpp>
#include <boost\asio.hpp>
#include <boost/filesystem.hpp>
#include <stdio.h>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include "utente.h"
#include "protoType.h"
#include "UserProgressBar.h"
#include "WindowProgressBar.h"
#include "protoType.h"


/********************************************************************************
Invia il file specificato in initialAbsolutePath all'user con username "username"
Riceve come parametri:
-utenteProprietario, ovvero il riferimento all'utente che ha aperto l'applicazione con tutti gli utenti ad esso connessi
-L'username dell'utente a cui inviare il file
-Il path assoluto del file da inviare
-Il riferimento alla barra di progresso
**********************************************************************************/
void sendTCPfile(utente& utenteProprietario, std::string ipAddr,
	std::string initialAbsolutePath, UserProgressBar* progBar);


/********************************************************************************
Invia l'immagine specificata in filePath all'user con ip "ipAddr"
Riceve come parametri:
-filePath: il path assoluto dell'immagine da inviare
-ipAddr: L'ip dell'utente a cui inviare l'immagine
**********************************************************************************/
void sendImage(std::string filePath, std::string ipAddr);