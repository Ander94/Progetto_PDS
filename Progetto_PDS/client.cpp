#include <fstream>
#include <cstdlib>
#include <cstring>
#include "client.h"
#include "protoType.h"
#include <stdio.h>
#include <filesystem>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include "boost\asio.hpp"
#include <boost/filesystem.hpp>

namespace bf = boost::filesystem;
using boost::asio::ip::tcp;

/********************************************************************************
Invia un il file specificato in filePath sul socket s.
Riceve come parametri:
-Il socket sul quale inviare il file
-Il path assoluto del file da inviare
-Il path relativo del file da inviare
-Il riferimento alla barra di progresso per la grafica.
**********************************************************************************/
void send_file(boost::asio::basic_stream_socket<boost::asio::ip::tcp>& s, 
	std::string filePath, std::string sendPath, UserProgressBar* progBar);


/********************************************************************************
Invia la directory specificata in initialAbsolutePath sul socket s
Riceve come parametri:
-Il socket sul quale inviare il file
-Il path assoluto della directory da inviare
-Il nome della directory da inviare
-Il riferimento alla barra di progresso per la grafica.
**********************************************************************************/
void send_directory(boost::asio::basic_stream_socket<boost::asio::ip::tcp>& s, 
	std::string initialAbsolutePath, std::string folder, UserProgressBar* progBar);


/********************************************************************************
Ritorna il path relativo del file da inviare a partire dalla cartella folder.
Riceve come parametri:
-Il path assoluto del file da inviare
-Il path assoluto della directory iniziale
-La cartella da cui far partire il path
ES:
	absolutePath: C:\Utente\Desktop\download\sotto_cartella_1\sotto_cartella_2\prova.txt
	initialAbsolutePath: C:\Utente\Desktop\download
	folder: download
Viene tornata la stringa
	download\sotto_cartella_1\sotto_cartella_2\prova.txt
**********************************************************************************/
std::string relative_path(std::string absolutePath, std::string initialAbsolutePath, std::string folder);

/********************************************************************************
Ritorna la dimensione della cartella specificata in absolutePath
Riceve come parametro il path assoluto.
**********************************************************************************/
size_t folder_size(std::string absolutePath);

/********************************************************************************
Sgancia un thread che permette l'invio deò file specificato in initalAbsolutePath.
Riceve come parametri:
-utenteProprietario, ovvero il riferimento all'utente che ha aperto l'applicazione con tutti gli utenti ad esso connessi
-L'username dell'utente a cui inviare il file
-Il path assoluto del file da inviare
-Il riferimento alla barra di progresso
**********************************************************************************/
void sendThreadTCPfile(utente& utenteProprietario, std::string username, 
	std::string initialAbsolutePath, UserProgressBar* progBar);



/*IMPLEMENTAZIONE*/


void sendTCPfile(utente& utenteProprietario, std::string username, std::string initialAbsolutePath, UserProgressBar* progBar) {
	//Sgancia il thread che permette l'invio del file
	std::thread(sendThreadTCPfile, std::ref(utenteProprietario), username, initialAbsolutePath, progBar).detach();
}



void sendImage(std::string filePath, std::string ipAddr) {

	boost::asio::io_service io_service;
	tcp::resolver resolver(io_service);
	tcp::resolver::query query(tcp::v4(), ipAddr, std::to_string(PORT_TCP));
	tcp::resolver::iterator iterator = resolver.resolve(query);
	tcp::socket s(io_service);

	//Mi connetto al server, se qualcosa non va a buon fine, ritorno al main
	try {
		boost::asio::connect(s, iterator);
	}
	catch (std::exception e) {
		return throw std::invalid_argument("Non sono riuscito a connettermi con l'utente. Controllare che l'utente sia attivo.");
	}


	try {
		std::ifstream file_in(filePath, std::ios::in | std::ios::binary);
		std::string fileSize;
		size_t dim_to_send;
		char buf_response[PROTOCOL_PACKET];
		std::string buf_send, response;
		size_t length;
		double dim_write;
		char buf_to_send[BUFLEN];

		long int size = boost::filesystem::file_size(filePath);
		fileSize = std::to_string(size);

		if (file_in.is_open())
		{
			//Invio +IM per dire che è un file immagine
			buf_send = "+IM";
			boost::asio::write(s, boost::asio::buffer(buf_send));
			//Vedo se il server mi ha detto che va ok
			length = s.read_some(boost::asio::buffer(buf_response, PROTOCOL_PACKET));
			buf_response[length] = '\0';
			response = buf_response;
			if (response != "+OK") {
				return throw std::invalid_argument("Il server ha dato risposta negativa per la ricezione dell'immagine.");
			}

			//Invio qui la dimensione del file
			boost::asio::write(s, boost::asio::buffer(fileSize));

			length = s.read_some(boost::asio::buffer(buf_response, PROTOCOL_PACKET));
			buf_response[length] = '\0';
			if (response != "+OK") {
				return throw std::invalid_argument("Il server ha dato risposta negativa per la ricezione dell'immagine.");
			}


			dim_to_send = size;
			while (dim_to_send > 0) {
				dim_write = dim_to_send < BUFLEN ? dim_to_send : BUFLEN;
				dim_to_send -= dim_write;
				file_in.read(buf_to_send, dim_write);
				boost::asio::write(s, boost::asio::buffer(buf_to_send, (int)dim_write));
			}

			length = s.read_some(boost::asio::buffer(buf_response, PROTOCOL_PACKET));
			buf_response[length] = '\0';
			if (response != "+OK") {
				return throw std::invalid_argument("Il server ha dato risposta negativa per la ricezione dell'immagine.");
			}
			file_in.close();
		}
		else {
			return throw std::invalid_argument("Errore nell'invio dell'immagine. Immagine inesistente.");
		}
	}
	catch (std::exception& e)
	{
		return throw std::invalid_argument(e.what());
	}

	//A QUI
	s.close();
	io_service.stop();
}

void sendThreadTCPfile(utente& utenteProprietario, std::string username, std::string initialAbsolutePath, UserProgressBar* progBar) {

	std::string ipAddr;       //Indirizzo ip dell'utente a cui inviare il file
	std::string basename;       //Nome dell'inizio da inviare

	wxThreadEvent event(wxEVT_THREAD, CLIENT_EVENT);

	//controllare se l'user esiste
	try {
		ipAddr = utenteProprietario.getUtente(username).getIpAddr(); //Se l'username non esiste, lancio un eccezione e torno, altrimenti ottengo il suo ip
	}
	catch (std::exception e) {
		wxQueueEvent(progBar, event.Clone());
		wxMessageBox(e.what(), wxT("Errore"), wxOK | wxICON_ERROR);
		return;
	}
	
	//Se la directory/file specificata/o non è valida/o, ritorno al main
	if (!boost::filesystem::is_directory(initialAbsolutePath) && !boost::filesystem::is_regular_file(initialAbsolutePath)) {
		wxMessageBox("Path specificato non valido.", wxT("Errore"), wxOK | wxICON_ERROR);
		return;
	}

	//Ottengo il nome della cartella che contiene il file da inviare 
	basename = boost::filesystem::basename(initialAbsolutePath);

	//Ne ottengo il path assoluto
	boost::filesystem::path path_absolutePath(initialAbsolutePath); 

	// Avvio la procedura di servizio boost, che servirà a mettersi in contatto con il server
	//La porta utilizzata in ambito TCP e' la 1400																		  
	boost::asio::io_service io_service;
	tcp::resolver resolver(io_service);
	tcp::resolver::query query(tcp::v4(), ipAddr, std::to_string(PORT_TCP));
	tcp::resolver::iterator iterator = resolver.resolve(query);
	tcp::socket s(io_service);

	//Mi connetto al server, se qualcosa non va a buon fine, ritorno al main
	try {
		boost::asio::connect(s, iterator);
	}
	catch (std::exception e) {
		wxQueueEvent(progBar, event.Clone());
		wxMessageBox("Non sono riuscito a connettermi con l'utente. Controllare che l'utente sia attivo", wxT("Errore"), wxOK | wxICON_ERROR);
		return;
	}

	//Se sto inviando un file, chiamo send_file
	if (boost::filesystem::is_regular_file(path_absolutePath)) {
		try {
			send_file(s, initialAbsolutePath, basename + boost::filesystem::extension(initialAbsolutePath), progBar);
		}
		catch (std::exception& e) {
			wxQueueEvent(progBar, event.Clone());
			wxMessageBox(e.what(), wxT("Errore"), wxOK | wxICON_ERROR);
		}
		
	}
	//Senno chiamo send_directory
	else if (boost::filesystem::is_directory(path_absolutePath)) {
		try {
			send_directory(s, initialAbsolutePath, basename, progBar);
		}
		catch (std::exception& e) {
			wxQueueEvent(progBar, event.Clone());
			wxMessageBox(e.what(), wxT("Errore"), wxOK | wxICON_ERROR);
		}
	}
	//Chiudo il socket e la procedurea di servizio
	s.close();
	io_service.stop();
	//segnalo la fine del trasferimento alla GUI
	wxQueueEvent(progBar, event.Clone());
	return;

}


void send_directory(boost::asio::basic_stream_socket<boost::asio::ip::tcp>& s, 
	std::string initialAbsolutePath, std::string folder, UserProgressBar* progBar) {
	
	//Dimensione della directory in char e size_t
	std::string directorySize;
	size_t directory_size_to_send;
	size_t directory_size_send = 0;
	std::ostringstream convert;
	std::string buf_send, response;
	
	char buf_recive[PROTOCOL_PACKET];
	boost::posix_time::ptime start = boost::posix_time::second_clock::local_time();   //tempo in cui viene lanciata la funzione, utile per il calcolo del tempo residuo
	boost::posix_time::ptime end;   //Utile per il calcolo del tempo residuo
	long int dif = 0, min = 0, sec = 0;
	int length;

	//Dico che voglio inviare un direttorio
	buf_send = "+DR\0";
	boost::asio::write(s, boost::asio::buffer(buf_send));

	//Vedo se il server mi risponde con ok
	length = s.read_some(boost::asio::buffer(buf_recive, PROTOCOL_PACKET));
	buf_recive[length] = '\0';
	response = buf_recive;
	if (response != "+OK") {
		return throw std::invalid_argument("Errore nell'invio del direttorio.");
	}

	//Devo inviare qui la dimensione
	directory_size_to_send = folder_size(initialAbsolutePath); // dimensione directory
	
	wxThreadEvent event(wxEVT_THREAD, SetMaxDir_EVENT);
	event.SetExtraLong(directory_size_to_send);
	wxQueueEvent(progBar, event.Clone());
	//progBar->SetMaxDir(directory_size_to_send); //<-- inizializzo barra progresso directory
	
	//Converto in string la dimensione cosi da poterla inviare al server.
	convert << directory_size_to_send;
	directorySize = convert.str();
	boost::asio::write(s, boost::asio::buffer(directorySize));
	length = s.read_some(boost::asio::buffer(buf_recive, PROTOCOL_PACKET));
	buf_recive[length] = '\0';
	response = buf_recive;
	if (response != "+OK") {
		return throw std::invalid_argument("Errore nell'invio del direttorio.");
	}


	//Attenzione:qua bisogna inviare il relativePath
	//Il concetto è questo: noi utilizziamo il path assoluto, ma a lui dobbiamo inviare quello relativo
	//Creo la cartella a partire dal path assoluto

	boost::asio::write(s, boost::asio::buffer(folder));
	length = s.read_some(boost::asio::buffer(buf_recive, PROTOCOL_PACKET));
	buf_recive[length] = '\0';
	response = buf_recive;
	if (response != "+OK") {
		return throw std::invalid_argument("Attenzione: l'utente ha rifiutato il trasferimento.");
	}

	for (bf::recursive_directory_iterator it(initialAbsolutePath); it != bf::recursive_directory_iterator(); ++it)
	{
		if (!bf::is_directory(*it)) {
			bf::path p(bf::absolute(*it));

			wxThreadEvent event(wxEVT_THREAD, SetNewFile_EVENT);
			event.SetString(p.filename().string());
			wxQueueEvent(progBar, event.Clone());
			//progBar->SetNewFile(p.filename().string()); //<-- inizializzo barra progresso file
			try {
				send_file(s, bf::absolute(*it).string(), relative_path(bf::absolute(*it).string(), initialAbsolutePath, folder), progBar);
				if (progBar->testAbort())
					return;
			}
			catch (std::exception&) {
				return throw std::invalid_argument("Attenzione: l'utente ha interrotto il trasferimento");
			}
			directory_size_send += (size_t)bf::file_size(*it);

			if (directory_size_send > 0) {
				end = boost::posix_time::second_clock::local_time();
				dif = (end - start).total_seconds();
				min = (int)(((directory_size_to_send - directory_size_send) / ((directory_size_send)))*dif) / 60;
				sec = (int)(((directory_size_to_send - directory_size_send) / ((directory_size_send)))*dif) % 60;
			}


			int length = s.read_some(boost::asio::buffer(buf_recive, PROTOCOL_PACKET));
			buf_recive[length] = '\0';
			std::string  response(buf_recive);
			if (response != "+CN") {
				return throw std::invalid_argument("Errore nell'invio del direttorio.");
			}

		}
		else {
			buf_send = "+DR\0";
			boost::asio::write(s, boost::asio::buffer(buf_send));
			int length = s.read_some(boost::asio::buffer(buf_recive, PROTOCOL_PACKET));
			buf_recive[length] = '\0';
			response = buf_recive;
			if (response != "+OK") {
				wxMessageBox("Errore nell'invio del direttorio.", wxT("Errore"), wxOK | wxICON_ERROR);
				return throw std::invalid_argument("Errore nell'invio del direttorio.");
			}
			boost::asio::write(s, boost::asio::buffer(relative_path(bf::absolute(*it).string(), initialAbsolutePath, folder)));
			length = s.read_some(boost::asio::buffer(buf_recive, PROTOCOL_PACKET));
			buf_recive[length] = '\0';
			response = buf_recive;
			if (response != "+OK") {
				return throw std::invalid_argument("Errore nell'invio del direttorio.");
			}
		}
	}
	buf_send = "-END";
	boost::asio::write(s, boost::asio::buffer(buf_send));
}

void send_file(boost::asio::basic_stream_socket<boost::asio::ip::tcp>& s, 
	std::string filePath, std::string sendPath, UserProgressBar* progBar) {

	std::ifstream file_in(filePath, std::ios::in | std::ios::binary);
	std::string fileSize; //Dimensione del file sotto forma di stringa 
	long int size;    //Dimensione del file sotto forma di intero
	boost::posix_time::ptime start = boost::posix_time::second_clock::local_time();
	boost::posix_time::ptime end;
	long int dif = 0, min = 0, sec = 0;
	double dim_write, dim_send = 0;
	int calcola_tempo = 1; //è utile per non valutare il tempo troppe volte, ma solo ogni 50 volte.
	std::ostringstream convert;
	std::string response; //Risposta del server
	std::string buf_send; //Buffer utile all'invio di un pacchetto
	char buf_to_send[BUFLEN]; //char utili per caricare il pacchetto di lunghezza BUFLEN da inviare al server.
	char buf_recive[PROTOCOL_PACKET]; //Buffer utile alla ricezione di un pacchetto. La funzione da me utilizzata prevede di ricevere un char di lunghezza fissa.
	size_t length; //Lunghezza della risposta ricevuta.

	try {
		
		//Valuto la dimensione del file 
		size = bf::file_size(filePath);


		wxThreadEvent event(wxEVT_THREAD, SetMaxFile_EVENT);
		event.SetExtraLong(size);
		wxQueueEvent(progBar, event.Clone());
		//progBar->SetMaxFile(size);  //<-- inizializzo barra di progresso

		//Converto la dimensione in una stringa cosi da poterla inviare al server
		fileSize = std::to_string(size);
		
		if (file_in.is_open())
		{
			//Qui inizia il protocollo di invio del file 
			//Invio +FL per dire che è un file
			buf_send = "+FL\0";
			boost::asio::write(s, boost::asio::buffer(buf_send));
			//Vedo se il server mi ha detto che va ok
			length = s.read_some(boost::asio::buffer(buf_recive, PROTOCOL_PACKET));
			buf_recive[length] = '\0';
			response = buf_recive;
			if (response != "+OK") {
				return throw std::invalid_argument("Errore nell'invio del file.");
			}
			//invio qui il nome del file
			boost::asio::write(s, boost::asio::buffer(sendPath));
			//leggere un buffer e portarlo in una stringa

			length = s.read_some(boost::asio::buffer(buf_recive, PROTOCOL_PACKET));
			buf_recive[length] = '\0';
			response.clear();
			response = buf_recive;
			if (response != "+OK") {
				return throw std::invalid_argument("Attenzione: l'utente ha rifiutato il trasferimento.");
			}

			//Invio qui la dimensione del file
			boost::asio::write(s, boost::asio::buffer(fileSize));

			length = s.read_some(boost::asio::buffer(buf_recive, PROTOCOL_PACKET));
			buf_recive[length] = '\0';
			if (response != "+OK") {
				return throw std::invalid_argument("Errore nell'invio del file.");
			}

			
			wxThreadEvent event1(wxEVT_THREAD, IncFile_EVENT);
			wxThreadEvent event2(wxEVT_THREAD, SetTimeFile_EVENT);
			//Carico il buffer da caricare 
			size_t dim_to_send = size;
			while (dim_send < size && !progBar->testAbort()) {
				//Funzionante
				/*dim_write = 0;
				while (dim_write < BUFLEN && dim_send < size) {
					file_in.get(c);
					buf_to_send[(int)dim_write] = c;
					dim_write++;
					dim_send++;
				}
				//Invio il buffer al server, ricordare di mettere un controllo qui
				boost::asio::write(s, boost::asio::buffer(buf_to_send, (int)dim_write));*/

				dim_write = dim_to_send < BUFLEN ? dim_to_send : BUFLEN;
				dim_send += dim_write;
				dim_to_send -= dim_write;
				file_in.read(buf_to_send, dim_write);
				boost::asio::write(s, boost::asio::buffer(buf_to_send, (int)dim_write));

				if (calcola_tempo % 50 == 0)
				{
					end = boost::posix_time::second_clock::local_time();
					dif = (end - start).total_seconds();
					//min = (int)(((size - dim_send) / ((dim_send)))*dif) / 60;
					//sec = (int)((((size - dim_send) / ((dim_send)))*dif)) % 60;
					sec = (int)((((size - dim_send) / ((dim_send)))*dif));
					
				}
				calcola_tempo++;	

				event1.SetExtraLong(dim_send);
				wxQueueEvent(progBar, event1.Clone());
				event2.SetExtraLong(sec);
				wxQueueEvent(progBar, event2.Clone());

				//progBar->IncFile(dim_send);
				//progBar->SetTimeFile(sec);
				
			}
			file_in.close();
		}
		else {
			return throw std::invalid_argument("File non aperto correttamente.");
		}
	}
	catch (std::exception&)
	{
		return throw std::invalid_argument("Attenzione: l'utente ha interrotto il trasferimento");
	}
}




std::string relative_path(std::string absolutePath, std::string initialAbsolutePath, std::string folder) {
	std::string relativePath;
	if (initialAbsolutePath[initialAbsolutePath.length() - 1] == '\\')
		relativePath = absolutePath.erase(0, initialAbsolutePath.length() - folder.length() - 1);
	else
		relativePath = absolutePath.erase(0, initialAbsolutePath.length() - folder.length());

	for (unsigned int i = 0; i < relativePath.length(); i++)
		if (relativePath[i] == '\\')
			relativePath[i] = '/';

	return relativePath;
}


size_t folder_size(std::string absolutePath) {

	size_t size = 0;
	for (bf::recursive_directory_iterator it(absolutePath); it != bf::recursive_directory_iterator(); ++it)
	{
		if (!bf::is_directory(*it)) {
			size += (size_t)bf::file_size(*it);
		}
	}

	return size;
}