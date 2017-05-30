#pragma once

#include "utente.h"
#include "boost\asio.hpp"

void reciveTCPfile(utente& utenteProprietario);
void reciveAfterAccept(boost::asio::basic_stream_socket<boost::asio::ip::tcp>& s, utente& utenteProprietario);