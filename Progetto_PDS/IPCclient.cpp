//NON COMMENTATO

#include <wx/wx.h>
#include <wx/datetime.h>

#include "ipcsetup.h"
#include "IPCclient.h"


// ----------------------------------------------------------------------------
// MyClient
// ----------------------------------------------------------------------------
MyClient::MyClient() : wxClient()
{
	m_connection = NULL;
}

bool MyClient::Connect(const wxString& sHost, const wxString& sService, const wxString& sTopic)
{
	// suppress the log messages from MakeConnection()
	wxLogNull nolog;

	m_connection = (MyConnectionClient *)MakeConnection(sHost, sService, sTopic);
	return m_connection != NULL;
}

wxConnectionBase *MyClient::OnMakeConnection()
{
	return new MyConnectionClient;
}

void MyClient::Disconnect()
{
	if (m_connection)
	{
		m_connection->Disconnect();
		wxDELETE(m_connection);
	}
}

MyClient::~MyClient()
{
	Disconnect();
}

// ----------------------------------------------------------------------------
// MyConnection
// ----------------------------------------------------------------------------


//bool MyConnectionClient::OnDisconnect()
//{
//	return true;
//}
//
//bool MyConnectionClient::DoExecute(const void *data, size_t size, wxIPCFormat format)
//{
//	bool retval = wxConnection::DoExecute(data, size, format);
//	return retval;
//}
//
//bool MyConnectionClient::DoPoke(const wxString& item, const void *data, size_t size, wxIPCFormat format)
//{
//	return wxConnection::DoPoke(item, data, size, format);
//}