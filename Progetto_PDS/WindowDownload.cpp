#include <wx/statbmp.h>

#include "share_icon.xpm"

#include "WindowDownload.h"
#include "utente.h"

wxBEGIN_EVENT_TABLE(FileInDownload, wxWindow)
EVT_THREAD(SetMaxDim_EVENT, FileInDownload::OnSetMaxDim)
EVT_THREAD(Inc_EVENT, FileInDownload::OnIncFile)
wxEND_EVENT_TABLE()
//
FileInDownload::FileInDownload(wxWindow* parent, wxWindowID id, std::string user, std::string file, std::string generalPath) :
	wxWindow(parent, id, wxDefaultPosition, wxDefaultSize, wxNO_FULL_REPAINT_ON_RESIZE)
{
	flagAbort.store(false);
	m_parentWindow = dynamic_cast<WindowDownload*>(parent);
	m_perc = new wxStaticText(this, wxID_ANY, "0   ", wxDefaultPosition, wxDefaultSize);
	
	wxBitmap* cancel = new wxBitmap();
	cancel->LoadFile(generalPath + "bottoni\\annulla.png", wxBITMAP_TYPE_ANY);
	wxBitmap* cancel_hover = new wxBitmap();
	cancel_hover->LoadFile(generalPath + "bottoni\\annulla_hover.png", wxBITMAP_TYPE_ANY);
	m_abort = new wxBitmapButton(this, wxID_CANCEL, *cancel, wxDefaultPosition, wxDefaultSize);
	m_abort->SetWindowStyle(wxNO_BORDER);
	m_abort->SetBackgroundColour(this->GetBackgroundColour());
	m_abort->SetBitmapHover(*cancel_hover);


	wxFlexGridSizer* sizerTop = new wxFlexGridSizer(4);
	wxBoxSizer* percSizer = new wxBoxSizer(wxHORIZONTAL);

	percSizer->Add(m_perc);
	percSizer->Add(new wxStaticText
	(
		this,
		wxID_ANY,
		"%",
		wxDefaultPosition,
		wxDefaultSize
	), 0, wxLEFT, 5);

	sizerTop->Add(new wxStaticText
	(
		this,
		wxID_ANY,
		user,
		wxDefaultPosition,
		wxSize(100, 50)
	), 1, wxALIGN_CENTER | wxALL, 5);
	sizerTop->Add(new wxStaticText
	(
		this,
		wxID_ANY,
		file,
		wxDefaultPosition,
		wxSize(100, 50)
	), 1, wxALIGN_CENTER | wxALL, 5);
	sizerTop->Add(percSizer, 1, wxALIGN_CENTER | wxALL, 5);
	sizerTop->Add(m_abort, 0, wxALIGN_CENTER | wxALL, 5);


	m_abort->Bind(wxEVT_BUTTON, &FileInDownload::OnAbortClick, this);

	this->SetSizerAndFit(sizerTop);
}

//interrompe trasferimento
void FileInDownload::OnAbortClick(wxCommandEvent& event) {
	flagAbort.store(true);
}

void FileInDownload::SetMaxDim(int dim) {
	m_tot = dim;
	m_parziale = 0;
}

void FileInDownload::IncFile(int dim) {
	int diff = dim - m_parziale;
	m_parziale = dim;
	int value = m_parziale * 100 / m_tot;
	int intval = (int)floor(value + 0.5);
	
	if (m_perc->GetLabelText() != std::to_string(intval))
		m_perc->SetLabelText(std::to_string(intval));

	Update();
}





/*--------------------------------------------------------------------------------------------------------------------*/




wxBEGIN_EVENT_TABLE(WindowDownload, wxFrame)
EVT_BUTTON(wxID_OK, WindowDownload::OnOk)
EVT_ICONIZE(WindowDownload::OnMinimize)
EVT_THREAD(SERVER_EVENT, WindowDownload::OnEndDownload)
wxEND_EVENT_TABLE()

WindowDownload::WindowDownload(wxWindow* parent, Settings* settings)
	: wxFrame(parent, wxID_ANY, wxT("Download in corso"), wxDefaultPosition, wxDefaultSize, wxMINIMIZE_BOX | wxSYSTEM_MENU | wxCAPTION | wxCLIP_CHILDREN)
{
	this->SetBackgroundColour(wxColour(240, 242, 245));
	this->SetFont(this->GetFont().Bold().Scale(0.9f));
	this->SetIcon(wxIcon(share_icon));
	
	m_settings = settings;
	m_MappaDownload = std::map<std::string, FileInDownload*>();
	
	//Button ok
	wxBitmap* ok = new wxBitmap();
	ok->LoadFile(m_settings->getGeneralPath() + "bottoni\\esci.png", wxBITMAP_TYPE_ANY);
	wxBitmap* ok_hover = new wxBitmap();
	ok_hover->LoadFile(m_settings->getGeneralPath() + "bottoni\\esci_hover.png", wxBITMAP_TYPE_ANY);
	m_ok = new wxBitmapButton(this, wxID_OK, *ok, wxDefaultPosition, wxDefaultSize);
	m_ok->SetWindowStyle(wxNO_BORDER);
	m_ok->SetBackgroundColour(this->GetBackgroundColour());
	m_ok->SetBitmapHover(*ok_hover);

		
	wxSizer* sizerTop = new wxBoxSizer(wxVERTICAL);
	wxSizer* sizerIntestazione = new wxBoxSizer(wxHORIZONTAL);
	m_sizerDownload = new wxBoxSizer(wxVERTICAL);

	sizerIntestazione->Add(new wxStaticText
	(
		this,
		wxID_ANY,
		"NOME FILE",
		wxDefaultPosition,
		wxSize(100, 50)
	), 1, wxALIGN_CENTER | wxALL, 5);
	sizerIntestazione->Add(new wxStaticText
	(
		this,
		wxID_ANY,
		"UTENTE",
		wxDefaultPosition,
		wxSize(100, 50)
	), 1, wxALIGN_CENTER | wxALL, 5);
	sizerIntestazione->Add(new wxStaticText
	(
		this,
		wxID_ANY,
		"AVANZAMENTO",
		wxDefaultPosition,
		wxSize(100, 50)
	), 1, wxALIGN_CENTER | wxALL, 5);

	m_sizerDownload->Add(new wxStaticText
	(
		this,
		wxID_ANY,
		"Nessun download in corso",
		wxDefaultPosition,
		wxDefaultSize
	), 1, wxALIGN_CENTER | wxALL, 5);

	sizerTop->Add(sizerIntestazione);
	sizerTop->Add(m_sizerDownload, 1, wxALL, 5);
	sizerTop->AddStretchSpacer()->SetMinSize(200, 20);
	sizerTop->Add(m_ok, 1, wxALIGN_CENTER | wxALL, 5);

	this->SetSizerAndFit(sizerTop);
}

void WindowDownload::OnOk(wxCommandEvent& event) {
	Show(false);
}

void WindowDownload::OnMinimize(wxIconizeEvent& event) {
	Iconize(false);
	Show(false);
}

//void OnSetTimeFile(wxThreadEvent& event) { SetTimeFile(event.GetPayload<long>()); };

//void WindowDownload::OnNewDownload(wxThreadEvent& event) {
//	FileInDownload* fp = event.GetPayload<FileInDownload*>();
//	std::string ip = event.GetString();
//	m_MappaDownload.insert(std::pair<std::string, FileInDownload*>(ip, fp));
//}

FileInDownload* WindowDownload::newDownload(std::string user, std::string file) {
	if (m_counter++ == 0)
		m_sizerDownload->Clear();

	FileInDownload* fp = new FileInDownload(this, wxID_ANY, user, file, m_settings->getGeneralPath());
	m_sizerDownload->Add(fp, 1, wxALL | 5);

	Layout();
	Fit();

	return fp;
}

void WindowDownload::OnEndDownload(wxThreadEvent& event) {
	FileInDownload* tmp = event.GetPayload<FileInDownload*>();
	tmp->Destroy();

	if (--m_counter == 0) 
		m_sizerDownload->Add(new wxStaticText
		(
			this,
			wxID_ANY,
			"Nessun download in corso",
			wxDefaultPosition,
			wxDefaultSize
		), 1, wxALIGN_CENTER | wxALL, 5);

	Layout();
	Fit();
}

//void WindowSelectUser::deleteUtenteLista(utente user) {
//	m_MappaInvio.erase(user.getIpAddr());
//	if (m_MappaInvio.size() == 0)
//		m_ok->Disable();
//}
//
//void WindowSelectUser::addUtente(utente user) {
//	UserSizer *u = new UserSizer(this, m_settings, user);
//	m_ListaUtenti.push_back(u);
//	m_sizerUsers->Add(u, 1, wxALIGN_CENTER);
//
//	Update();
//}
//
//void WindowSelectUser::removeUtente(utente user) {
//	int i = 0;
//	std::string ip = user.getIpAddr();
//
//	for (auto it = m_ListaUtenti.begin(); it != m_ListaUtenti.end(); it++) {
//		if ((*it)->getIpAddr() == ip) {
//			this->m_ListaUtenti.erase(it);
//			m_sizerUsers->Remove(i);
//			break;
//		}
//		i++;
//	}
//
//	Update();
//}
//
//std::vector<utente> WindowSelectUser::getListaInvio()
//{
//	std::vector<utente> lista;
//	for (auto it : m_MappaInvio)
//		lista.push_back(it.second);
//
//	return lista;
//}