#pragma once

#include <wx/wx.h>
#include <wx/ipc.h>

#include "TaskBarIcon.h"
#include "ipcsetup.h"

class MyConnectionServer : public wxConnection
{
private:
	class MainFrame* m_frame;
public:
	MyConnectionServer(class MainFrame* frame) { m_frame = frame; }

	virtual bool OnExecute(const wxString& topic, const void *data, size_t size, wxIPCFormat format) wxOVERRIDE;
	virtual bool OnPoke(const wxString& topic, const wxString& item, const void *data, size_t size, wxIPCFormat format) wxOVERRIDE;
	virtual bool OnDisconnect() wxOVERRIDE;
};

class MyServer : public wxServer
{
private:
	MainFrame* m_frame;

public:
	MyServer(MainFrame* frame);
	~MyServer();

	void Disconnect();
	bool IsConnected() { return m_connection != NULL; }
	MyConnectionServer *GetConnection() { return m_connection; }

	virtual wxConnectionBase *OnAcceptConnection(const wxString& topic) wxOVERRIDE;

protected:
	MyConnectionServer *m_connection;
};
