#pragma once

#include "utente.h"
#include "boost\asio.hpp"

void reciveTCPfile(utente& utenteProprietario, std::string generalPath);
void reciveAfterAccept(boost::asio::basic_stream_socket<boost::asio::ip::tcp>& s, utente& utenteProprietario, std::string generalPath);