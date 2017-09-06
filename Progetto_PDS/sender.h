#pragma once

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <atomic>
#include "utente.h"
#include "protoType.h"
#include "Settings.h"


/********************************************************************************
Invia in tutta la LAN un pacchetto nella forma username\r\nstato\r\n.
Riceve come parametri:
-L'username da inviare
-Lo stato da inviare
-l'atomic exit_app, che indica quando interrompere l'invio di messaggi UDP in LAN.
**********************************************************************************/
void sendUDPMessage(std::string& username, status& current_status, std::atomic<bool>& exit_app);