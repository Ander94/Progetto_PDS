#include <wx/wx.h>
#include <wx/wfstream.h>

#include "WindowSaveFile.h"

//TODO completare finestra di salvataggio file
WindowSaveFile::WindowSaveFile()
	: wxDialog(NULL, wxID_ANY, wxT("File in arrivo"), wxDefaultPosition, wxDefaultSize) {

	
	wxBoxSizer *m_sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *m_ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	
	//testo
	wxStaticText *testo = new wxStaticText(this, wxID_ANY, "File in arrivo da USER", wxDefaultPosition, wxDefaultSize,
		wxALIGN_LEFT);
	m_sizer->Add(testo, 1, wxALL, 3);

	//pulsanti
	m_SelectDir = new wxButton(this, wxID_SAVE, "SELECT DIR");
	m_Accept = new wxButton(this, wxID_SAVE, "ACCEPT");
	m_Cancel = new wxButton(this, wxID_SAVE, "CANCEL");
	m_ButtonSizer->Add(m_SelectDir, 1, wxALL, 3);
	m_ButtonSizer->Add(m_Accept, 1, wxALL, 3);
	m_ButtonSizer->Add(m_Cancel, 1, wxALL, 3);

	m_sizer->Add(m_ButtonSizer, 0, wxALL, 3);
	
	//eventi collegati ai pulsanti
	m_SelectDir->Bind(wxEVT_BUTTON, &WindowSaveFile::OnSelectDir, this);
	m_Accept->Bind(wxEVT_BUTTON, &WindowSaveFile::OnAccept, this);
	m_Cancel->Bind(wxEVT_BUTTON, &WindowSaveFile::OnCancel, this);

	this->SetSizerAndFit(m_sizer);
}

//finestra di salvataggio
void WindowSaveFile::OnSelectDir(wxCommandEvent& event) {

	wxDirDialog selectDirDialog(NULL, "Choose input directory", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

	if (selectDirDialog.ShowModal() == wxID_CANCEL)
		return;     // the user changed idea...

	std::string path = selectDirDialog.GetPath().ToStdString();
	wxMessageBox(selectDirDialog.GetPath(), wxT("INFO"), wxOK | wxICON_INFORMATION);
}

void WindowSaveFile::OnAccept(wxCommandEvent & event) {

}

void WindowSaveFile::OnCancel(wxCommandEvent & event) {
	Destroy();
}
