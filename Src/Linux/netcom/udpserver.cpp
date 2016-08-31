
#include "udpserver.h"
#include "netlauncher.h"

using namespace std;
using namespace network;

void* UDPServerThreadFunction(void* arg);


cUDPServer::cUDPServer()
	: m_id(0)
	, m_socket(INVALID_SOCKET)
	, m_isConnect(false)
	, m_maxBuffLen(BUFFER_LENGTH)
	, m_threadLoop(true)
	, m_sleepMillis(1)
{
}

cUDPServer::~cUDPServer()
{
	Close();
}


bool cUDPServer::Init(const int id, const int port)
{
	m_id = id;
	m_port = port;

	if (m_isConnect)
	{
		Close();
	}
	else
	{
		cout << "Bind UDP Server port = " << port << endl;

		if (network::LaunchUDPServer(port, m_socket))
		{
			if (!m_recvQueue.Init(m_maxBuffLen, 512))
			{
				Close();
				return false;
			}

			m_isConnect = true;
			m_threadLoop = true;
			if (!m_handle)
			{
				pthread_create(&m_handle, NULL, UDPServerThreadFunction, this);
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}


// 받은 패킷을 dst에 저장해서 리턴한다.
// 동기화 처리.
int cUDPServer::GetRecvData(OUT BYTE *dst, const int maxSize)
{
	network::sSockBuffer sb;
	if (!m_recvQueue.Front(sb))
		return 0;

	const int len = min(maxSize, sb.actualLen);
	memcpy(dst, sb.buffer, len);
	m_recvQueue.Pop();
	return len;
}


void cUDPServer::Close(const bool isWait) // isWait = false
{
	m_isConnect = false;
	m_threadLoop = false;
	if (isWait && m_handle)
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


void cUDPServer::SetMaxBufferLength(const int length)
{
	m_maxBuffLen = length;
}


// UDP 서버 쓰레드
void* UDPServerThreadFunction(void* arg)
{
	cUDPServer *udp = (cUDPServer*)arg;

	BYTE *buff = new BYTE[udp->m_maxBuffLen];
	memset(buff, 0, udp->m_maxBuffLen);

	while (udp->m_threadLoop)
	{
		timeval t = { 0, udp->m_sleepMillis }; // ? millisecond
		fd_array readSockets;
		FD_ZERO(&readSockets);
		FD_SET(udp->m_socket, &readSockets);
		readSockets.fd_array[0] = udp->m_socket;
		const int maxfd = udp->m_socket + 1;

		const int ret = select(maxfd, &readSockets, NULL, NULL, &t);
		if (ret != 0 && ret != SOCKET_ERROR)
		{
			const int result = recv(readSockets.fd_array[0], buff, udp->m_maxBuffLen, 0);
			if (result == SOCKET_ERROR || result == 0) // 받은 패킷사이즈가 0이면 서버와 접속이 끊겼다는 의미다.
			{
				// 에러가 발생하더라도, 수신 대기상태로 계속 둔다.
			}
			else
			{
				udp->m_recvQueue.Push(readSockets.fd_array[0], buff, result, true);
			}

			//cout << "recv = " << result << ", maxbufflen=" << udp->m_maxBuffLen << endl;
		}
	}

	SAFE_DELETEA(buff);
	return 0;
}
