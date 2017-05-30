#pragma once

#include "wx/ipc.h"

// the default service name
#define IPC_SERVICE "4242"
//#define IPC_SERVICE wxT("/tmp/wxsrv424")

// the hostname
#define IPC_HOST "localhost"

// the IPC topic
#define IPC_TOPIC "IPC server"

// the name of the item we're being advised about
#define IPC_ADVISE_NAME "Item"