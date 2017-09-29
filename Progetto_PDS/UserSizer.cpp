#include <wx/statbmp.h>

#include "UserSizer.h"
#include "utente.h"
#include "WindowSelectUser.h"


UserSizer::UserSizer(wxWindow* parent, Settings* settings, utente& user) : wxWindow(parent, wxID_ANY)
{
	this->SetMaxSize(wxSize(90, 120));
	m_parentWindow = dynamic_cast<WindowSelectUser*>(parent);
	m_utente = user;
	m_settings = settings;

	wxString filepath = wxT("" + m_settings->getGeneralPath() +"local_image\\" + user.getIpAddr() + ".png");
	m_settings->resizeImage(filepath.ToStdString());

	wxImage *img = new wxImage();
	img->LoadFile(filepath, wxBITMAP_TYPE_ANY, -1);

	m_button = new wxBitmapToggleButton
	(
		this, 
		wxID_ANY, 
		wxBitmap(img->Scale(70, 70, wxIMAGE_QUALITY_HIGH)),
		wxDefaultPosition,
		wxDefaultSize
	);
	
	m_text = new wxStaticText
	(
		this,
		wxID_ANY, 
		m_utente.getUsername(), 
		wxDefaultPosition, 
		wxDefaultSize, 
		wxALIGN_CENTER_HORIZONTAL | wxST_ELLIPSIZE_END
	);
	m_text->SetFont((m_text->GetFont()).Larger());
	m_text->SetToolTip(m_utente.getUsername());


	wxBoxSizer* sizerTop = new wxBoxSizer(wxVERTICAL);

	sizerTop->Add(m_button, 0, wxALIGN_CENTER);
	sizerTop->Add(m_text, 1, wxALIGN_CENTER | wxALL, 5);

	this->SetSizerAndFit(sizerTop);
	
	//collego l'evento click alla funzione OnUserClick
	m_button->Bind(wxEVT_TOGGLEBUTTON, &UserSizer::OnUserClick, this);
}

void UserSizer::OnUserClick(wxCommandEvent& event) {
	if (m_button->GetValue())
		m_parentWindow->insertUtenteLista(m_utente);
	else {
		m_parentWindow->deleteUtenteLista(m_utente);
		m_parentWindow->SetFocus();
	}
		
}