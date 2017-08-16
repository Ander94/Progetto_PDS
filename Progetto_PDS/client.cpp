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

//PROVA NUOVO PC
//TEST MAC 
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

//Questa è la funzione che vedrà il thread, a noi non importa.
void sendTCPfile(utente& utenteProprietario, std::string username, std::string initialAbsolutePath, UserProgressBar* progBar) {
	boost::thread sendTCPfileThread(sendThreadTCPfile, boost::ref(utenteProprietario), username, initialAbsolutePath, progBar);
	//sendTCPfileThread.join();
}

void sendThreadTCPfile(utente& utenteProprietario, std::string username, std::string initialAbsolutePath, UserProgressBar* progBar) {

	std::string relativePath;  //Path relativo della cartella
	std::string path_to_send;  //Il path da inviare al server
	std::string ipAddr;       //Indirizzo ip dell'utente a cui inviare il file
	std::string folder;


	//controllare se l'user esiste
	try {
		utenteProprietario.getUtente(username);  //Se l'username non esiste, lancio un eccezione e torno
	}
	catch (std::exception e) {
		std::cerr << e.what() << std::endl;
		return;
	}
	//Se l'username esiste, ottendo il suo ip
	ipAddr = utenteProprietario.getUtente(username).getIpAddr();

	//Se la directory specificata non è valida, ritorno al main
	if (!boost::filesystem::is_directory(initialAbsolutePath) && !boost::filesystem::is_regular_file(initialAbsolutePath)) {
		std::cout << "Attenzione: la directory specificata non e' valida." << std::endl;
		return;
	}

	//Ottengo il nome della cartella
	folder = takeFolder(initialAbsolutePath);
	boost::filesystem::path path_absolutePath(initialAbsolutePath);  //Ne ottengo il path assoluto
	relativePath = boost::filesystem::relative(path_absolutePath).string();   //E relativo

																			  //Avvio la procedura di servizio boost, che servirà a mettersi in contatto con il server
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
		std::cerr << "Non sono riuscito a connettermi con l'utente. Controllare che l'utente sia attivo" << std::endl;
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
	char buf[256];
	std::string buf_send;
	boost::posix_time::ptime start = boost::posix_time::second_clock::local_time();   //tempo in cui viene lanciata la funzione, utile per il calcolo del tempo residuo
	boost::posix_time::ptime end;   //Utile per il calcolo del tempo residuo
	long int dif = 0;                //Utile per il calcolo del tempo residuo, al fine di calcolare end-start
	long int min = 0, sec = 0;

	//Dico che voglio inviare un direttorio
	buf_send = "+DR\0";
	boost::asio::write(s, boost::asio::buffer(buf_send));

	//Vedo se il server mi risponde con ok
	int length = s.read_some(boost::asio::buffer(buf, 256));
	buf[length] = '\0';
	std::string response(buf);
	if (response != "+OK") {
		printf("Errore nella ricezione del direttorio.\n");
		return;
	}

	//Devo inviare qui la dimensione
	size_t directory_size_to_send = folder_size(initialAbsolutePath); // dimensione directory
	progBar->SetMaxDir(directory_size_to_send); //<-- inizializzo barra progresso directory
	std::string directorySize;
	std::ostringstream convert;
	convert << directory_size_to_send;
	directorySize = convert.str();
	boost::asio::write(s, boost::asio::buffer(directorySize));
	length = s.read_some(boost::asio::buffer(buf, 256));
	buf[length] = '\0';
	response = buf;
	if (response != "+OK") {
		printf("Errore nella ricezione del direttorio.\n");
		return;
	}


	//Attenzione:qua bisogna inviare il relativePath
	//Il concetto è questo: noi utilizziamo il path assoluto, ma a lui dobbiamo inviare quello relativo
	//Creo la cartella a partire dal path assoluto

	boost::asio::write(s, boost::asio::buffer(folder));
	length = s.read_some(boost::asio::buffer(buf, 256));
	buf[length] = '\0';
	response = buf;
	if (response != "+OK") {
		printf("Errore nella ricezione del direttorio.\n");
		return;
	}

	//std::cout << "the folder is " << folder << std::endl;
	//std::cout << "The absPath is " << absolutePath << std::endl;

	size_t directory_size_send = 0;
	min = 0; sec = 0;
	for (bf::recursive_directory_iterator it(initialAbsolutePath); it != bf::recursive_directory_iterator(); ++it)
	{
		if (!bf::is_directory(*it)) {
			//size += bf::file_size(*it);
			//std::cout << "File " << relative_path(bf::relative(*it).string(), folder) << std::endl;
			
			//aggiornare qui nome file da inviare
			//bf::absolute(*it).string() -> path assoluto
			//bf::relative(*it).string()
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
			//printf("Inviato %d perc, Tempo rimanente: %d min %d sec \r", (int)(((float)directory_size_send / (float)directory_size_to_send) * 100), min, sec);


			int length = s.read_some(boost::asio::buffer(buf, 256));
			buf[length] = '\0';
			std::string  response(buf);
			if (response != "+CN") {
				printf("Errore nell'invio del file\n");
			}

		}
		else {
			//std::cout << "Directory " << relative_path(bf::absolute(*it).string(), initialAbsolutePath,folder) << std::endl;

			// *****Da mettere in funzione separata ****
			std::string buf_send;
			//Dico che voglio inviare un direttorio
			buf_send = "+DR\0";
			//std::cout << "Client: Invio " << buf_send << std::endl;
			boost::asio::write(s, boost::asio::buffer(buf_send));
			//Vedo se il server mi risponde con ok
			int length = s.read_some(boost::asio::buffer(buf, 256));
			buf[length] = '\0';
			std::string response(buf);

			//std::cout << "Client: ricevo " << response << std::endl;
			if (response != "+OK") {
				printf("Errore nella ricezione del direttorio.\n");
				return;
			}
			boost::asio::write(s, boost::asio::buffer(relative_path(bf::absolute(*it).string(), initialAbsolutePath, folder)));
			//std::cout << "Client: Invio " << filePath << std::endl;
			//Vedo se il server mi risponde con ok
			length = s.read_some(boost::asio::buffer(buf, 256));
			buf[length] = '\0';
			response = buf;
			if (response != "+OK") {
				printf("Errore nella ricezione del direttorio.\n");
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

	//std::cout << "**Client: cerco di inviare il file " << filePath << std::endl;

	try {
		std::ifstream file_in(filePath, std::ios::in | std::ios::binary);
		std::ifstream file_dim(filePath, std::ios::in | std::ios::binary);
		//std::string buf;
		std::streampos begin, end_pos;
		begin = file_dim.tellg();
		file_dim.seekg(0, std::ios::end);
		end_pos = file_dim.tellg();
		file_dim.close();
		long int size = (long)(end_pos - begin); //dim file
		
		progBar->SetMaxFile(size);  //<-- inizializzo barra di progresso

		std::string fileSize;
		std::ostringstream convert;

		convert << size;
		fileSize = convert.str();
		char buf[256];
		std::string buf_send;
		size_t length;
		if (file_in.is_open())
		{
			//Invio +FL per dire che è un file
			buf_send = "+FL\0";
			boost::asio::write(s, boost::asio::buffer(buf_send));
			//Vedo se il server mi ha detto che va ok
			length = s.read_some(boost::asio::buffer(buf, 256));
			buf[length] = '\0';
			std::string response(buf);
			if (response != "+OK") {
				std::cerr << "Il server ha dato risposta negativa per la ricezione del file." << std::endl;
				return;
			}
			//invio qui il nome del file
			boost::asio::write(s, boost::asio::buffer(sendPath));
			//leggere un buffer e portarlo in una stringa

			length = s.read_some(boost::asio::buffer(buf, 256));
			buf[length] = '\0';
			response.clear();
			response = buf;
			if (response != "+OK") {
				std::cerr << "Il server ha dato risposta negativa per la ricezione del file." << std::endl;
				return;
			}
			//Invio qui la dimensione del file
			boost::asio::write(s, boost::asio::buffer(fileSize));

			length = s.read_some(boost::asio::buffer(buf, 256));
			buf[length] = '\0';
			if (response != "+OK") {
				std::cerr << "Il server ha dato risposta negativa per la ricezione del file." << std::endl;
				return;
			}

			char c;
			boost::posix_time::ptime start = boost::posix_time::second_clock::local_time();
			boost::posix_time::ptime end;
			long int dif = 0;
			long int min = 0, sec = 0;
			double dim_write;
			double dim_send = 0;
			char buf_to_send[BUFLEN];
			int calcola_tempo = 1;
			while (dim_send < size) {
				dim_write = 0;
				while (dim_write < BUFLEN && dim_send < size) {
					file_in.get(c);
					buf_to_send[(int)dim_write] = c;
					dim_write++;
					dim_send++;
				}
				boost::asio::write(s, boost::asio::buffer(buf_to_send, (int)dim_write));
				if (calcola_tempo % 50 == 0)
				{
					end = boost::posix_time::second_clock::local_time();
					dif = (end - start).total_seconds();
					min = (int)(((size - dim_send) / ((dim_send)))*dif) / 60;
					sec = (int)((((size - dim_send) / ((dim_send)))*dif)) % 60;
				}
				calcola_tempo++;
				if (print == true) { //aggiongere aggiornamento qui
					//dim_send->byte inviati
					//min->minuti rimanenti
					//sec->secondi
					
					progBar->IncFile(dim_send);
					progBar->SetTimeFile(min, sec);

					//printf("Inviato %d perc, Tempo rimanente: %d min %d sec \r", (int)(((float)dim_send / (float)size) * 100), min, sec);
				}
			}
			if (print == true)
				std::cout << std::endl;
			file_in.close();
		}
		else {
			std::cerr << "File inesistente." << std::endl;
			return;
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
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