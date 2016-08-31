//
// 2016-02-06, jjuiddong
//
// gcc용 tcp,udp server, client socket 생성 함수
//
#pragma once

#include "network.h"

namespace network
{
	bool LaunchClient(const std::string &ip, const int port, OUT SOCKET &out);
	bool LaunchUDPClient(const std::string &ip, const int port, OUT sockaddr_in &sockAddr, OUT SOCKET &out);

	bool LaunchServer(const int port, OUT SOCKET &out);
	bool LaunchUDPServer(const int port, OUT SOCKET &out);
}
