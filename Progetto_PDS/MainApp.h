#pragma once

#include "wx/wxprec.h"
#include "wx/wx.h"
#include <wx/cmdline.h>

#include "settings.h"

class MainApp : public wxApp
{
public:

	bool OnInit();
	void OnInitCmdLine(wxCmdLineParser& parser);
	int OnExit();

	class MainFrame* GetFrame() { return m_frame; }
	class Settings* GetSettings() { return m_settings; }
protected:
	class MainFrame* m_frame;
	Settings* m_settings;
};


DECLARE_APP(MainApp)
