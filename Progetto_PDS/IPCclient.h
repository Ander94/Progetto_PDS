#pragma once

#include <wx/wx.h>

#include "ipcsetup.h"

class MyConnectionClient : public wxConnection
{
//public:
//	//Comunica al processo principale di mostrare la finestra principale
//	virtual bool DoExecute(const void *data, size_t size, wxIPCFormat format) wxOVERRIDE;
//
//	//Passa al processo principale il path del file o del direttorio da inviare
//	virtual bool DoPoke(const wxString& item, const void* data, size_t size, wxIPCFormat format) wxOVERRIDE;
//
//	//Termina la connessione
//	virtual bool OnDisconnect() wxOVERRIDE;
};

class MyClient : public wxClient
{
public:
	MyClient();
	~MyClient();
	bool Connect(const wxString& sHost, const wxString& sService, const wxString& sTopic);
	void Disconnect();
	wxConnectionBase *OnMakeConnection() wxOVERRIDE;
	bool IsConnected() { return m_connection != NULL; };
	MyConnectionClient *GetConnection() { return m_connection; };

protected:
	MyConnectionClient *m_connection;
};
