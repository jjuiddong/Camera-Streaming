
#include "stdafx.h"
#include "udpserver.h"

using namespace network;

unsigned WINAPI UDPServerThreadFunction(void* arg);


cUDPServer::cUDPServer() 
	: m_id(0)
	, m_isConnect(false)
	, m_threadLoop(true)
	, m_maxBuffLen(BUFFER_LENGTH)
	, m_sleepMillis(1)
{
}

cUDPServer::~cUDPServer()
{
	Close(true);
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
		dbg::Log("Bind UDP Server port = %d \n", port);

		if (network::LaunchUDPServer(port, m_socket))
		{
			if (!m_recvQueue.Init(m_maxBuffLen, 512))
			{
				Close();
				return false;
			}

			m_threadLoop = false;
			if (m_thread.joinable())
				m_thread.join();

			m_isConnect = true;
			m_threadLoop = true;
			m_thread = std::thread(UDPServerThreadFunction, this);
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

	const int len = std::min(maxSize, sb.actualLen);
	memcpy(dst, sb.buffer, len);
	m_recvQueue.Pop();
	return len;
}


void cUDPServer::Close(const bool isWait) // isWait = false
{
	m_threadLoop = false;
	if (isWait && m_thread.joinable())
		m_thread.join();

	m_isConnect = false;
	closesocket(m_socket);
}


void cUDPServer::SetMaxBufferLength(const int length)
{
	m_maxBuffLen = length;
}


// UDP 서버 쓰레드
unsigned WINAPI UDPServerThreadFunction(void* arg)
{
	cUDPServer *udp = (cUDPServer*)arg;

	BYTE *buff = new BYTE[udp->m_maxBuffLen];
	ZeroMemory(buff, udp->m_maxBuffLen);

	while (udp->m_threadLoop)
	{
		const timeval t = {0, 1}; // ? millisecond
		fd_set readSockets;
		FD_ZERO(&readSockets);
		FD_SET(udp->m_socket, &readSockets);

		const int ret = select(readSockets.fd_count, &readSockets, NULL, NULL, &t);
		if (ret != 0 && ret != SOCKET_ERROR)
		{
			const int result = recv(readSockets.fd_array[0], (char*)buff, udp->m_maxBuffLen, 0);
			if (result == SOCKET_ERROR || result == 0) // 받은 패킷사이즈가 0이면 서버와 접속이 끊겼다는 의미다.
			{
				// 에러가 발생하더라도, 수신 대기상태로 계속 둔다.
			}
			else
			{
				udp->m_recvQueue.Push(readSockets.fd_array[0], buff, result, true);
			}
		}
		
		if (udp->m_sleepMillis)
			std::this_thread::sleep_for(std::chrono::microseconds(udp->m_sleepMillis));
	}

	delete[] buff;
	return 0;
}
