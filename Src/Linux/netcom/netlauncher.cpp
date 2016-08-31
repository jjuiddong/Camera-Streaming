
#include "netlauncher.h"

using namespace network;


//------------------------------------------------------------------------
// 
//------------------------------------------------------------------------
bool	network::LaunchClient(const std::string &ip, const int port, OUT SOCKET &out)
{
	// TCP/IP 스트림 소켓을 생성합니다.
	// socket(주소 계열, 소켓 형태, 프로토콜
	SOCKET ssock = socket(AF_INET, SOCK_STREAM, 0);
	if (ssock == INVALID_SOCKET)
	{
		//clog::Error( clog::ERROR_CRITICAL, "socket() error\n" );
		return false;
	}

	// 주소 구조체를 채웁니다.
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
	server_addr.sin_port = htons(port);
	int clen = sizeof(server_addr);

	// 서버로 접속합니다
	// connect(소켓, 서버 주소, 서버 주소의 길이
	int nRet = connect(ssock, (struct sockaddr*)&server_addr, clen);
	if (nRet == SOCKET_ERROR)
	{
		//clog::Error( clog::ERROR_CRITICAL, "connect() error ip=%s, port=%d\n", ip.c_str(), port );
		close(ssock);
		return false;
	}

	out = ssock;

	return true;
}



//------------------------------------------------------------------------
// 서버 시작
//------------------------------------------------------------------------
bool network::LaunchServer(const int port, OUT SOCKET &out)
{
	int ssock;
	if ((ssock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		//perror("socket()");
		return false;
	}

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

	if (bind(ssock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		//perror("bind()");
		return false;
	}

	if (listen(ssock, 8) < 0)
	{
		//perror("listen()");
		return false;
	}

	char szBuf[256];
	int nRet = gethostname(szBuf, sizeof(szBuf));
	if (nRet == SOCKET_ERROR)
	{
		//clog::Error( clog::ERROR_CRITICAL, "gethostname() error\n" );
		close(ssock);
		return false;
	}

	out = ssock;

	return true;
}



//------------------------------------------------------------------------
// 서버 시작
//------------------------------------------------------------------------
bool network::LaunchUDPServer(const int port, OUT SOCKET &out)
{
	// UDP/IP 스트림 소켓을 생성합니다.
	// socket(주소 계열, 소켓 형태, 프로토콜
	SOCKET ssock = socket(AF_INET, SOCK_DGRAM, 0);
	if (ssock == INVALID_SOCKET)
	{
		//clog::Error( clog::ERROR_CRITICAL, "socket() error\n" );
		return false;
	}

	// 주소 구조체를 채웁니다.
	struct sockaddr_in serverAddr;
	bzero(&serverAddr, sizeof(sockaddr_in));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(port);
	bind(ssock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

	out = ssock;

	return true;
}



//------------------------------------------------------------------------
// 
//------------------------------------------------------------------------
bool network::LaunchUDPClient(const std::string &ip, const int port, OUT sockaddr_in &sockAddr, OUT SOCKET &out)
{
	// UDP/IP 스트림 소켓을 생성합니다.
	// socket(주소 계열, 소켓 형태, 프로토콜
	SOCKET ssock = socket(AF_INET, SOCK_DGRAM, 0);
	if (ssock == INVALID_SOCKET)
	{
		//clog::Error( clog::ERROR_CRITICAL, "socket() error\n" );
		return false;
	}

	// 주소 구조체를 채웁니다.
	bzero(&sockAddr, sizeof(sockaddr_in));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = inet_addr(ip.c_str());
	sockAddr.sin_port = htons(port);
	int clen = sizeof(sockaddr_in);

	// 서버로 접속합니다
	// connect(소켓, 서버 주소, 서버 주소의 길이
	int nRet = connect(ssock, (struct sockaddr*)&sockAddr, clen);
	if (nRet == SOCKET_ERROR)
	{
		//clog::Error( clog::ERROR_CRITICAL, "connect() error ip=%s, port=%d\n", ip.c_str(), port );
		close(ssock);
		return false;
	}

	out = ssock;

	return true;
}
