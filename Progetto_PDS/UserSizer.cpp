#include <wx/statbmp.h>

#include "UserSizer.h"
#include "utente.h"
#include "WindowSelectUser.h"

// TODO gestire la dimensione dei nomi in caso sia eccessiva!

UserSizer::UserSizer(wxWindow* parent, Settings* settings, utente& user) : wxWindow(parent, wxID_ANY)
{
	//inizializzo alcuni parametri
	m_parentWindow = dynamic_cast<WindowSelectUser*>(parent);
	m_utente = user;
	m_settings = settings;

	//preparo l'immagine utente
	//wxString filepath = wxT("C:\\Users\\ander\\Desktop\\Progetto_PDS\\user_default.png");
	//**Si deve creare un path "generale" qui

	//wxString filepath = wxT("C:\\Users\\Sergio\\Desktop\\Progetto_PDS\\" + user.getIpAddr() + ".png");

	//wxMessageBox("" + m_settings->getGeneralPath() + "local_image\\" + user.getIpAddr() + ".png", wxT("INFO"), wxOK | wxICON_INFORMATION);
	wxString filepath = wxT("" + m_settings->getGeneralPath() +"local_image\\" + user.getIpAddr() + ".png");
	m_settings->resizeImage(filepath.ToStdString());
	//wxPNGHandler *handler = new wxPNGHandler();
	//wxImage::AddHandler(handler);
	wxImage *img = new wxImage();
	img->LoadFile(filepath, wxBITMAP_TYPE_ANY, -1);
	//img->LoadFile(filepath, wxBITMAP_TYPE_JPEG, -1);
	//creo il pulsante con l'immagine dell'utente
	m_button = new wxBitmapToggleButton
	(
		this, 
		wxID_ANY, 
		wxBitmap(img->Scale(70, 70, wxIMAGE_QUALITY_HIGH)),
		wxDefaultPosition,
		wxDefaultSize
	);
	
	//wxImage::AddHandler(new wxPNGHandler);
	//wxBitmap bitmap;

	
	//bitmap.LoadFile(filepath, wxBITMAP_TYPE_PNG);
	//bitmap.LoadFile(m_settings->getImagePath(), wxBITMAP_TYPE_PNG);


	//m_button = new wxBitmapButton(this, wxID_ANY, bitmap, wxDefaultPosition);
	wxBoxSizer* sizerTop = new wxBoxSizer(wxVERTICAL);
	
	sizerTop->Add(m_button, 0, wxALIGN_CENTER);

	//aggiungo il nome utente al sizer
	m_text = new wxStaticText
	(
		this,
		wxID_ANY, 
		m_utente.getUsername(), 
		wxDefaultPosition, 
		wxDefaultSize, 
		wxALIGN_CENTER_HORIZONTAL | wxELLIPSIZE_END
	);
	m_text->SetFont((m_text->GetFont()).Larger());
	sizerTop->Add(m_text, 1, wxALIGN_CENTER | wxALL, 5);
	sizerTop->SetDimension(wxDefaultPosition, wxSize(90, 90));
	this->SetSizerAndFit(sizerTop);
	
	//collego l'evento click alla funzione OnUserClick
	m_button->Bind(wxEVT_TOGGLEBUTTON, &UserSizer::OnUserClick, this);
}

UserSizer::~UserSizer() {
}

//seleziona o deseleziona utente
void UserSizer::PerformClick() {
	if (m_button->GetValue())
		m_parentWindow->insertUtenteLista(m_utente);
	else
		m_parentWindow->deleteUtenteLista(m_utente);
}

//esegue il click dell'utente
void UserSizer::OnUserClick(wxCommandEvent& event) {
	this->PerformClick();
}