#include <iostream>
#include <vector>
#include <string>
#pragma once
#include <atomic>
#include <iterator>
#include <fstream>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost\filesystem.hpp>
#include "UserProgressBar.h"
#include "WindowProgressBar.h"
#include "client.h"
#include "utente.h"
#include "sender.h"
#include "protoType.h"



/********************************************************************************
Riceve da tutta la LAN un pacchetto nella forma username\r\nstato\r\n.
Riceve come parametri:
-utenteProprietario: contiene tutti gli utenti iscritti
-generalPath: path da cui prelevare l'immagine del profilo.
-l'atomic exit_app, che indica quando interrompere la ricezione di messaggi UDP dalla LAN.
**********************************************************************************/
void reciveUDPMessage(utente& utenteProprietario, std::string generalPath, std::atomic<bool>& exit_app);

