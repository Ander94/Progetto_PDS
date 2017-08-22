#include <fstream>
#include <cstdlib>
#include <cstring>
#include "client.h"
#include <stdio.h>
#include <filesystem>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include "boost\asio.hpp"
#include <boost/filesystem.hpp>

#define BUFLEN 65536
namespace bf = boost::filesystem;
using boost::asio::ip::tcp;

void send_file(boost::asio::basic_stream_socket<boost::asio::ip::tcp>& s, 
	std::string filePath, std::string sendPath, bool print, UserProgressBar* progBar);
void send_directory(boost::asio::basic_stream_socket<boost::asio::ip::tcp>& s, 
	std::string initialAbsolutePath, std::string folder, UserProgressBar* progBar);
std::string relative_path(std::string absolutePath, std::string initialAbsolutePath, std::string folder);
std::string takeFolder(std::string path);
std::string reverse_slash(std::string str);
size_t folder_size(std::string absolutePath);
void sendThreadTCPfile(utente& utenteProprietario, std::string username, 
	std::string initialAbsolutePath, UserProgressBar* progBar);


void sendImage(std::string filePath, std::string ipAddr) {
	//Invio la mia immagine di profilo al nuovo utente iscritto
	//Il protocollo che utilizzerò sarà
	//Invia +IM
	//Attendi +OK
	//Invia dim del file
	//Attendi +OK
	//Invia file
	//Chiudi tutto

	//Qua dovrà andare il path della mia immagine di default
	

	boost::asio::io_service io_service;
	tcp::resolver resolver(io_service);
	tcp::resolver::query query(tcp::v4(), ipAddr, "1400");
	tcp::resolver::iterator iterator = resolver.resolve(query);
	tcp::socket s(io_service);

	//Mi connetto al server, se qualcosa non va a buon fine, ritorno al main
	try {
		boost::asio::connect(s, iterator);
	}
	catch (std::exception e) {
		wxMessageBox("Non sono riuscito a connettermi con l'utente. Controllare che l'utente sia attivo", wxT("Errore"), wxOK | wxICON_ERROR);
		return;
	}


	//DA QUI
	try {
		std::ifstream file_in(filePath, std::ios::in | std::ios::binary);
		std::ifstream file_dim(filePath, std::ios::in | std::ios::binary);
		std::streampos begin, end_pos;
		std::string fileSize;
		std::ostringstream convert;
		char buf_response[256];
		std::string buf_send, response;
		size_t length;
		char c;
		double dim_write;
		double dim_send = 0;
		char buf_to_send[BUFLEN];

		//Calcolo la dimensione dell'immagine
		begin = file_dim.tellg();
		file_dim.seekg(0, std::ios::end);
		end_pos = file_dim.tellg();
		file_dim.close();
		long int size = (long)(end_pos - begin); //dim file

		convert << size;
		fileSize = convert.str();

		if (file_in.is_open())
		{
			//Invio +IM per dire che è un file immagine
			buf_send = "+IM";
			boost::asio::write(s, boost::asio::buffer(buf_send));
			//Vedo se il server mi ha detto che va ok
			length = s.read_some(boost::asio::buffer(buf_response, 256));
			buf_response[length] = '\0';
			response = buf_response;
			if (response != "+OK") {
				wxMessageBox("Il server ha dato risposta negativa per la ricezione dell'immagine.", wxT("Errore"), wxOK | wxICON_ERROR);
				return;
			}

			//Invio qui la dimensione del file
			boost::asio::write(s, boost::asio::buffer(fileSize));

			length = s.read_some(boost::asio::buffer(buf_response, 256));
			buf_response[length] = '\0';
			if (response != "+OK") {
				wxMessageBox("Il server ha dato risposta negativa per la ricezione dell'immagine.", wxT("Errore"), wxOK | wxICON_ERROR);
				return;
			}


			while (dim_send < size) {
				dim_write = 0;
				while (dim_write < BUFLEN && dim_send < size) {
					file_in.get(c);
					buf_to_send[(int)dim_write] = c;
					dim_write++;
					dim_send++;

				}
				boost::asio::write(s, boost::asio::buffer(buf_to_send, (int)dim_write));
			}
			length = s.read_some(boost::asio::buffer(buf_response, 256));
			buf_response[length] = '\0';
			if (response != "+OK") {
				wxMessageBox("Il server ha dato risposta negativa per la ricezione dell'immagine.", wxT("Errore"), wxOK | wxICON_ERROR);
				return;
			}
			file_in.close();
		}
		else {
			wxMessageBox("Errore nell'invio dell'immagine. Immagine inesistente.", wxT("Errore"), wxOK | wxICON_ERROR);
			return;
		}
	}
	catch (std::exception& e)
	{
		wxMessageBox(e.what(), wxT("Errore"), wxOK | wxICON_ERROR);
	}

	//A QUI
	s.close();
	io_service.stop();
}


//Questa è la funzione che vedrà il thread, a noi non importa.
void sendTCPfile(utente& utenteProprietario, std::string username, std::string initialAbsolutePath, UserProgressBar* progBar) {
	boost::thread sendTCPfileThread(sendThreadTCPfile, boost::ref(utenteProprietario), username, initialAbsolutePath, progBar);
	//sendTCPfileThread.join();
}


void sendThreadTCPfile(utente& utenteProprietario, std::string username, std::string initialAbsolutePath, UserProgressBar* progBar) {

	std::string ipAddr;       //Indirizzo ip dell'utente a cui inviare il file
	std::string folder;       //Nome della cartella che contiene il file


	//controllare se l'user esiste
	try {
		ipAddr = utenteProprietario.getUtente(username).getIpAddr(); //Se l'username non esiste, lancio un eccezione e torno, altrimenti ottengo il suo ip
	}
	catch (std::exception e) {
		wxMessageBox(e.what(), wxT("Errore"), wxOK | wxICON_ERROR);
		return;
	}
	
	//Se la directory/file specificata/o non è valida/o, ritorno al main
	if (!boost::filesystem::is_directory(initialAbsolutePath) && !boost::filesystem::is_regular_file(initialAbsolutePath)) {
		wxMessageBox("Path specificato non valido.", wxT("Errore"), wxOK | wxICON_ERROR);
		return;
	}

	//Ottengo il nome della cartella che contiene il file da inviare 
	folder = takeFolder(initialAbsolutePath);
	//Ne ottengo il path assoluto
	boost::filesystem::path path_absolutePath(initialAbsolutePath); 

	// Avvio la procedura di servizio boost, che servirà a mettersi in contatto con il server
	//La porta utilizzata in ambito TCP e' la 1400																		  
	boost::asio::io_service io_service;
	tcp::resolver resolver(io_service);
	tcp::resolver::query query(tcp::v4(), ipAddr, "1400");
	tcp::resolver::iterator iterator = resolver.resolve(query);
	tcp::socket s(io_service);

	//Mi connetto al server, se qualcosa non va a buon fine, ritorno al main
	try {
		boost::asio::connect(s, iterator);
	}
	catch (std::exception e) {
		wxMessageBox("Non sono riuscito a connettermi con l'utente. Controllare che l'utente sia attivo", wxT("Errore"), wxOK | wxICON_ERROR);
		return;
	}

	//Se sto inviando un file, chiamo send_file
	if (boost::filesystem::is_regular_file(path_absolutePath)) {
		send_file(s, initialAbsolutePath, folder, true, progBar);
	}
	//Senno chiamo send_directory
	else if (boost::filesystem::is_directory(path_absolutePath)) {
		send_directory(s, initialAbsolutePath, folder, progBar);

	}
	//Chiudo il socket e la procedurea di servizio
	s.close();
	io_service.stop();

	//segnalo la fine del trasferimento alla GUI
	wxThreadEvent event(wxEVT_THREAD, CLIENT_EVENT);
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
	
	char buf_recive[256];
	boost::posix_time::ptime start = boost::posix_time::second_clock::local_time();   //tempo in cui viene lanciata la funzione, utile per il calcolo del tempo residuo
	boost::posix_time::ptime end;   //Utile per il calcolo del tempo residuo
	long int dif = 0, min = 0, sec = 0;
	int length;

	//Dico che voglio inviare un direttorio
	buf_send = "+DR\0";
	boost::asio::write(s, boost::asio::buffer(buf_send));

	//Vedo se il server mi risponde con ok
	length = s.read_some(boost::asio::buffer(buf_recive, 256));
	buf_recive[length] = '\0';
	response = buf_recive;
	if (response != "+OK") {
		wxMessageBox("Errore nell'invio del direttorio.", wxT("Errore"), wxOK | wxICON_ERROR);
		return;
	}

	//Devo inviare qui la dimensione
	directory_size_to_send = folder_size(initialAbsolutePath); // dimensione directory
	progBar->SetMaxDir(directory_size_to_send); //<-- inizializzo barra progresso directory
	
	//Converto in string la dimensione cosi da poterla inviare al server.
	convert << directory_size_to_send;
	directorySize = convert.str();
	boost::asio::write(s, boost::asio::buffer(directorySize));
	length = s.read_some(boost::asio::buffer(buf_recive, 256));
	buf_recive[length] = '\0';
	response = buf_recive;
	if (response != "+OK") {
		wxMessageBox("Errore nell'invio del direttorio.", wxT("Errore"), wxOK | wxICON_ERROR);
		return;
	}


	//Attenzione:qua bisogna inviare il relativePath
	//Il concetto è questo: noi utilizziamo il path assoluto, ma a lui dobbiamo inviare quello relativo
	//Creo la cartella a partire dal path assoluto

	boost::asio::write(s, boost::asio::buffer(folder));
	length = s.read_some(boost::asio::buffer(buf_recive, 256));
	buf_recive[length] = '\0';
	response = buf_recive;
	if (response != "+OK") {
		wxMessageBox("Errore nell'invio del direttorio.", wxT("Errore"), wxOK | wxICON_ERROR);
		return;
	}

	for (bf::recursive_directory_iterator it(initialAbsolutePath); it != bf::recursive_directory_iterator(); ++it)
	{
		if (!bf::is_directory(*it)) {
			bf::path p(bf::absolute(*it));
			progBar->SetNewFile(p.filename().string()); //<-- inizializzo barra progresso file
			send_file(s, reverse_slash(bf::absolute(*it).string()), relative_path(bf::absolute(*it).string(), initialAbsolutePath, folder), true, progBar);

			directory_size_send += (size_t)bf::file_size(*it);

			if (directory_size_send > 0) {
				end = boost::posix_time::second_clock::local_time();
				dif = (end - start).total_seconds();
				min = (int)(((directory_size_to_send - directory_size_send) / ((directory_size_send)))*dif) / 60;
				sec = (int)(((directory_size_to_send - directory_size_send) / ((directory_size_send)))*dif) % 60;
			}


			int length = s.read_some(boost::asio::buffer(buf_recive, 256));
			buf_recive[length] = '\0';
			std::string  response(buf_recive);
			if (response != "+CN") {
				wxMessageBox("Errore nell'invio del file.", wxT("Errore"), wxOK | wxICON_ERROR);
			}

		}
		else {
			buf_send = "+DR\0";
			boost::asio::write(s, boost::asio::buffer(buf_send));
			int length = s.read_some(boost::asio::buffer(buf_recive, 256));
			buf_recive[length] = '\0';
			response = buf_recive;
			if (response != "+OK") {
				wxMessageBox("Errore nell'invio del direttorio.", wxT("Errore"), wxOK | wxICON_ERROR);
				return;
			}
			boost::asio::write(s, boost::asio::buffer(relative_path(bf::absolute(*it).string(), initialAbsolutePath, folder)));
			length = s.read_some(boost::asio::buffer(buf_recive, 256));
			buf_recive[length] = '\0';
			response = buf_recive;
			if (response != "+OK") {
				wxMessageBox("Errore nell'invio del direttorio.", wxT("Errore"), wxOK | wxICON_ERROR);
				return;
			}
		}
	}
	buf_send = "-END";
	boost::asio::write(s, boost::asio::buffer(buf_send));
	std::cout << std::endl;
}

void send_file(boost::asio::basic_stream_socket<boost::asio::ip::tcp>& s, 
	std::string filePath, std::string sendPath, bool print, UserProgressBar* progBar) {

	std::ifstream file_in(filePath, std::ios::in | std::ios::binary);
	std::ifstream file_dim(filePath, std::ios::in | std::ios::binary);
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
	char buf_to_send[BUFLEN], c; //char utili per caricare il pacchetto di lunghezza BUFLEN da inviare al server.
	char buf_recive[256]; //Buffer utile alla ricezione di un pacchetto. La funzione da me utilizzata prevede di ricevere un char di lunghezza fissa.
	size_t length; //Lunghezza della risposta ricevuta.

	try {
		
		 
		//Valuto la dimensione del file 
		std::streampos begin, end_pos;
		begin = file_dim.tellg();
		file_dim.seekg(0, std::ios::end);
		end_pos = file_dim.tellg();
		file_dim.close();
		//E la salvo su size
		size = (long)(end_pos - begin); //dim file
		
		progBar->SetMaxFile(size);  //<-- inizializzo barra di progresso

		//Converto la dimensione in una stringa cosi da poterla inviare al server
		convert << size;
		fileSize = convert.str();
		
		if (file_in.is_open())
		{
			//Qui inizia il protocollo di invio del file 
			//Invio +FL per dire che è un file
			buf_send = "+FL\0";
			boost::asio::write(s, boost::asio::buffer(buf_send));
			//Vedo se il server mi ha detto che va ok
			length = s.read_some(boost::asio::buffer(buf_recive, 256));
			buf_recive[length] = '\0';
			response = buf_recive;
			if (response != "+OK") {
				wxMessageBox("Il server ha dato risposta negativa per la ricezione del file.", wxT("Errore"), wxOK | wxICON_ERROR);
				return;
			}
			//invio qui il nome del file
			boost::asio::write(s, boost::asio::buffer(sendPath));
			//leggere un buffer e portarlo in una stringa

			length = s.read_some(boost::asio::buffer(buf_recive, 256));
			buf_recive[length] = '\0';
			response.clear();
			response = buf_recive;
			if (response != "+OK") {
				wxMessageBox("Il server ha dato risposta negativa per la ricezione del file.", wxT("Errore"), wxOK | wxICON_ERROR);
				return;
			}

			//Invio qui la dimensione del file
			boost::asio::write(s, boost::asio::buffer(fileSize));

			length = s.read_some(boost::asio::buffer(buf_recive, 256));
			buf_recive[length] = '\0';
			if (response != "+OK") {
				wxMessageBox("Il server ha dato risposta negativa per la ricezione del file.", wxT("Errore"), wxOK | wxICON_ERROR);
				return;
			}

			

			//Carico il buffer da caricare 
			while (dim_send < size) {
				dim_write = 0;
				while (dim_write < BUFLEN && dim_send < size) {
					file_in.get(c);
					buf_to_send[(int)dim_write] = c;
					dim_write++;
					dim_send++;
				}
				//Invio il buffer al server, ricordare di mettere un controllo qui
				boost::asio::write(s, boost::asio::buffer(buf_to_send, (int)dim_write));
				if (calcola_tempo % 50 == 0)
				{
					end = boost::posix_time::second_clock::local_time();
					dif = (end - start).total_seconds();
					min = (int)(((size - dim_send) / ((dim_send)))*dif) / 60;
					sec = (int)((((size - dim_send) / ((dim_send)))*dif)) % 60;
				}
				calcola_tempo++;	
				progBar->IncFile(dim_send);
				progBar->SetTimeFile(min, sec);
				
			}
			file_in.close();
		}
		else {
			wxMessageBox("File non aperto correttamente.", wxT("Errore"), wxOK | wxICON_ERROR);
			return;
		}
	}
	catch (std::exception& e)
	{
		wxMessageBox(e.what(), wxT("Errore"), wxOK | wxICON_ERROR);
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


std::string takeFolder(std::string path) {
	int i, lengthPath, start;
	char *buf = (char *)malloc((path.length() + 1) * sizeof(char));
	std::string folderName;

	lengthPath = path.length();
	for (i = lengthPath - 2; i >= 0 && path[i] != '\\'; i--);
	if (i == -1) {
		folderName = path;
		free(buf);
		return folderName;
	}
	int end = 0;
	if (path[lengthPath - 1] == '\\')
		end = 1;
	start = i + 1;
	for (i = 0; i < lengthPath - start - end; i++)
		buf[i] = path[i + start];
	buf[i] = '\0';

	folderName = buf;
	free(buf);
	return folderName;
}

std::string reverse_slash(std::string str) {
	for (unsigned int i = 0; i < str.length(); i++)
		if (str[i] == '\\')
			str[i] = '/';

	return str;
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