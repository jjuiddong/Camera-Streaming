#pragma once


namespace network
{

	bool LaunchTCPClient(const std::string &ip, const int port, OUT SOCKET &out);
	bool LaunchUDPClient(const std::string &ip, const int port, OUT SOCKADDR_IN &sockAddr, OUT SOCKET &out);

	bool LaunchTCPServer(const int port, OUT SOCKET &out);
	bool LaunchUDPServer(const int port, OUT SOCKET &out);
}
