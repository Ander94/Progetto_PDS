#include "client.h"
#include "timeout.h"
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
void send_file(boost::asio::io_service& io_service, boost::asio::basic_stream_socket<boost::asio::ip::tcp>& s,
	std::string filePath, std::string sendPath, UserProgressBar* progBar);


/********************************************************************************
Invia la directory specificata in initialAbsolutePath sul socket s
Riceve come parametri:
-Il socket sul quale inviare il file
-Il path assoluto della directory da inviare
-Il nome della directory da inviare
-Il riferimento alla barra di progresso per la grafica.
**********************************************************************************/
void send_directory(boost::asio::io_service& io_service, boost::asio::basic_stream_socket<boost::asio::ip::tcp>& s,
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
double long folder_size(std::string absolutePath);

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
	boost::thread(sendThreadTCPfile, boost::ref(utenteProprietario), username, initialAbsolutePath, progBar).detach();
}


/*************
Funzionamento del protocollo dell'invio dell'immagine del profilo
-Connessione sulla porta PORT_TCP
-Invio della stringa "+IM"
-Ricezione della stringa "+OK" in caso di successo, "-ERR" in caso di errore.
-Invio della dimensione del file sotto forma di stringa
-Ricezione della stringa "+OK" in caso di successo, "-ERR" in caso di errore.
-Invio del file sotto forma di pacchetti di dimensione BUFLEN
-Ricezione della stringa "+OK" in caso di successo, "-ERR" in caso di errore.
********************/
void sendImage(std::string filePath, std::string ipAddr) {

	boost::asio::io_service io_service;  //Procedura di servizio boost
	tcp::resolver resolver(io_service); //Da la possibilità di risolvere la query  specificata sucessivamente
	tcp::resolver::query query(tcp::v4(), ipAddr, std::to_string(PORT_TCP));  //La query mostra la volontà di connettersi all'indirizzo IPv4 alla porta PORT_TCP
	tcp::resolver::iterator iterator = resolver.resolve(query);
	tcp::socket s(io_service);

	//Mi connetto al server, se qualcosa non va a buon fine, ritorno al main lanciando un eccezione
	try {
		boost::asio::connect(s, iterator);
	}
	catch (std::exception e) {
		return throw std::invalid_argument("Non sono riuscito a connettermi con l'utente. Controllare che l'utente sia attivo.");
	}


	try {
		std::ifstream file_in(filePath, std::ios::in | std::ios::binary);  //File da inviare 
		std::string fileSize; //Dimensione del file da inviare sotto forma di stringa
		size_t dim_to_send, size, length, dim_write;
		char buf_to_send[BUFLEN];   //Pacchetto che contiene parte del file da inviare
		char buf_response[PROTOCOL_PACKET];  //Contiene la risposta del protocollo
		std::string send, response;          //Contiene domanda e risposta del protocollo

											 //Valuta la dimensione del file da inviare, sotto forma di stringa 
		size = boost::filesystem::file_size(filePath);
		fileSize = std::to_string(size);

		if (file_in.is_open())
		{
			//Invio +IM per dire che è un file immagine
			send = "+IM";
			boost::asio::write(s, boost::asio::buffer(send));
			//Vedo se il server ha risposto con successo, e nel caso sollevo un eccezione
			length = s.read_some(boost::asio::buffer(buf_response, PROTOCOL_PACKET));
			buf_response[length] = '\0';
			response = buf_response;
			if (response != "+OK") {
				return;
			}

			//Invio qui la dimensione del file
			boost::asio::write(s, boost::asio::buffer(fileSize));
			//Ed attendo la risposta
			length = s.read_some(boost::asio::buffer(buf_response, PROTOCOL_PACKET));
			buf_response[length] = '\0';
			if (response != "+OK") {
				return; 
			}
			//Invio i diversi pacchetti che contengono l'immagine, i pacchetti verranno poi ricomposti lato server.
			dim_to_send = size;
			while (dim_to_send > 0) {
				dim_write = dim_to_send < BUFLEN ? dim_to_send : BUFLEN;
				dim_to_send -= dim_write;
				file_in.read(buf_to_send, dim_write);
				write_some(s, buf_to_send, dim_write);
			}

			//Controllo il successo della ricezione dell'immagine.
			length = s.read_some(boost::asio::buffer(buf_response, PROTOCOL_PACKET));
			buf_response[length] = '\0';
			if (response != "+OK") {
				return; 
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

	s.close();
	io_service.stop();
}

void sendThreadTCPfile(utente& utenteProprietario, std::string username, std::string initialAbsolutePath, UserProgressBar* progBar) {

	std::string ipAddr;       //Indirizzo ip dell'utente a cui inviare il file
	std::string basename;     //Nome del file/cartella da inviare.

	wxThreadEvent event(wxEVT_THREAD, CLIENT_EVENT);

	//Controllo se l'utente esiste.
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

	//Ottengo il nome della cartella/file che devo inviare. Attenzione: se viene tornato il nome di un file bisognerà aggiungere l'estensione.
	basename = boost::filesystem::basename(initialAbsolutePath);

	//Ne ottengo il path assoluto
	boost::filesystem::path path_absolutePath(initialAbsolutePath);

	// Avvio la procedura di servizio boost, che servirà a mettersi in contatto con il server																	  
	boost::asio::io_service io_service; //Procedura di servizio boost
	tcp::resolver resolver(io_service); //Da la possibilità di risolvere la query  specificata sucessivamente
	tcp::resolver::query query(tcp::v4(), ipAddr, std::to_string(PORT_TCP)); //La query mostra la volontà di connettersi all'indirizzo IPv4 alla porta PORT_TCP
	tcp::resolver::iterator iterator = resolver.resolve(query);
	tcp::socket s(io_service);   //Inizializzo il socket

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
			send_file(io_service, s, initialAbsolutePath, basename + boost::filesystem::extension(initialAbsolutePath), progBar);
		}
		catch (std::exception& e) {
			wxMessageBox(e.what(), wxT("Errore"), wxOK | wxICON_ERROR);
		}

	}
	//Senno chiamo send_directory
	else if (boost::filesystem::is_directory(path_absolutePath)) {
		try {
			send_directory(io_service, s, initialAbsolutePath, basename, progBar);
		}
		catch (std::exception& e) {
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

/*************
Funzionamento del protocollo dell'invio dell'immagine del profilo
-Connessione sulla porta PORT_TCP
-Invio della stringa "+DR" per comunicare l'invio di un direttorio
-Ricezione della stringa "+OK" in caso di successo, "-ERR" in caso di errore.
-Invio della dimensione del direttorio
-Ricezione della stringa "+OK" in caso di successo, "-ERR" in caso di errore.
-Invio del nome del direttorio
Basandomi sull'invio del file o del nome di un altro direttorio, ho due opzioni:
1)Chiamo send_file, per inviare un file.
Attendo poi la stringa +CN da parte del server, che mi indica che il file è stato salvato con successo, e posso procedere con l'invio di un latro file.
2)Invio +DR al server, seguito dalla stringa che contiene il nome del direttorio,
attendendo poi la risposta +OK per indicarmi che il salvataggio della nuova directory è avvenuto con successo in entrambi i casi
-Invio la strina -END per notificare al server la fine dell'invio.
********************/

void send_directory(boost::asio::io_service& io_service, boost::asio::basic_stream_socket<boost::asio::ip::tcp>& s,
	std::string initialAbsolutePath, std::string folder, UserProgressBar* progBar) {

	std::string directorySize;   //Dimensione della directory sotto forma di stringa
	double long directory_size_to_send;
	size_t directory_size_send = 0, length; //Dimensione della directory, dimensione inviata e lunghezza di un pacchetto di risposta da parte del server
	std::string send, response;  //Pacchetto inviato/ricevuto da parte del server sotto forma di stringa.
	char buf_recive[PROTOCOL_PACKET];  //Buf che contine le risposte provenienti dal server

									   //Invio +DR per comunicare che voglio invare un direttorio
	send = "+DR";
	boost::asio::write(s, boost::asio::buffer(send));

	//Attendo la risposta da parte del server
	length = s.read_some(boost::asio::buffer(buf_recive, PROTOCOL_PACKET));
	buf_recive[length] = '\0';
	response = buf_recive;
	if (response != "+OK") {
		return throw std::invalid_argument("Errore nell'invio del direttorio.");
	}

	//Valuto la dimensione della directory
	directory_size_to_send = folder_size(initialAbsolutePath);

	wxThreadEvent event(wxEVT_THREAD, SetMaxDir_EVENT);
	event.SetPayload(directory_size_to_send);
	wxQueueEvent(progBar, event.Clone());

	//Converto in string la dimensione cosi da poterla inviare al server.
	directorySize = std::to_string(directory_size_to_send);
	boost::asio::write(s, boost::asio::buffer(directorySize));
	length = s.read_some(boost::asio::buffer(buf_recive, PROTOCOL_PACKET));
	buf_recive[length] = '\0';
	response = buf_recive;
	//E attendo la risposta da parte del server
	if (response != "+OK") {
		return throw std::invalid_argument("Errore nell'invio del direttorio.");
	}

	//Invio il nome della cartella
	boost::asio::write(s, boost::asio::buffer(folder));
	length = s.read_some(boost::asio::buffer(buf_recive, PROTOCOL_PACKET));
	buf_recive[length] = '\0';
	response = buf_recive;
	if (response != "+OK") {
		return throw std::invalid_argument("Attenzione: l'utente ha rifiutato il trasferimento.");
	}

	for (bf::recursive_directory_iterator it(initialAbsolutePath); it != bf::recursive_directory_iterator(); ++it)
	{
		//Attenzione:qua bisogna inviare il relativePath
		//Il concetto è questo: noi utilizziamo il path assoluto, ma a lui dobbiamo inviare quello relativo al nome della directory da inviare
		if (!bf::is_directory(*it)) {
			//Se devo inviare un file chiamo send_file.
			bf::path p(bf::absolute(*it));
			wxThreadEvent event(wxEVT_THREAD, SetNewFile_EVENT);
			event.SetString(p.filename().string());
			wxQueueEvent(progBar, event.Clone());
			try {
				send_file(io_service, s, bf::absolute(*it).string(), relative_path(bf::absolute(*it).string(), initialAbsolutePath, folder), progBar);
				//Se è stato chiamato testAbort, vuol dire che l'invio è stato interroto a seguito del click su "CANCEL" nella GUI
				if (progBar->testAbort())
					return;
			}
			catch (std::exception& e) {
				return throw std::invalid_argument(e.what());
			}
			//Aggiorno la dimensione inviata fin ora
			directory_size_send += (size_t)bf::file_size(*it);
			//Attendo la stringa +CN da parte del server che mi indica che posso continuare.
			length = s.read_some(boost::asio::buffer(buf_recive, PROTOCOL_PACKET));
			buf_recive[length] = '\0';
			response = buf_recive;
			if (response != "+CN") {
				return throw std::invalid_argument("Errore nell'invio del direttorio.");
			}

		}
		else {
			//In questo caso voglio invare una stringa contenente il nome del direttorio.
			//Prima invio la richesta +DR al server
			send = "+DR";
			boost::asio::write(s, boost::asio::buffer(send));
			length = s.read_some(boost::asio::buffer(buf_recive, PROTOCOL_PACKET));
			buf_recive[length] = '\0';
			response = buf_recive;
			//E ne attendo la risposta
			if (response != "+OK") {
				wxMessageBox("Errore nell'invio del direttorio.", wxT("Errore"), wxOK | wxICON_ERROR);
				return throw std::invalid_argument("Errore nell'invio del direttorio.");
			}
			//Successivamente invio il path relativo a folder al server.
			boost::asio::write(s, boost::asio::buffer(relative_path(bf::absolute(*it).string(), initialAbsolutePath, folder)));
			//E ne attendo la risposta.
			length = s.read_some(boost::asio::buffer(buf_recive, PROTOCOL_PACKET));
			buf_recive[length] = '\0';
			response = buf_recive;
			if (response != "+OK") {
				return throw std::invalid_argument("Errore nell'invio del direttorio.");
			}
		}
	}
	send = "-END";
	boost::asio::write(s, boost::asio::buffer(send));
}



/*************
Funzionamento del protocollo dell'invio dell'immagine del profilo
-Connessione sulla porta PORT_TCP
-Invio della stringa "+FL"
-Ricezione della stringa "+OK" in caso di successo, "-ERR" in caso di errore.
-Invio del nome del file
-Ricezione della stringa "+OK" in caso di successo, "-ERR" in caso di errore.
-Invio della dimensione del file sotto forma di stringa
-Ricezione della stringa "+OK" in caso di successo, "-ERR" in caso di errore.
-Invio del file sotto forma di pacchetti di dimensione BUFLEN
********************/
void send_file(boost::asio::io_service& io_service, boost::asio::basic_stream_socket<boost::asio::ip::tcp>& s,
	std::string filePath, std::string sendPath, UserProgressBar* progBar) {

	std::ifstream file_in(filePath, std::ios::in | std::ios::binary);  //Fiel da inviare
	std::string fileSize; //Dimensione del file sotto forma di stringa 
	double long size, dim_write, dim_send = 0, dim_to_send;    //Dimensione del file sotto forma di double per effettuare la divisione
	size_t length;
	boost::posix_time::ptime start, end; //Utile per valuare il tempo di invio di un pacchetto verso il server
	long int dif = 0, sec = 0;  //Utile per valuare il tempo di invio di un pacchetto verso il server
	int calcola_tempo = 1; //è utile per non valutare il tempo troppe volte, ma solo ogni 50 pacchetti inviati.
	std::string response, send; //Risposta del server e Buffer utile all'invio di un pacchetto
	char buf_to_send[BUFLEN];  //char utili per caricare il pacchetto di lunghezza BUFLEN da inviare al server.
	char buf_recive[PROTOCOL_PACKET]; //Buffer utile alla ricezione di un pacchetto. La funzione da me utilizzata prevede di ricevere un char di lunghezza fissa.

	try {

		//Valuto la dimensione del file 
		size = bf::file_size(filePath);

		wxThreadEvent event(wxEVT_THREAD, SetMaxFile_EVENT);
		event.SetPayload(size);
		wxQueueEvent(progBar, event.Clone());

		//Converto la dimensione in una stringa cosi da poterla inviare al server
		fileSize = std::to_string(size);

		if (file_in.is_open())
		{
			//Qui inizia il protocollo di invio del file 
			//Invio +FL per dire che è un file
			send = "+FL";
			boost::asio::write(s, boost::asio::buffer(send));

			//Leggo la risposta da parte del server e se negativa, lancio un eccezione
			length = s.read_some(boost::asio::buffer(buf_recive, PROTOCOL_PACKET));
			buf_recive[length] = '\0';
			response = buf_recive;
			if (response != "+OK") {
				return throw std::invalid_argument("Errore nell'invio del file.");
			}

			//invio il nome del file al server
			boost::asio::write(s, boost::asio::buffer(sendPath));
			//Leggo la risposta da parte del server e se negativa, lancio un eccezione
			length = s.read_some(boost::asio::buffer(buf_recive, PROTOCOL_PACKET));
			buf_recive[length] = '\0';
			response = buf_recive;
			if (response != "+OK") {
				return throw std::invalid_argument("Attenzione: l'utente ha rifiutato il trasferimento.");
			}

			//Invio qui la dimensione del file
			boost::asio::write(s, boost::asio::buffer(fileSize));
			//Leggo la risposta da parte del server e se negativa, lancio un eccezione
			length = s.read_some(boost::asio::buffer(buf_recive, PROTOCOL_PACKET));
			buf_recive[length] = '\0';
			if (response != "+OK") {
				return throw std::invalid_argument("Errore nell'invio del file.");
			}


			wxThreadEvent event1(wxEVT_THREAD, IncFile_EVENT);
			wxThreadEvent event2(wxEVT_THREAD, SetTimeFile_EVENT);

			dim_to_send = size;
			start = boost::posix_time::second_clock::local_time();

			

			//Carico il buffer buf_to_send di dimensione BUFLEN, che verrà caricato ogni volta con una parte diversa del file
			//e poi inviato al server
			while (dim_send < size && !progBar->testAbort()) {
				dim_write = dim_to_send < BUFLEN ? dim_to_send : BUFLEN; //Valuto la quantità di dati da caricare nel buffer, basandomi sulla quantità di file rimanente.
				dim_send += dim_write;   //Incremento la dimensione già inviata di dim_write
				dim_to_send -= dim_write;  //decremento la dimensione da inviare di dim_write
				file_in.read(buf_to_send, dim_write);  //Carico in buf_to_send una quantità di dati pari a dim_write.
				//boost::asio::write(s, boost::asio::buffer(buf_to_send, (int)dim_write));   //E la invio al server.
				write_some(s, buf_to_send, dim_write);
				
				//Valuto il tempo di invio di EVALUATE_TIME pacchetti.
				//Ho fatto la scelta di valutare il tempo ogni EVALUATE_TIME 
				//pacchetti perchè sennò la variazione di tempo sarebbe stata troppo evidente.
				if (calcola_tempo % EVALUATE_TIME == 0)
				{
					end = boost::posix_time::second_clock::local_time();
					dif = (end - start).total_seconds();
					//Valuto quanti secondi sono stati necessari per inviare dim_send byte.
					sec = (int)((((size - dim_send) / ((dim_send)))*dif));
					// dif : EVALUATE_TIME = sec : size-dim_send/BUFLEN pacchetti?

					//start = boost::posix_time::second_clock::local_time();

				}
				calcola_tempo++;

				event1.SetPayload(dim_send);
				wxQueueEvent(progBar, event1.Clone());
				event2.SetPayload(sec);
				wxQueueEvent(progBar, event2.Clone());
			}
			file_in.close();
		}
		else {
			return throw std::invalid_argument("File non aperto correttamente.");
		}
	}
	catch (std::exception& e)
	{
		file_in.close();
		return throw std::invalid_argument(e.what());
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


double long folder_size(std::string absolutePath) {

	double long size = 0;
	for (bf::recursive_directory_iterator it(absolutePath); it != bf::recursive_directory_iterator(); ++it)
	{
		if (!bf::is_directory(*it)) {
			size += (double long)bf::file_size(*it);
		}
	}

	return size;
}