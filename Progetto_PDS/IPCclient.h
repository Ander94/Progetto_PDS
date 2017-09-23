#pragma once

#include <wx/wx.h>

#include "ipcsetup.h"


class MyClient : public wxClient
{
private:
	wxConnection *m_connection;

public:
	MyClient();
	~MyClient();
	bool Connect(const wxString& sHost, const wxString& sService, const wxString& sTopic);
	void Disconnect();
	wxConnection *GetConnection() { return m_connection; };
};
