//COMMENTATO TUTTO

#include "server.h"

using boost::asio::ip::tcp;

/********************************************************************************
Funzione che viene chiamata dopo aver accettato la richesta di connessione da parte del client
Riceve come parametri:
-s: socket su cui vieme scambiato il file
-utenteProprietario: riferimento a tutti gli utenti connessi all'applicazione.
-generalPath: path dove salvare il file ricevuto
-settings: riferimento utile per la grafica
**********************************************************************************/
void reciveAfterAccept(boost::asio::io_service& io_service, tcp::socket s, utente& utenteProprietario, std::string generalPath, Settings* settings);

/********************************************************************************
Funzione che salva un file scambiato sul socket s.
Riceve come parametri:
-s: socket su cui vieme scambiato il file
-fileName: nome del file da salvare
**********************************************************************************/
//LEO: void recive_file(boost::asio::io_service& io_service, boost::asio::basic_stream_socket<boost::asio::ip::tcp>& s, std::string fileName, FileInDownload* fp);
void recive_file(boost::asio::io_service& io_service, boost::asio::basic_stream_socket<boost::asio::ip::tcp>& s, std::string fileName);

/********************************************************************************
StartAccept inizializza il socket e lancia l'accettazione asincrona di richieste da parte del client.
Riceve come parametri:
-a: acceptor utile per gestire l'accettazione di nuove richieste
-utenteProprietario: riferimento a tutti gli utenti connessi all'applicazione.
-generalPath: path dove salvare il file ricevuto
-settings: riferimento utile per la grafica
**********************************************************************************/
void StartAccept(boost::asio::io_service& io_service, boost::asio::ip::tcp::acceptor& acceptor, utente& utenteProprietario, std::string generalPath, Settings* settings);

/********************************************************************************
HandleAccept gestisce l'accettazione di nuove richieste, gestendo eventuali errori
Riceve come parametri:
-error: parametro utile per la gestione degli errori
-socket: socket su cui vieme scambiato il file
-acceptor: acceptor utile per gestire l'accettazione di nuove richieste
-utenteProprietario: riferimento a tutti gli utenti connessi all'applicazione.
-generalPath: path dove salvare il file ricevuto
-settings: riferimento utile per la grafica
**********************************************************************************/
void HandleAccept(const boost::system::error_code& error, boost::asio::io_service& io_service,boost::shared_ptr< boost::asio::ip::tcp::socket > socket, boost::asio::ip::tcp::acceptor& acceptor
	, utente& utenteProprietario, std::string generalPath, Settings* settings);

/********************************************************************************
getNumberDownload torna il numero di download in caso di conflitto di nome
Riceve come parametri:
-path: path assoluto della posizione del file/directory
-fileName: nome del file/directory
**********************************************************************************/
int getNumberDownload(std::string& path, std::string& fileName);

void StartAccept(boost::asio::io_service& io_service, boost::asio::ip::tcp::acceptor& acceptor, utente& utenteProprietario, std::string generalPath, Settings* settings) {
	//Inizializzo il socket
	boost::shared_ptr< tcp::socket > socket(new tcp::socket(acceptor.get_io_service()));
	//Lancio l'accettazione asincrona di richieste
	acceptor.async_accept(*socket, boost::bind(HandleAccept, boost::asio::placeholders::error, boost::ref(io_service), socket, boost::ref(acceptor), boost::ref(utenteProprietario), generalPath, settings));

}

void HandleAccept(const boost::system::error_code& error, boost::asio::io_service& io_service, boost::shared_ptr< boost::asio::ip::tcp::socket > socket, boost::asio::ip::tcp::acceptor& acceptor
	, utente& utenteProprietario, std::string generalPath, Settings* settings)
{
	//Controllo di eventuali errori durante l'accettazione.
	if (error)
	{
		wxMessageBox("Errore nell'accettazione della connessione: " + error.message(), wxT("Errore"), wxOK | wxICON_ERROR);
		return;
	}

	//Se tutto è andato bene, si può lanciare un thread per accettare ciò che invia il client.
	std::thread(reciveAfterAccept, std::ref(io_service), std::move(*socket), std::ref(utenteProprietario), generalPath, settings).detach();

	//Chiamo nuovamente StartAccept per esser capace di accettare una nuova connessione
	StartAccept(io_service, acceptor, utenteProprietario, generalPath, settings);
};

void reciveTCPfile(utente& utenteProprietario, std::string generalPath, Settings* settings, boost::asio::io_service& io_service) {
	//Inizializzio l'acceptor
	tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), PORT_TCP));
	//Chiamo StartAccept per esser capace di accettare una nuova connessione
	StartAccept(io_service, acceptor, utenteProprietario, generalPath, settings);

	//Faccio partire la procedura boost, da interrompere quando cade la linea.
	io_service.run();
}


/*Funzionamento del protocollo:
-Ricezione della query: +DR, +FL o +IM (Direttorio, file, immagine)
1) query +FL
-Risposta +OK in caso di successo, -ERR in caso di errore
-Ricezione del nome del file
-Risposta +OK in caso di successo, -ERR in caso di mancata accetttazione da parte dell'utente.
-Chiamata alla funzione recive_file che gestisce la ricezione di un file
2) query +IM
-Risposta +OK in caso di successo, -ERR in caso di errore
-Chiamata alla funzione recive_file che gestisce la ricezione di un file
3)query +DR
-Risposta +OK in caso di successo, -ERR in caso di errore
-Ricezione della dimensione della directory (solo prima volta)
-Risposta +OK in caso di successo, -ERR in caso di errore  (solo prima volta)
-Ricezione del nome della directory
-Risposta +OK in caso di successo, -ERR in caso di errore o mancata accettazione
-Ricezine della prossima query
*/
void reciveAfterAccept(boost::asio::io_service& io_service, tcp::socket s, utente& utenteProprietario, std::string generalPath, Settings* settings) {
	long long directory_size_to_send, directorySize, directory_size_send = 0;
	int length;
	bool firstTime = true, firstDirectory = true;  //Indica se è la stringa contenente una directory che si sta ricevendo, cosi da inizializzare il tutto.
	char buf[PROTOCOL_PACKET];  //Buffer utile per le risposte in ricezione
	std::string ipAddrRemote, query, response, fileName;  //Ip di chi invia il file, query richesta, risposta inviata al client, e nome del file
	bool is_dir = false, is_file = false;
	std::string name_dir(""), path_dir("");
	
	try {
		ipAddrRemote = s.remote_endpoint().address().to_string();
		//Questo primo pacchetto serve a vedere se sto ricevendo un file o una directory
		length = read_some(s, buf, PROTOCOL_PACKET);
		buf[length] = '\0';
		query = buf;

		//Risoluzione della query di ricezione file da parte del client
		if (query == "+FL") {
			is_file = true;
			if (!boost::filesystem::is_directory(settings->getSavePath())) {
				wxMessageBox("La directory " + settings->getSavePath() + " Non esiste.\nControllare le proprie impostazioni e scegliere una directory valida.", wxT("Errore"), wxOK | wxICON_ERROR);
				response = "+ERR";
				write_some(s, response);
				if (s.is_open()) {
					s.close();
				}
				return;
			}
			//Qui ricevo un file
			//Se tutto va bene invio una risposta positiva
			response = "+OK";
			write_some(s, response);
			//La risposta conterrà il pathName
			length = read_some(s, buf, PROTOCOL_PACKET);
			buf[length] = '\0';
			fileName = buf;

			//Salvo su savePath il path assoluto dove scaricare il file
			std::string savePath = settings->getSavePath() + "\\" + fileName;
			std::string extension = boost::filesystem::extension(fileName);

			//Controllo l'impostazione "autosalvataggio dell'utente"
			if (settings->getAutoSaved() == save_request::SAVE_REQUEST_YES) {
				//Se l'utente ha settato l'opzione, vedo se l'utente vuol accettare il file o meno, oppure salvarlo in una parte specifica 
				wxMessageDialog *dial = new wxMessageDialog(NULL, wxT("Accettare il file \"" + fileName + "\" da " + utenteProprietario.getUsernameFromIp(ipAddrRemote) + "?"), wxT("INFO"), wxYES_NO | wxHELP | wxICON_QUESTION);
				dial->SetHelpLabel(wxID_SAVEAS);
				int ret_val = dial->ShowModal();
				if (ret_val == wxID_NO) {
					//Se l'utente non accetta il file, invio rispsota negativa al client.
					response = "-ERR";
					write_some(s, response);
					if (s.is_open()) {
						s.close();
					}
					return;
				}
				//Cambio il savePath se l'utente desidera scegliere un altro path di salvataggio per il file che si sta ricevendo
				else if (ret_val == wxID_HELP) {
					wxFileDialog saveFileDialog(NULL, "Salva " + fileName + " come", settings->getSavePath(), fileName,
						"(*" + extension + ")|*" + extension, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
					if (saveFileDialog.ShowModal() == wxID_CANCEL)
						return;     // the user changed idea...
					savePath = saveFileDialog.GetPath().ToStdString();
				}
			}

			//Controllo se il file già esiste e nel caso lo notifico all'utente
			if (boost::filesystem::is_regular_file(savePath)) {
				//In questo caso accetto la connessione
				wxMessageDialog *dial = new wxMessageDialog(NULL, wxT("Il file " + fileName + " già esiste. Accettarlo?"), wxT("INFO"), wxYES_NO | wxICON_QUESTION | wxSTAY_ON_TOP | wxCENTRE);
				/*if (dial->ShowModal() == wxID_YES) {
					//COSI SOVRASCRIVE
					//wxMutexGuiEnter();
					settings->showBal("Ricezione file", fileName + "\nDa " + utenteProprietario.getUsernameFromIp(ipAddrRemote));
					recive_file(io_service, s, settings->getSavePath() + "\\" + fileName);
				}
				else {
					//Se si rifiuta la ricezione, invio -ERR al client
					response = "-ERR";
					write_some(s, response);
				}

			}
			else {
				//In questo caso accetto la connessione senza alcun opzione scelta dall'utente
				settings->showBal("Ricezione file", fileName + "\nDa " + utenteProprietario.getUsernameFromIp(ipAddrRemote));
				recive_file(io_service, s, savePath);
				settings->showBal("File ricevuto", fileName + "\nDa " + utenteProprietario.getUsernameFromIp(ipAddrRemote));
			}
		}


		//Risolzuione della query di ricezione di un immagine del profilo da parte degli utenti connessi
		if (query == "+IM") {
			//Qui ricevo l'immagine di profilo
			//Se tutto va bene invio una risposta positiva
			response = "+OK";
			write_some(s, response);

			//Ricevo il file immagine, che salverò con il nome dell'ip dell'utente cosi da essere univoco
			try {
				//fp = nullptr;
				//recive_file(io_service, s, generalPath + "local_image\\" + ipAddrRemote + ".png", nullptr);
				recive_file(io_service, s, generalPath + "local_image\\" + ipAddrRemote + ".png");
				response = "+OK";
				write_some(s, response);
				//Notifico che l'immagine è stat ricevuta e posso registrare correttamente l'utente.
				utenteProprietario.registraImmagine(ipAddrRemote);
			}
			catch (std::exception&) {
				if (s.is_open()) {
					s.close();
				}
				return;
			}
		}

		//Risolzuione della query di ricezione di un immagine del profilo da parte degli utenti connessi
		if (query == "+DR") {
			is_dir = true;
			if (!boost::filesystem::is_directory(settings->getSavePath())) {
				wxMessageBox("La directory " + settings->getSavePath() + " Non esiste.\nControllare le proprie impostazioni e scegliere una directory valida.", wxT("Errore"), wxOK | wxICON_ERROR);
				response = "+ERR";
				write_some(s, response);
				if (s.is_open()) {
					s.close();
				}
				return;
			}

			std::string savePathName = settings->getSavePath();
			std::string dir_name;
			bool same_directory = false;
			//Finchè la query ricevuta è diversa da -END che indica la fine di invio di una cartella
			while (query != "-END") {
				//Ho due opzioni, ricevo la query +DR che mi notifica il bisogno di creare un nuovo direttorio
				//o la query +FL, che mi indica l'upload da parte del client verso il server di un nuovo file.
				if (query == "+DR") {
					//Rispondo al server che posso accettare il direttorio
					response = "+OK";
					write_some(s, response);
					//Se è il "primo giro", vuol dire che ricevo le informazioni utili alla ricezione della directory
					if (firstTime == true) {
						length = s.read_some(boost::asio::buffer(buf, PROTOCOL_PACKET));
						buf[length] = '\0';
						directorySize = std::atoi(buf);
						directory_size_to_send = directorySize;
						//Comunico al server che può inviare il file
						response = "+OK";
						write_some(s, response);
						firstTime = false;
					}
					//Aspetto il nome del path
					length = read_some(s, buf, PROTOCOL_PACKET);
					buf[length] = '\0';
					fileName = buf;
					
					//Se è la prima volta che ricevo una query di tipo +DR, vedo se l'utente ha settato l'opzione per la quale bisogna richiedere l'accettazione
					
					if (firstDirectory == true) {
						if (settings->getAutoSaved() == save_request::SAVE_REQUEST_YES) {
							wxMessageDialog *dial = new wxMessageDialog(NULL, wxT("Accettare la directory " + fileName + " da " + utenteProprietario.getUsernameFromIp(ipAddrRemote) + "?"), wxT("INFO"), wxYES_NO | wxICON_QUESTION | wxHELP);
							dial->SetHelpLabel(wxID_SAVEAS);
							int ret_val = dial->ShowModal();
							if (ret_val == wxID_NO) {
								//Se l'utente non accetta la directory, diamo risposta negativa.
								response = "-ERR";
								write_some(s, response);
								if (s.is_open()) {
									s.close();
								}
								return;
							}
							//Cambio il savePath se l'utente desidera scegliere un altro path di salvataggio per il file che si sta ricevendo
							else if (ret_val == wxID_HELP) {
								wxDirDialog selectDirDialog(NULL, "Salva " + fileName + " come", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
								if (selectDirDialog.ShowModal() == wxID_CANCEL)
									return;     // the user changed idea...
								savePathName = selectDirDialog.GetPath().ToStdString();
							}

						}

						//Controlliamo se la directory esiste già
						if (boost::filesystem::is_directory(savePathName + "\\" + fileName)) {
							
							//Richiedere se salvare o meno
							wxMessageDialog *dial = new wxMessageDialog(NULL, wxT("La directory " + fileName + " già esiste. Accettarla?"), wxT("INFO"), wxYES_NO | wxICON_QUESTION | wxSTAY_ON_TOP | wxCENTRE);
							/*if (dial->ShowModal() == wxID_YES) {
							//COSI SOVRASCRIVE
								settings->showBal("Ricezione Directory", fileName + "\nDa " + utenteProprietario.getUsernameFromIp(ipAddrRemote));
							}*/
							if (dial->ShowModal() == wxID_YES) {
							//COSI SALVA CON UN NUOVO NOME
								settings->showBal("Ricezione Directory", fileName + "\nDa " + utenteProprietario.getUsernameFromIp(ipAddrRemote));
								savePathName = savePathName + "\\" + "(" + std::to_string(getNumberDownload(savePathName, fileName)) + ")";
								same_directory = true;
							}
							else {
								//Se l'utente non accetta la directory, diamo risposta negativa.
								response = "-ERR\0";
								write_some(s, response);
								if (s.is_open()) {
									s.close();
								}
								return;
							}

						}
						else {
							settings->showBal("Ricezione Directory", fileName + "\nDa " + utenteProprietario.getUsernameFromIp(ipAddrRemote));
						}
						firstDirectory = false;

						name_dir = fileName;
						if (same_directory==true) {
							path_dir = savePathName + fileName;
						}
						else {
							path_dir = savePathName + "\\" + fileName;
						}
					}

					//Se arriviamo qui, vuol dire che tutto si è svolto in modo positivo, e posso accettare la directory
					//Rispondo con +OK
					response = "+OK";
					write_some(s, response);
					//Creo la directory.
					if (same_directory==true) {
						boost::filesystem::create_directory(savePathName + fileName);
					}
					else {
						boost::filesystem::create_directory(savePathName + "\\" + fileName);
					}
					
				}

				//In questo caso vuol dire che sto ricevondo un file da salvare all'interno di un direttorio creato precedentemente.
				if (query == "+FL") {
					//Rispondo con +OK
					response = "+OK";
					write_some(s, response);
					//Leggo il nome del file
					length = read_some(s, buf, PROTOCOL_PACKET);
					buf[length] = '\0';
					fileName = buf;

					if (same_directory == true) {
						fileName = savePathName + fileName;
					}
					else {
						fileName = savePathName + "\\" + fileName;
					}
					
				//	recive_file(io_service, s, fileName);
					recive_file(io_service, s, fileName);

					//Vedo la dim del file che ho ricevuto, e aggiorno la quantità di byte ricevuta fin ora.
					//Ciò è utile per l'avanzamento della barra di progresso.
					directory_size_send += (size_t)boost::filesystem::file_size(fileName);
					//Invio la risposta +CN per dire al client che ho ricevuto il tutto, e può continuare ad inviare.
					response = "+CN";
					write_some(s, response);
				}
				//Leggo la prossima Query, ovvero leggo se sarà un File o un altra Directory
				length = read_some(s, buf, PROTOCOL_PACKET);
				buf[length] = '\0';
				query = buf;
			}

			settings->showBal("Ricezione Directory", name_dir + "\nDa " + utenteProprietario.getUsernameFromIp(ipAddrRemote) + "\navvenuta con successo.");
		}
	}
	catch (std::exception& e) {
		if (s.is_open()) {
			s.close();
		}
		std::string error(e.what());
		if (is_file == true) {
			wxMessageBox("File: " + fileName + "\n" + error, wxT("Errore"), wxOK | wxICON_ERROR);
		}
		else if (is_dir==true) {
			wxMessageBox("Directory: " + name_dir + "\n" + error, wxT("Errore"), wxOK | wxICON_ERROR);
			if (boost::filesystem::is_directory(path_dir + "\\") == true) {
				boost::filesystem::remove_all(path_dir + "\\");
			}
		}
		else {
			wxMessageBox(error, wxT("Errore"), wxOK | wxICON_ERROR);
		}

		return;
	}
	
	if (s.is_open()) {
		s.close();
	}
}

void recive_file(boost::asio::io_service& io_service, boost::asio::basic_stream_socket<boost::asio::ip::tcp>& s, std::string fileName) {

	std::ofstream file_out(fileName, std::ios::out | std::ios::binary);  //File da salvare
	std::string response; //Risposta da invare al client
	char buf[PROTOCOL_PACKET];  //Buffer che contiene i pacchetti utili alla sincronizzazione con il client.
	char buf_recive[BUFLEN];  //Buffer che conterrà i pacchetti contenenti il file
	long long dim_recived = 0, dim_read, size, count = 0;
	int length;
	bool abort = false;
	try
	{
		if (file_out.is_open()) {
			//Se tutto va bene, dico che posso ricevere il file con +OK
			response = "+OK";
			write_some(s, response);
			//Leggo la dimensione del file che ricevero, e la salvo su size
			length = read_some(s, buf, PROTOCOL_PACKET);
			buf[length] = '\0';
			size = std::atoll(buf);
			/*if (fp != nullptr) {
				wxThreadEvent event1(wxEVT_THREAD, SetMaxDim_EVENT);
				event1.SetPayload(size);
				wxQueueEvent(fp, event1.Clone());
			}*/

			//Comunico al server che può inviare il file
			response = "+OK";
			write_some(s, response);
			
			wxThreadEvent event2(wxEVT_THREAD, SetMaxDim_EVENT);
			//ricevo pacchetti finchè non ho ricevuto tutto il file
			/*if (fp != nullptr) {
				wxMutexGuiEnter();
				abort = fp->testAbort();
				wxMutexGuiLeave();
			}*/
			while (dim_recived<size && !abort)
			{
				dim_read = read_some(s, buf_recive, BUFLEN);
				file_out.write(buf_recive, dim_read);
				dim_recived += dim_read;
				/*if (fp != nullptr && count++ == FILTER_EVENT) {
					count = 0;
					event2.SetPayload(dim_recived);
					wxQueueEvent(fp, event2.Clone());
				}
				if (fp != nullptr) {
					wxMutexGuiEnter();
					abort = fp->testAbort();
					wxMutexGuiLeave();
				}*/
			}
			//ultimo evento per settare avanzamento a 100%
			/*if (fp != nullptr) {
				event2.SetPayload(dim_recived);
				wxQueueEvent(fp, event2.Clone());
			}*/
			file_out.close();
		}
		else
		{
			//Se ho avuto qualche errore nell'apertura del file, invio -ERR
			response = "-ERR";
			write_some(s, response);
			return throw std::invalid_argument("Errore nell'apertura del file.");
		}
	}
	catch (std::exception& e)
	{
		//In tal caso vuol dire che ho riscontrato qualche problema 
		file_out.close();
		boost::filesystem::remove(fileName);
		return throw std::invalid_argument(e.what());
	}
}

//Torna il numero di download
int getNumberDownload(std::string& path, std::string& fileName) {
	int counter = 0;
	bool find = false;
	if (boost::filesystem::is_directory(path + "\\" + fileName)) {
		while (true) {
			find = false;
			counter++;
			for (boost::filesystem::directory_iterator it(path); it != boost::filesystem::directory_iterator(); ++it) {
				boost::filesystem::path filePath(boost::filesystem::absolute(*it));
				if (boost::filesystem::is_directory(*it)) {
					std::string pathToCheck(path + "\\(" + std::to_string(counter) + ")" + fileName);
					if (filePath.string() == pathToCheck) {
						find = true;
					}
				}
			}
			if (find == false) {
				break;
			}
		}

	}
	else {
		while (true) {
			find = false;
			counter++;
			for (boost::filesystem::directory_iterator it(path); it != boost::filesystem::directory_iterator(); ++it) {
				boost::filesystem::path filePath(boost::filesystem::absolute(*it));
				if (boost::filesystem::is_regular_file(*it)) {
					if (filePath.string() == path + "\\(" + std::to_string(counter) + ")" + fileName) {
						find = true;
					}
				}
			}
			if (find == false) {
				break;
			}
			
		}
	}
	return counter;
}
