//NON COMMENTATO

#include <wx/wx.h>
#include <wx/datetime.h>

#include "ipcsetup.h"
#include "IPCserver.h"
#include "TaskBarIcon.h"

// ----------------------------------------------------------------------------
// MyServer
// ----------------------------------------------------------------------------

MyServer::MyServer(MainFrame* frame) : wxServer()
{
	this->m_frame = frame;
	m_connection = NULL;
}

MyServer::~MyServer()
{
	Disconnect();
}

wxConnectionBase *MyServer::OnAcceptConnection(const wxString& topic)
{
	if (topic == IPC_TOPIC)
	{
		m_connection = new MyConnectionServer(m_frame);
		return m_connection;
	}
	return NULL;
}

void MyServer::Disconnect()
{
	if (m_connection)
	{
		wxDELETE(m_connection);
	}
}

// ----------------------------------------------------------------------------
// MyConnection
// ----------------------------------------------------------------------------

bool
MyConnectionServer::OnExecute(const wxString& topic,
	const void *data,
	size_t size,
	wxIPCFormat format)
{
	m_frame->Iconize(false);
	m_frame->Show(true);
	return true;
}

bool
MyConnectionServer::OnPoke(const wxString& topic,
	const wxString& item,
	const void *data,
	size_t size,
	wxIPCFormat format)
{
	m_frame->SendFile(item.ToStdString());
	return wxConnection::OnPoke(topic, item, data, size, format);
}

bool MyConnectionServer::OnDisconnect()
{
	return true;
}