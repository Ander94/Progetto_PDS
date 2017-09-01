#include <iostream>
#include <vector>
#include <string>
#include <atomic>
#include "utente.h"
#include <boost/thread.hpp>
#include "UserProgressBar.h"
#include "WindowProgressBar.h"
#include "client.h"
void reciveUDPMessage(utente& utenteProprietario, std::string generalPath, std::atomic<bool>& exit_app);

