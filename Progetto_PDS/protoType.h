#pragma once
#define PORT_UDP 1500 //Porta su cui si gestiscono i pacchetti UDP
#define PORT_UDP_OWNIP 1501 //Porta su cui si gestiscono i pacchetti UDP
#define PORT_TCP 1400 //Porta su cui si gestisce la connessione TCP
#define BUFLEN 4096   //Grandezza del buffer che viene inviato in rete (file)
#define TIME_SEND_MESSAGE_UDP 300 //ms   //Un pacchetto UDP viene inviato in rete ogno TIME_SEND_MESSAGE_UDP ms
#define CHECK_TIME 1000 //ms    //Controllo dello stato dell'utente
#define DELETE_USER 2 //s       //Cancello un utente dalla lista degli utenti connessi dopo DELETE_USER secondi, ciò vuol dire che l'utente è rimasto inattivo.
#define UPDATE_WINDOW 1000 //ms   //Aggiornamento della finestra
#define PROTOCOL_PACKET 4096   //Grandezza del buffer che viene inviato in rete (Pacchetti utilizzati per protocollo)
#define EVALUATE_TIME 1000      //Calcolo il tempo rimanente per l'invio di un file ogni EVALUATE_TIME pacchetti
#define TIMEOUT 10 //s      //Tempo per cui una connessione può rimanere inattiva
#define TIMEOUT_IMAGE 3 //s      //Tempo per cui si cerca di inviare un immagine
#define EVALUATE_TIME_DIR 800  //Calcolo il tempo rimanente per l'invio di un file ogni EVALUATE_TIME pacchetti
#define USERNAME_MAX_LENGTH 20 //Massima lunghezza dell'username
