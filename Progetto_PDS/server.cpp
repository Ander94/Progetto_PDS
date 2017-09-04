#include <fstream>
#include <cstdlib>
#include <cstring>
#include "server.h"
#include "boost/asio.hpp"
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>

#include <windows.h>


using boost::asio::ip::tcp;
void reciveAfterAccept(tcp::socket s, utente utenteProprietario, std::string generalPath, MainFrame* mainframe);
void recive_file(boost::asio::basic_stream_socket<boost::asio::ip::tcp>& s, std::string fileName);
void StartAccept(boost::asio::ip::tcp::acceptor& acceptor, utente& utenteProprietario, std::string generalPath, MainFrame* mainframe);
void HandleAccept(const boost::system::error_code& error, boost::shared_ptr< boost::asio::ip::tcp::socket > socket, boost::asio::ip::tcp::acceptor& acceptor
	, utente& utenteProprietario, std::string generalPath, MainFrame* mainframe);

void StartAccept(boost::asio::ip::tcp::acceptor& acceptor, utente& utenteProprietario, std::string generalPath, MainFrame* mainframe) {
	boost::shared_ptr< tcp::socket > socket(new tcp::socket(acceptor.get_io_service()));

	acceptor.async_accept(*socket, boost::bind(HandleAccept, boost::asio::placeholders::error, socket, boost::ref(acceptor), boost::ref(utenteProprietario), generalPath, mainframe));

}

void HandleAccept(const boost::system::error_code& error, boost::shared_ptr< boost::asio::ip::tcp::socket > socket, boost::asio::ip::tcp::acceptor& acceptor
 , utente& utenteProprietario, std::string generalPath, MainFrame* mainframe)
{
	// If there was an error, then do not add any more jobs to the service.
	if (error)
	{
		wxMessageBox("Errore nell'accettazione della connessione: " + error.message(), wxT("Errore"), wxOK | wxICON_ERROR);
		return;
	}

	// Otherwise, the socket is good to use.
	// Perform async operations on the socket.
	std::thread(reciveAfterAccept, std::move( *socket), utenteProprietario, generalPath, mainframe).detach();


	// Done using the socket, so start accepting another connection.  This
	// will add a job to the service, preventing io_service::run() from
	// returning.
	StartAccept(acceptor, utenteProprietario, generalPath, mainframe);
};

void reciveTCPfile(utente& utenteProprietario, std::string generalPath , MainFrame* mainframe, boost::asio::io_service& io_service) {

	tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 1400));

	StartAccept(acceptor, utenteProprietario, generalPath, mainframe);

	// Process event loop.
	io_service.run();
}


void reciveAfterAccept(tcp::socket s, utente utenteProprietario, std::string generalPath, MainFrame* mainframe) {
	int length;
	bool first_directory = true;
	char buf[256];
	std::string ipAddrRemote;
	std::string query;
	std::string response;
	std::string fileName;
	Settings *m_settings = mainframe->GetSettings(); //Ricordare di liberare

	try {
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
			std::string savePath = mainframe->GetSettings()->getSavePath() + "\\" + fileName;
			std::string extension = boost::filesystem::extension(fileName);

			if (m_settings->getAutoSaved() == 1) {
				wxMessageDialog *dial = new wxMessageDialog(NULL, wxT("Accettare il file \"" + fileName + "\" da " + utenteProprietario.getUsernameFromIp(ipAddrRemote) + "?"), wxT("INFO"), wxYES_NO | wxHELP | wxICON_QUESTION);
				dial->SetHelpLabel(wxID_SAVEAS);
				int ret_val = dial->ShowModal();
				if (ret_val == wxID_NO) {
					response = "-ERR";
					boost::asio::write(s, boost::asio::buffer(response));
					s.close();
					return;
				}
				else if (ret_val == wxID_HELP) {
					wxFileDialog saveFileDialog(NULL, "Salva " + fileName + " come", mainframe->GetSettings()->getSavePath(), fileName,
							"(*" + extension + ")|*" + extension, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
					if (saveFileDialog.ShowModal() == wxID_CANCEL)
						return;     // the user changed idea...
					savePath = saveFileDialog.GetPath().ToStdString();
				}
			}

			//Inserire qui richesta recezione
			if (boost::filesystem::is_regular_file(savePath)) {
				//Richiedere se salvare o meno poichè vuol dire che il file già esiste
				wxMessageDialog *dial = new wxMessageDialog(NULL, wxT("Il file " + fileName + " già esiste. Sovrascriverlo?"), wxT("INFO"), wxYES_NO | wxICON_QUESTION);
				if (dial->ShowModal() == wxID_YES) {
					recive_file(s, mainframe->GetSettings()->getSavePath() + "\\" + fileName);
					mainframe->showBal("Ricezione file", fileName + "\nDa " + utenteProprietario.getUsernameFromIp(ipAddrRemote));
				}
				else {
					//Si rifiuta cosi la ricezione
					response = "-ERR\0";
					boost::asio::write(s, boost::asio::buffer(response));
				}

			}
			else {
				recive_file(s, savePath);
				mainframe->showBal("Ricezione file", fileName + "\nDa " + utenteProprietario.getUsernameFromIp(ipAddrRemote));
			}
		}

		if (query == "+IM") {
			//Qui ricevo l'immagine di profilo
			//Se tutto va bene invio una risposta positiva
			response = "+OK\0";
			boost::asio::write(s, boost::asio::buffer(response));

			//Ricevo il file immagine, che salverò con il nome dell'ip dell'utente cosi da essere univoco
			recive_file(s, generalPath + "local_image\\" + ipAddrRemote + ".png");
			response = "+OK\0";
			boost::asio::write(s, boost::asio::buffer(response));
		}

		if (query == "+DR") {
			boost::posix_time::ptime start = boost::posix_time::second_clock::local_time();
			boost::posix_time::ptime end;
			std::string response;
			int dif, min = 0, sec = 0;
			size_t directory_size_to_send, directorySize, directory_size_send = 0;
			//Finche non ricevo -END, vuol dire che ho o una directory o un file da ricevere
			bool firstTime = true;
			
			while (query != "-END") {
				if (query == "+DR") {
					//Rispondo ok 
					response = "+OK";
					boost::asio::write(s, boost::asio::buffer(response));

					//Se è il "primo giro", vuol dire che ricevo le informazioni utili alla ricezione della directory
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
					if (first_directory == true) {
						//Qui per prendere decisione sulla directory
						if (m_settings->getAutoSaved() == 1) {
							wxMessageDialog *dial = new wxMessageDialog(NULL, wxT("Accettare la directory " + fileName + " da " + utenteProprietario.getUsernameFromIp(ipAddrRemote) + "?"), wxT("INFO"), wxYES_NO | wxICON_QUESTION);
							if (dial->ShowModal() == wxID_NO) {
								response = "-ERR";
								boost::asio::write(s, boost::asio::buffer(response));
								s.close();
								return;
								//Bisogna rispondere in modo negativo
							}
							//Bisogna fare richiesta ed eventualmente annullare l'invio
						}

						if (boost::filesystem::is_directory(mainframe->GetSettings()->getSavePath() + "\\" + fileName)) {
							//Richiedere se salvare o meno
							wxMessageDialog *dial = new wxMessageDialog(NULL, wxT("La directory " + fileName + " già esiste. Sovrascriverla?"), wxT("INFO"), wxYES_NO | wxICON_QUESTION);
							if (dial->ShowModal() == wxID_YES) {
								mainframe->showBal("Ricezione Directory", fileName + "\nDa " + utenteProprietario.getUsernameFromIp(ipAddrRemote));
							}
							else {
								response = "-ERR\0";
								boost::asio::write(s, boost::asio::buffer(response));
								s.close();
								return;
							}

						}
						else {
							mainframe->showBal("Ricezione Directory", fileName + "\nDa " + utenteProprietario.getUsernameFromIp(ipAddrRemote));
						}
						first_directory = false;
					}
					//Cambiare qui
					std::string pathName(mainframe->GetSettings()->getSavePath() + "\\" + fileName);
					//Rispondo ok se riescxo ad ivniare il path
					response = "+OK\0";
					boost::asio::write(s, boost::asio::buffer(response));
					boost::filesystem::create_directory(pathName);

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
					//Cambiare qui
					std::string fileName(mainframe->GetSettings()->getSavePath() + "\\" + fileName);
					recive_file(s, fileName);
					//Vedo la dim del file che ho ricevuto
					directory_size_send += (size_t)boost::filesystem::file_size(fileName);
					//Invio la risposta +CN per dire al client che ho ricevuto il tutto, e può continuare ad inviare.
					response = "+CN";
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
			}
		}
	}
	catch (std::exception& e) {
		s.close();
		wxMessageBox(e.what(), wxT("Errore"), wxOK | wxICON_ERROR);
		return;
	}
	s.close();
}

void recive_file(boost::asio::basic_stream_socket<boost::asio::ip::tcp>& s, std::string fileName) {

	std::ofstream file_out(fileName, std::ios::out | std::ios::binary);
	std::string response;
	boost::system::error_code error;
	char buf[BUFLEN];
	int length, i;
	long int size;
	long int dif = 0, min = 0, sec = 0;
	long int calcola_tempo = 0;
	char buf_recive[BUFLEN]; 
	double dim_recived = 0, dim_read;
	boost::posix_time::ptime start, end;

	try
	{
		if (file_out.is_open()) {
			//Se tutto va bene, dico che posso ricevere il file con +OK\0
			response = "+OK";
			boost::asio::write(s, boost::asio::buffer(response));
			//Leggo la dimensione del file che ricevero, e la salvo su size
			length = s.read_some(boost::asio::buffer(buf, 256));
			buf[length] = '\0';
			size = std::atoi(buf);
			//Comunico al server che può inviare il file
			response = "+OK";
			boost::asio::write(s, boost::asio::buffer(response));

			start = boost::posix_time::second_clock::local_time();

			//ricevo finchè non ho ricevuto tutto il file
			while (dim_recived<size)
			{
				dim_read = s.read_some(boost::asio::buffer(buf_recive, BUFLEN));
			/*	if (error == boost::asio::error::eof)
					break;
				else if (error) {
					file_out.close();
					return throw std::invalid_argument("L'utente ha interrotto l'invio del file.");
				}*/
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
			}
			file_out.close();
		}
		else
		{
			//Se ho avuto qualche errore nell'apertura del file, invio -ERR
			response = "-ERR";
			boost::asio::write(s, boost::asio::buffer(response));
			return throw std::invalid_argument("Errore nell'apertura del file.");
		}
	}
	catch (std::exception&)
	{
		file_out.close();
		return throw std::invalid_argument("L'utente ha interrotto l'invio del file.");
	}
}
