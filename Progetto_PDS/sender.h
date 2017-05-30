#pragma once

#include <boost/thread.hpp>
#include <iostream>
#include <vector>
#include <string>
#include "utente.h"


enum status {
	STAT_ONLINE = 0, 
	STAT_OFFLINE = 1
};

void sendUDPMessage(std::string& username, status& current_status);
