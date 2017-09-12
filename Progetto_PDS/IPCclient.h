#pragma once

#include <wx/wx.h>

#include "ipcsetup.h"

class MyConnectionClient : public wxConnection
{
public:
	virtual bool DoExecute(const void *data, size_t size, wxIPCFormat format) wxOVERRIDE;
	virtual bool DoPoke(const wxString& item, const void* data, size_t size, wxIPCFormat format) wxOVERRIDE;
	virtual bool OnDisconnect() wxOVERRIDE;
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
