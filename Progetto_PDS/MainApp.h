#pragma once

#include "wx/wxprec.h"
#include "wx/wx.h"
#include <wx/cmdline.h>

#include "settings.h"

class MainApp : public wxApp
{
private:
	std::recursive_mutex rm_frame;  //1
	std::recursive_mutex rm_settings;//2

public:

	bool OnInit();
	void OnInitCmdLine(wxCmdLineParser& parser);
	int OnExit();

	class MainFrame* GetFrame() { 
		std::lock_guard<std::recursive_mutex> lk_frame(rm_frame);
		return m_frame; 
	}
	class Settings* GetSettings() { 
		std::lock_guard<std::recursive_mutex> lk_settings(rm_settings);
		return m_settings; }

	void setFrame(class MainFrame* mainframe) {
		std::lock_guard<std::recursive_mutex> lk_frame(rm_frame);
		this->m_frame = mainframe;
	}

	void setSettings(class Settings* settings) {
		std::lock_guard<std::recursive_mutex> lk_settings(rm_settings);
		this->m_settings = settings;
	}
	
protected:
	class MainFrame* m_frame;
	Settings* m_settings;
};


DECLARE_APP(MainApp)
