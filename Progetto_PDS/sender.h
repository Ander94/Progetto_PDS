#pragma once

#include <boost/thread.hpp>
#include <iostream>
#include <vector>
#include <string>
#include "utente.h"




enum save_request {
	SAVE_REQUEST_YES = 0,
	SAVE_REQUEST_NO = 1
};


void sendUDPMessage(std::string& username, status& current_status);
