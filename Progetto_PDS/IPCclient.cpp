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

	//Sopprime i messaggi di log da MakeConnection()
	wxLogNull nolog;

	m_connection = (wxConnection *)MakeConnection(sHost, sService, sTopic);
	return m_connection != NULL;
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