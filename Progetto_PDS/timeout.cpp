#include "timeout.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <future>
#include "wx\wx.h"

std::future<int> read_future;
std::future<int> write_future;

int read_some(tcp::socket &s, char* buf, size_t dim_buf) {

	//Lancio il thread in modo asincrono, cosi da non bloccarmi sulla lettura.
	//Avrò come risultato un ogetto di tipo future, che avrà valore:
	//- (-1) in caso di errore
	//- un valore maggiore di zero se la lettura è andata a buon fine
	read_future = std::async(std::launch::async, [&]() {
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
		std::future_status state = read_future.wait_for(std::chrono::seconds(TIMEOUT));
		//Se il risultato non è stato prodotto, vuol dire che read è rimasta bloccata per troppo tempo
		//ad esempio a causa della perdita di connessione
		if (state != std::future_status::ready) {
			throw std::invalid_argument("Attenzione: controllare la propria connessione prima di proseguire.");
		}
		else {
			//Altrimenti leggo il risultato della read, notificando un eventuale errore.
			dim_read = read_future.get();
			if (dim_read < 0) {
				throw std::invalid_argument("Attenzione l'utente ha interrotto l'invio del file.");
			}
		}
		return dim_read;
}


int write_some(tcp::socket &s, char* buf, size_t dim_buf) {

	//Lancio il thread in modo asincrono, cosi da non bloccarmi sulla scrittura.
	//Avrò come risultato un ogetto di tipo future, che avrà valore:
	//- (-1) in caso di errore
	//- un valore maggiore di zero se la scrittura è andata a buon fine
	write_future = std::async(std::launch::async, [&]() {
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
	std::future_status state = write_future.wait_for(std::chrono::seconds(TIMEOUT));
	//Se il risultato non è stato prodotto, vuol dire che write è rimasta bloccata per troppo tempo
	//ad esempio a causa della perdita di connessione
	if (state != std::future_status::ready) {
		throw std::invalid_argument("Attenzione: controllare la propria connessione prima di proseguire.");
	}
	else {
		//Altrimenti leggo il risultato della write, notificando un eventuale errore.
		dim_write = write_future.get();
		if (dim_write < 0) {
			throw std::invalid_argument("Attenzione l'utente ha interrotto la ricezione del file.");
		}
	}
	return dim_write;
}
