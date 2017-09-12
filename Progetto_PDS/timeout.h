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
-L'endpoint da cui riceve tale messaggio.
**********************************************************************************/
int recive_from(udp::socket &s, char* buf, size_t dim_buf, boost::asio::ip::udp::endpoint& reciver_endpoint);

/********************************************************************************
Ritorna il numero di byte scritti sul socket s, presi dal buffer buf.
Riceve come parametri:
-Il socket su cui scrivere
-il buffer da cui leggere ciò da inviare
-la dimensione del buffer
**********************************************************************************/
int write_some(tcp::socket &s, char* buf, size_t dim_buf);

/********************************************************************************
Ritorna il numero di byte scritti sul socket s, presi dalla stringa message.
Riceve come parametri:
-Il socket su cui scrivere
-il messaggio da inviare sotto forma di stringa
-l'endpoint che riceverà il messsaggio.
**********************************************************************************/
int send_to(udp::socket &s, std::string message, boost::asio::ip::udp::endpoint& sender_endpoint);