#include <fstream>
#include <cstdlib>
#include <cstring>
#include "server.h"
#include "boost/asio.hpp"
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>

#include <windows.h>

#define BUFLEN 65536
using boost::asio::ip::tcp;
void recive_file(boost::asio::basic_stream_socket<boost::asio::ip::tcp>& s, std::string fileName, bool print);
void service(boost::asio::basic_stream_socket<boost::asio::ip::tcp>& s, utente utenteProprietario, std::string generalPath, MainFrame* mainframe);

std::wstring s2ws(const std::string& s);
void reciveTCPfile(utente& utenteProprietario, std::string generalPath , MainFrame* mainframe) {


	try {
		//Dichiaro le strutture boost necessarie
		boost::asio::io_service io_service;
		tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), 1400));
		tcp::socket s(io_service);
		//Dichiaro le variabili necessarie

		//Accetto richieste finchè non viene chiuso il main
		for (;;)
		{
			//Accetto una nuova richesta
			a.accept(s);
			//mainframe->showBal("Ricezione!", "Ricezione di un nuovo file");
			//service(s, utenteProprietario, generalPath);
			//Chiamo service qui
			reciveAfterAccept(s, utenteProprietario, generalPath, mainframe);
		}
		io_service.stop();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
	return;
}

void reciveAfterAccept(boost::asio::basic_stream_socket<boost::asio::ip::tcp>& s, utente& utenteProprietario, std::string generalPath, MainFrame* mainframe) {
	service(s, utenteProprietario, generalPath,  mainframe);
	//boost::thread(service, boost::ref(s), utenteProprietario, generalPath);
}

void service(boost::asio::basic_stream_socket<boost::asio::ip::tcp>& s, utente utenteProprietario, std::string generalPath, MainFrame* mainframe) {
	int length;
	bool first_directory = true;
	//unsigned int i;
	char buf[256];
	std::string ipAddrRemote;
	std::string query;
	std::string response;
	std::string fileName;
	ipAddrRemote = s.remote_endpoint().address().to_string();

	//Questo primo pacchetto serve a vedere se sto ricevendo un file o una directory
	length = s.read_some(boost::asio::buffer(buf, 256));
	buf[length] = '\0';
	query = buf;
	if (query == "+FL") {
		//Qui ricevo un file
		//Se tutto va bene invio una risposta positiva
		response = "+OK\0";
		boost::asio::write(s, boost::asio::buffer(response));
		//La risposta conterrà il pathName
		length = s.read_some(boost::asio::buffer(buf, 256));
		buf[length] = '\0';
		fileName = buf;
		//Saranno da levare tutti questi download, servono per il momento a ricever in loopback
		//Creo la directory per ricevere il file
		CreateDirectory(L"./download/", NULL);
		//Ricevo il file
		mainframe->showBal("Ricezione file", fileName +"\nDa " + utenteProprietario.getUsernameFromIp(ipAddrRemote));
		recive_file(s, "./download/" + fileName, true);

		std::cout << "Ho ricevuto il file " << fileName << " da " << utenteProprietario.getUsernameFromIp(ipAddrRemote) << std::endl;
	}

	if (query == "+IM") {
		//Qui ricevo un file
		//Se tutto va bene invio una risposta positiva
		response = "+OK\0";
		boost::asio::write(s, boost::asio::buffer(response));

		//Ricevo il file

		recive_file(s, generalPath + "local_image\\" + ipAddrRemote + ".png", false);
		response = "+OK\0";
		boost::asio::write(s, boost::asio::buffer(response));
	}

	if (query == "+DR") {
		//**
		//Saranno da levare tutti questi download, servono per il momento a ricever in loopback
		//Verranno sostituiti con il path assoluto
		CreateDirectory(L"./download/", NULL);
		boost::posix_time::ptime start = boost::posix_time::second_clock::local_time();
		boost::posix_time::ptime end;
		int dif, min = 0, sec = 0;
		size_t directory_size_to_send, directorySize, directory_size_send = 0;
		//Finche non ricevo -END, vuol dire che ho o una directory o un file da ricevere
		bool firstTime = true;
		while (query != "-END") {

			if (query == "+DR") {
				//Rispondo ok 
				std::string response("+OK");
				boost::asio::write(s, boost::asio::buffer(response));

				if (firstTime == true) {
					length = s.read_some(boost::asio::buffer(buf, 256));
					buf[length] = '\0';
					directorySize = std::atoi(buf);
					directory_size_to_send = directorySize;
					//Comunico al server che può inviare il file
					response = "+OK";
					boost::asio::write(s, boost::asio::buffer(response));
					firstTime = false;
				}

				//Aspetto il nome del path
				length = s.read_some(boost::asio::buffer(buf, 256));
				buf[length] = '\0';
				fileName = buf;
				//**
				if (first_directory==true) {
					mainframe->showBal("Ricezione Directory", fileName + "\nDa " + utenteProprietario.getUsernameFromIp(ipAddrRemote));
					first_directory = false;
				}
				std::string pathName("./download/" + fileName);
				//Rispondo ok se riesco ad ivniare il path
				//	std::cout << "**Server: Ricevo " << pathName << std::endl;
				response = "+OK\0";
				boost::asio::write(s, boost::asio::buffer(response));
				//Coverto il path name in un char, perchè la funzione createDirectory vuole un char e non string
				/*char *pathNameChar = (char*)malloc((pathName.length() + 1) * sizeof(char));
				for (i = 0; i < pathName.length(); i++) {
				pathNameChar[i] = pathName[i];
				}
				pathNameChar[pathName.length()] = '\0';
				//Creo la directory ricevuta
				//	printf("**Server: Creo la directory %s\n", pathNameChar);
				*/
				std::wstring stemp = s2ws(pathName);
				LPCWSTR result = stemp.c_str();

				CreateDirectory(result, NULL);
				//Leggo la prossima Query, ovvero leggo se sarà un File o un altra Directory
				length = s.read_some(boost::asio::buffer(buf, 256));
				buf[length] = '\0';
				query = buf;
			}
			if (query == "+FL") {
				response = "+OK\0";
				boost::asio::write(s, boost::asio::buffer(response));
				//Leggo il path del file
				length = s.read_some(boost::asio::buffer(buf, 256));
				buf[length] = '\0';
				fileName = buf;
				std::string fileName("./download/" + fileName);
				recive_file(s, fileName, false);
				//Vedo la dim del file che ho ricevuto
				directory_size_send += (size_t)boost::filesystem::file_size(fileName);
				//std::cout << "Server: ricevuto " << fileName << " di dim " << boost::filesystem::file_size(fileName) << std::endl;
				//Invio la risposta +CN per dire al client che ho ricevuto il tutto, e può continuare ad inviare.
				response = "+CN\0";
				boost::asio::write(s, boost::asio::buffer(response));
				//Leggo la prossima Query, ovvero leggo se sarà un File o un altra Directory
				length = s.read_some(boost::asio::buffer(buf, 256));
				buf[length] = '\0';
				query = buf;
				//Devo fare la stampa della progressione qui
				if (directory_size_send > 0) {
					end = boost::posix_time::second_clock::local_time();
					dif = (end - start).total_seconds();
					min = (int)(((directory_size_to_send - directory_size_send) / ((directory_size_send)))*dif) / 60;
					sec = (int)(((directory_size_to_send - directory_size_send) / ((directory_size_send)))*dif) % 60;
				}
			}
			printf("Ricevuto %d perc, Tempo rimanente: %d min %d sec \r", (int)(((float)directory_size_send / (float)directory_size_to_send) * 100), min, sec);


		}
		std::cout << std::endl;
		std::cout << "Ho ricevuto il direttorio " << std::endl;
	}
	s.close();

}

std::wstring s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}



void recive_file(boost::asio::basic_stream_socket<boost::asio::ip::tcp>& s, std::string fileName, bool print) {

	std::ofstream file_out(fileName, std::ios::out | std::ios::binary);
	//Attenzione: quando si inviano i file devo mettere questo!
	//std::ofstream file_out(".//scaricato//" + fileName + "", std::ios::out | std::ios::binary);
	std::string response;
	boost::system::error_code error;
	char buf[BUFLEN];
	int length, i;
	long int size;
	long int dif = 0, min = 0, sec = 0;
	long int calcola_tempo = 0;
	char buf_recive[BUFLEN];  //serve a memorizzare i pacchetti TCP
	double dim_recived = 0, dim_read;

	try
	{
		if (file_out.is_open()) {
			//Dico che posso ricevere il file con +OK\0
			response = "+OK\0";
			boost::asio::write(s, boost::asio::buffer(response));
			//Leggo la dimensione del file che ricevero, e la salvo su size
			length = s.read_some(boost::asio::buffer(buf, 256));
			buf[length] = '\0';
			size = std::atoi(buf);
			//Comunico al server che può inviare il file
			response = "+OK";
			boost::asio::write(s, boost::asio::buffer(response));

			boost::posix_time::ptime start = boost::posix_time::second_clock::local_time();
			boost::posix_time::ptime end;

			//ricevo finchè non ho ricevuto tutto il file
			while (dim_recived<size)
			{
				dim_read = s.read_some(boost::asio::buffer(buf_recive, BUFLEN), error);
				if (error == boost::asio::error::eof)
					break;
				else if (error)
					throw boost::system::system_error(error);
				//Scarico tutto il buffer nel file
				for (i = 0; i<dim_read; i++)
					file_out << buf_recive[i];
				dim_recived += dim_read;

				//Faccio una valutazione, ogni 50 pacchetti ricevuti, del tempo rimanente
				if (calcola_tempo % 50 == 0)
				{
					end = boost::posix_time::second_clock::local_time();
					dif = (end - start).total_seconds();
					min = (long int)(((size - dim_recived) / ((dim_recived)))*dif) / 60;
					sec = (long int)((((size - dim_recived) / ((dim_recived)))*dif)) % 60;
				}
				calcola_tempo++;
				if (print == true)
					printf("Ricevuto %d perc, Tempo rimanente: %d min %d sec \r", (int)(((float)dim_recived / (float)size) * 100), min, sec);
			}
			if (print == true)
				std::cout << std::endl;
			file_out.close();
		}
		else
		{
			std::cout << "Server: errore nell'apertura del file " << fileName << std::endl;
			//Se ho avuto qualche errore nella ricezione del file, invio -ERR
			response = "-ERR";
			boost::asio::write(s, boost::asio::buffer(response));
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception in thread: " << e.what() << "\n";
	}
}
