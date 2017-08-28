#pragma once

#include "utente.h"
#include "boost\asio.hpp"
#include "TaskBarIcon.h"
#include "Settings.h"
#include "MainApp.h"

void reciveTCPfile(utente& utenteProprietario, std::string generalPath, MainFrame* mainframe);

