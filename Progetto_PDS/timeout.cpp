#include "timeout.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <future>

std::future<int> read_future_tcp;
std::future<int> write_future_tcp;
std::future<int> read_future_udp;
std::future<int> write_future_udp;

int read_some(tcp::socket &s, char* buf, size_t dim_buf) {

	//Lancio il thread in modo asincrono, cosi da non bloccarmi sulla lettura.
	//Avrò come risultato un ogetto di tipo future, che avrà valore:
	//- (-1) in caso di errore
	//- un valore maggiore di zero se la lettura è andata a buon fine
	read_future_tcp = std::async(std::launch::async, [&]() {
			char *buffer = (char*)malloc((dim_buf + 1) * sizeof(char));
			int dim_read;
			try {
				//Leggo dal socket sul buffer "buffer"
				dim_read = s.read_some(boost::asio::buffer(buffer, dim_buf));
				//E copio in memoria il contenuto di buffer in buf
				memcpy(buf, buffer, dim_read);
				//Libero la memoria e torno la dimensione letta
				free(buffer);
				return dim_read;
			}
			catch (...) {
				//In caso di eccezione, libero la memoria e torno il valore -1.
				free(buffer);
				return -1;
			}
		});

		int dim_read = -1;
		//Attendo che std::async produca il risultato per un tempo TIMEOUT
		std::future_status state = read_future_tcp.wait_for(std::chrono::seconds(TIMEOUT));
		//Se il risultato non è stato prodotto, vuol dire che read è rimasta bloccata per troppo tempo
		//ad esempio a causa della perdita di connessione
		if (state != std::future_status::ready) {
			throw std::invalid_argument("Attenzione: la connessione con l'utente è stata persa.");
		}
		else {
			//Altrimenti leggo il risultato della read, notificando un eventuale errore.
			dim_read = read_future_tcp.get();
			if (dim_read < 0) {
				throw std::invalid_argument("Attenzione l'utente ha interrotto l'invio del file.");
			}
		}
		return dim_read;
}

int recive_from(udp::socket &s, char* buf, size_t dim_buf, boost::asio::ip::udp::endpoint& reciver_endpoint) {

	//Lancio il thread in modo asincrono, cosi da non bloccarmi sulla lettura.
	//Avrò come risultato un ogetto di tipo future, che avrà valore:
	//- (-1) in caso di errore
	//- un valore maggiore di zero se la lettura è andata a buon fine
	read_future_udp = std::async(std::launch::async, [&]() {
		char *buffer = (char*)malloc((dim_buf + 1) * sizeof(char));
		int dim_read;
		try {
			//Leggo dal socket sul buffer "buffer"
			dim_read = s.receive_from(boost::asio::buffer(buffer, dim_buf), reciver_endpoint);
			//E copio in memoria il contenuto di buffer in buf
			memcpy(buf, buffer, dim_read);
			//Libero la memoria e torno la dimensione letta
			free(buffer);
			return dim_read;
		}
		catch (...) {
			//In caso di eccezione, libero la memoria e torno il valore -1.
			free(buffer);
			return -1;
		}
	});

	int dim_read = -1;
	//Attendo che std::async produca il risultato per un tempo TIMEOUT
	std::future_status state = read_future_udp.wait_for(std::chrono::seconds(TIMEOUT));
	//Se il risultato non è stato prodotto, vuol dire che read è rimasta bloccata per troppo tempo
	//ad esempio a causa della perdita di connessione
	if (state != std::future_status::ready) {
		throw std::invalid_argument("Attenzione: la connessione è stata persa.\nControllare lo stato della propria connessione.");
	}
	else {
		//Altrimenti leggo il risultato della read, notificando un eventuale errore.
		dim_read = read_future_udp.get();
		if (dim_read < 0) {
			throw std::invalid_argument("Attenzione: la connessione è stata persa.\nControllare lo stato della propria connessione.\nSe il problema persiste, riavviare il programma.\n");
		}
	}
	return dim_read;
}


int write_some(tcp::socket &s, char* buf, size_t dim_buf) {

	//Lancio il thread in modo asincrono, cosi da non bloccarmi sulla scrittura.
	//Avrò come risultato un ogetto di tipo future, che avrà valore:
	//- (-1) in caso di errore
	//- un valore maggiore di zero se la scrittura è andata a buon fine
	write_future_tcp = std::async(std::launch::async, [&]() {
		try {
			//Scrivo sul socket "s" il contenuto di "buf"
			int dim_write = s.write_some(boost::asio::buffer(buf, dim_buf));
			return dim_write;
		}
		catch (...) {
			//In caso di eccezione, torno il valore -1
			return -1;
		}
	});
	int dim_write = -1;
	//Attendo che std::async produca il risultato per un tempo TIMEOUT
	std::future_status state = write_future_tcp.wait_for(std::chrono::seconds(TIMEOUT));
	//Se il risultato non è stato prodotto, vuol dire che write è rimasta bloccata per troppo tempo
	//ad esempio a causa della perdita di connessione
	if (state != std::future_status::ready) {
		throw std::invalid_argument("Attenzione: la connessione con l'utente è stata persa.");
	}
	else {
		//Altrimenti leggo il risultato della write, notificando un eventuale errore.
		dim_write = write_future_tcp.get();
		if (dim_write < 0) {
			throw std::invalid_argument("Attenzione l'utente ha interrotto la ricezione del file.");
		}
	}
	return dim_write;
}


int send_to(udp::socket &s, std::string message, boost::asio::ip::udp::endpoint& sender_endpoint) {

	//Lancio il thread in modo asincrono, cosi da non bloccarmi sulla scrittura.
	//Avrò come risultato un ogetto di tipo future, che avrà valore:
	//- (-1) in caso di errore
	//- un valore maggiore di zero se la scrittura è andata a buon fine
	write_future_udp = std::async(std::launch::async, [&]() {
		try {
			//Scrivo sul socket "s" il contenuto di "buf"
			int dim_write = s.send_to(boost::asio::buffer(message), sender_endpoint);
			return dim_write;
		}
		catch (...) {
			//In caso di eccezione, torno il valore -1
			return -1;
		}
	});
	int dim_write = -1;
	//Attendo che std::async produca il risultato per un tempo TIMEOUT
	std::future_status state = write_future_udp.wait_for(std::chrono::seconds(TIMEOUT));
	//Se il risultato non è stato prodotto, vuol dire che write è rimasta bloccata per troppo tempo
	//ad esempio a causa della perdita di connessione
	if (state != std::future_status::ready) {
		throw std::invalid_argument("Attenzione: la connessione è stata persa.\nControllare lo stato della propria connessione.");
	}
	else {
		//Altrimenti leggo il risultato della write, notificando un eventuale errore. L'errore potrebbe essere dovuto al cambio del proprio indirizzo IP
		dim_write = write_future_udp.get();
		if (dim_write < 0) {
			throw std::invalid_argument("Attenzione: la connessione è stata persa.\nControllare lo stato della propria connessione.\nSe il problema persiste, riavviare il programma.\n");
		}
	}
	return dim_write;
}