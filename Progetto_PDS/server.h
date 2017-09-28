#pragma once
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <windows.h>
#include "boost/asio.hpp"
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include "TaskBarIcon.h"
#include "Settings.h"
#include "utente.h"
#include "protoType.h"

/********************************************************************************
Riceve un file da un utente connesso al servizio, salvandolo sul path specificato come argomento
Riceve come parametri:
-utenteProprietario: ovvero il riferimento all'utente che ha aperto l'applicazione con tutti gli utenti ad esso connessi
-generalPath: path dove salvare il file ricevuto
-settings: riferimento utile per la grafica
-io_service: procedura di servizio boost, che verrà interrotta alla terminazione dell'applicazione per uscire dal thread
**********************************************************************************/
void reciveTCPfile(utente& utenteProprietario, std::string generalPath, Settings* settings, boost::asio::io_service& io_service);