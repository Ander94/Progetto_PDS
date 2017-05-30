#include <wx/statbmp.h>

#include "UserSizer.h"
#include "utente.h"
#include "WindowSelectUser.h"

// TODO gestire la dimensione dei nomi in caso sia eccessiva!

UserSizer::UserSizer(wxWindow* parent, utente& user) : wxWindow(parent, wxID_ANY)
{
	//inizializzo alcuni parametri
	m_parentWindow = dynamic_cast<WindowSelectUser*>(parent);
	clicked = FALSE;
	m_utente = user;
	m_sizer = new wxBoxSizer(wxVERTICAL);
	
	//preparo l'immagine utente
	wxString filepath = wxT("C:\\Users\\ander\\Desktop\\Progetto_PDS\\user_default.png");
	wxPNGHandler *handler = new wxPNGHandler();
	//wxJPEGHandler *handler = new wxJPEGHandler();
	wxImage::AddHandler(handler);
	wxImage *img = new wxImage();
	img->LoadFile(filepath, wxBITMAP_TYPE_PNG, -1);
	//img->LoadFile(filepath, wxBITMAP_TYPE_JPEG, -1);
	//creo il pulsante con l'immagine dell'utente
	m_button = new wxBitmapButton(this, wxID_ANY, wxBitmap(img->Scale(70, 70, wxIMAGE_QUALITY_HIGH)), wxDefaultPosition);
	m_sizer->Add(m_button, 0, wxALIGN_CENTER);

	//aggiungo il nome utente al sizer
	m_text = new wxStaticText(this, wxID_ANY, m_utente.getUsername(), wxDefaultPosition, wxDefaultSize, 
											wxALIGN_CENTER_HORIZONTAL);
	m_text->SetFont((m_text->GetFont()).Larger());
	m_sizer->Add(m_text, 1, wxALIGN_CENTER | wxALL, 5);
	
	this->SetSizerAndFit(m_sizer);

	//collego l'evento click alla funzione OnUserClick
	m_button->Bind(wxEVT_BUTTON, &UserSizer::OnUserClick, this);
	
	backgroundColour = m_button->GetBackgroundColour();
}

UserSizer::~UserSizer() {
}

//seleziona o deseleziona utente
void UserSizer::PerformClick() {
	if (clicked == FALSE) {
		clicked = TRUE;
		m_parentWindow->insertUtenteLista(m_utente);
		m_button->SetBackgroundColour(wxT("cyan"));
	}
	else {
		clicked = FALSE;
		m_parentWindow->deleteUtenteLista(m_utente);
		m_button->SetBackgroundColour(backgroundColour);
	}
}

//esegue il click dell'utente
void UserSizer::OnUserClick(wxCommandEvent& event) {
	this->PerformClick();
}