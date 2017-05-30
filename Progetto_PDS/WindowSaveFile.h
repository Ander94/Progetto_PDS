#pragma once

#include <wx/wx.h>




class WindowSaveFile : public wxDialog
{
private:
	wxButton *m_SelectDir;
	wxButton *m_Accept;
	wxButton *m_Cancel;


public:
	WindowSaveFile();

	void WindowSaveFile::OnSelectDir(wxCommandEvent& event);
	void WindowSaveFile::OnAccept(wxCommandEvent& event);
	void WindowSaveFile::OnCancel(wxCommandEvent& event);
};
