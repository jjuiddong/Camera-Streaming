#pragma once


#ifndef _CRT_SECURE_NO_WARNINGS
	#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS 
	#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#endif

#ifndef _WINSOCKAPI_
	#define _WINSOCKAPI_
#endif

#ifndef _WINSOCK2API_
#include <winsock2.h>
#include <windows.h>
#endif


#include <string>
#include <algorithm>
#include <mmsystem.h>
using std::string;

#pragma comment( lib, "wsock32.lib" )
#pragma comment(lib, "Ws2_32.lib")



namespace network {
	enum {
		BUFFER_LENGTH = 1024,
	};

// 	struct sFD_Set : fd_set {
// 		SOCKET socket_array[FD_SETSIZE];		// fd_array[] 소켓에 해당하는 netid 값
// 	};

}


#include "launcher.h"
#include "protocol.h"
#include "packetqueue.h"
#include "udpclient.h"
#include "udpserver.h"
#include "tcpserver.h"
#include "tcpclient.h"
