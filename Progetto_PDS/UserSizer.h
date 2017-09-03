#pragma once

#include "utente.h"

#include <wx/wx.h>
#include <wx/tglbtn.h>

#include "WindowSelectUser.h"

class UserSizer : public wxWindow
{
private:
	class WindowSelectUser* m_parentWindow;
	wxStaticText* m_text;
	wxBitmapToggleButton* m_button;
	Settings* m_settings;

	//riferimento all'utente, valutere shared_ptr
	utente m_utente;

	void OnUserClick(wxCommandEvent& event);
public:
	UserSizer(wxWindow* parent, Settings* settings, utente &user);
	~UserSizer();
	
	void UserSizer::PerformClick();
	std::string getIpAddr() { return m_utente.getIpAddr(); }
	std::string getUsername() { return m_utente.getUsername(); }
	utente getUser() { return m_utente; }
};