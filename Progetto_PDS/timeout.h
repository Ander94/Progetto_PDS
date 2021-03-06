#pragma once
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <iostream>
#include <string>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include <thread>
#include <chrono>
#include "protoType.h"
using boost::asio::ip::tcp;
using boost::asio::ip::udp;

/********************************************************************************
Ritorna il numero di byte letti dal socket s, presi dal buffer buf.
Riceve come parametri:
-Il socket da cui leggere
-il buffer su cui salvare il risultato
-la dimensione del buffer
**********************************************************************************/
int read_some(tcp::socket &s, char* buf, size_t dim_buf); 


/********************************************************************************
Ritorna il numero di byte letti dal socket s, presi dal buffer buf.
Riceve come parametri:
-Il socket da cui leggere
-il buffer su cui salvare il risultato
-la dimensione del buffer
-il valore del timeout
**********************************************************************************/
int read_some(tcp::socket &s, char* buf, size_t dim_buf, int timeout);


/********************************************************************************
Ritorna il numero di byte scritti sul socket s, presi dal buffer buf.
Riceve come parametri:
-Il socket su cui scrivere
-il buffer da cui leggere ci� da inviare
-la dimensione del buffer
**********************************************************************************/
int write_some(tcp::socket &s, char* buf, size_t dim_buf);


/********************************************************************************
Ritorna il numero di byte scritti sul socket s, presi dal buffer buf.
Riceve come parametri:
-Il socket su cui scrivere
-la stringa da inviare sul socket
**********************************************************************************/
void write_some(tcp::socket &s, std::string& buf);


/********************************************************************************
Ritorna il numero di byte scritti sul socket s, presi dal buffer buf.
Riceve come parametri:
-Il socket su cui scrivere
-il buffer da cui leggere ci� da inviare
-la dimensione del buffer
-il valore del timeout
**********************************************************************************/
int write_some(tcp::socket &s, char* buf, size_t dim_buf, int timeout);


/********************************************************************************
Ritorna il numero di byte scritti sul socket s, presi dal buffer buf.
Riceve come parametri:
-Il socket su cui scrivere
-la stringa da inviare sul socket
-il valore del timeout
**********************************************************************************/
void write_some(tcp::socket &s, std::string& buf, int timeout);
