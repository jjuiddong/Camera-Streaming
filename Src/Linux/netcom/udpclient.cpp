
#include "udpclient.h"
#include "netlauncher.h"

using namespace std;
using namespace network;

void* UDPClientThreadFunction(void* arg);


cUDPClient::cUDPClient() 
	: m_isConnect(false)
	, m_socket(INVALID_SOCKET)
	, m_maxBuffLen(BUFFER_LENGTH)
	, m_handle(0)
	, m_threadLoop(true)
	, m_sleepMillis(30)
{
}

cUDPClient::~cUDPClient()
{
	Close();
}


// UDP 클라언트 생성, ip, port 에 접속을 시도한다.
bool cUDPClient::Init(const string &ip, const int port, const int sleepMillis) //sleepMillis=30
{
	Close();

	m_ip = ip;
	m_port = port;
	m_sleepMillis = sleepMillis;

	if (network::LaunchUDPClient(ip, port, m_sockaddr, m_socket))
	{
		cout << "Connect UDP Client ip= " << ip << ", port= " << port << endl;

		if (!m_sndQueue.Init(m_maxBuffLen, 512))
		{
			Close();
			return false;
		}

		m_isConnect = true;
		m_threadLoop = true;
		if (!m_handle)
		{
			pthread_create(&m_handle, NULL, UDPClientThreadFunction, this);
		}
	}
	else
	{
		cout << "Error!! Connect UDP Client ip=" << ip << ", port=" << port << endl;
		return false;
	}

	return true;
}


// 전송할 정보를 설정한다.
void cUDPClient::SendData(const BYTE *buff, const int buffLen)
{
	m_sndQueue.Push(m_socket, buff, buffLen);
}


void cUDPClient::SetMaxBufferLength(const int length)
{
	m_maxBuffLen = length;
}


// 접속을 끊는다.
void cUDPClient::Close()
{
	m_isConnect = false;
	m_threadLoop = false;
	if (m_handle)
	{
		pthread_join(m_handle, NULL);
		m_handle = 0;
	}

	if (m_socket != INVALID_SOCKET)
	{
		close(m_socket);
		m_socket = INVALID_SOCKET;
	}
}


// UDP 네트워크 쓰레드.
void* UDPClientThreadFunction(void* arg)
{
	cUDPClient *udp = (cUDPClient*)arg;

	while (udp->m_threadLoop)
	{
		if (!udp->m_isConnect || (INVALID_SOCKET == udp->m_socket))
		{
			continue;
		}

		udp->m_sndQueue.SendAll(udp->m_sockaddr);
	}

	return 0;
}
