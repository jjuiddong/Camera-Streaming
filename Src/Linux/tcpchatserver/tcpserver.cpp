
#include "tcpserver.h"
#include "netlauncher.h"

using namespace std;
using namespace network;

void* TCPServerThreadFunction(void* arg);


cTCPServer::cTCPServer()
	: m_isConnect(false)
	, m_maxBuffLen(BUFFER_LENGTH)
	, m_handle(0)
	, m_threadLoop(true)
	, m_sleepMillis(10)
{
	pthread_mutex_init(&m_criticalSection, NULL);
}

cTCPServer::~cTCPServer()
{
	Close();

	pthread_mutex_destroy(&m_criticalSection);
}


bool cTCPServer::Init(const int port, const int packetSize, const int maxPacketCount, const int sleepMillis)
// packetSize=512, maxPacketCout=10, sleepMillis=30
{
	Close();

	m_port = port;
	m_sleepMillis = sleepMillis;
	m_maxBuffLen = packetSize;

	if (network::LaunchServer(port, m_svrSocket))
	{
		cout << "Bind TCP Server " << "port = " << port << endl;

		if (!m_recvQueue.Init(packetSize, maxPacketCount))
		{
			Close();
			return false;
		}

		if (!m_sendQueue.Init(packetSize, maxPacketCount))
		{
			Close();
			return false;
		}

		m_isConnect = true;
		m_threadLoop = true;
 		if (!m_handle)
 		{
// 			m_handle = (HANDLE)_beginthreadex(NULL, 0, TCPServerThreadFunction, this, 0, (unsigned*)&m_threadId);
			pthread_create(&m_handle, NULL, TCPServerThreadFunction, this);
 		}
	}
	else
	{
		cout << "Error!! Bind TCP Server port=" << port << endl;
		return false;
	}

	return true;
}


void cTCPServer::Close()
{
	m_isConnect = false;
	m_threadLoop = false;
 	if (m_handle)
 	{
//		::WaitForSingleObject(m_handle, 1000);
		pthread_join(m_handle, NULL);
 		m_handle = 0;
 	}
	close(m_svrSocket);
	m_svrSocket = INVALID_SOCKET;
}


int cTCPServer::MakeFdSet(OUT fd_array &out)
{
	int maxfd = 0;
	FD_ZERO(&out);
	out.fd_count = 0;
	for (auto &session : m_sessions)
	{
		FD_SET(session.socket, (fd_set*)&out);
		out.fd_array[out.fd_count++] = session.socket;
		if (maxfd < session.socket)
			maxfd = session.socket;
	}

	return maxfd+1;
}


bool cTCPServer::AddSession(const SOCKET remoteSock)
{
	cAutoCS cs(m_criticalSection);

	for (auto &session : m_sessions)
	{
		if (remoteSock == session.socket)
			return false; // 이미 있다면 종료.
	}

	sSession session;
	session.state = SESSION_STATE::LOGIN_WAIT;
	session.socket = remoteSock;
	m_sessions.push_back(session);
	return true;
}


void cTCPServer::RemoveSession(const SOCKET remoteSock)
{
	cAutoCS cs(m_criticalSection);

	for (u_int i = 0; i < m_sessions.size(); ++i)
	{
		if (remoteSock == m_sessions[i].socket)
		{
			close(m_sessions[i].socket);
			common::rotatepopvector(m_sessions, i);
			break;
		}
	}
}


 void* TCPServerThreadFunction(void* arg)
 {
 	cTCPServer *server = (cTCPServer*)arg;
 
 	char *buff = new char[server->m_maxBuffLen];
 	const int maxBuffLen = server->m_maxBuffLen;
 
	int lastAcceptTime = GetTickCount();

 	while (server->m_threadLoop)
 	{
		struct timeval t = { 0, server->m_sleepMillis };

		// Accept는 가끔씩 처리한다.
		const int curT = GetTickCount();
		if (curT - lastAcceptTime > 300)
		{
			lastAcceptTime = curT;

			//-----------------------------------------------------------------------------------
			// Accept Client
			fd_set acceptSockets;
			FD_ZERO(&acceptSockets);
			FD_SET(server->m_svrSocket, &acceptSockets);
			const int maxfd = server->m_svrSocket + 1;
			const int ret1 = select(maxfd, &acceptSockets, NULL, NULL, &t);
			if (ret1 != 0 && ret1 != SOCKET_ERROR)
			{
				// accept(요청을 받으 소켓, 선택 클라이언트 주소)
				struct sockaddr_in client_addr;
				socklen_t clen = sizeof(struct sockaddr_in);
				SOCKET remoteSocket = accept(server->m_svrSocket, (struct sockaddr*)&client_addr, &clen);
				if (remoteSocket == INVALID_SOCKET)
				{
					//clog::Error(clog::ERROR_CRITICAL, "Client를 Accept하는 도중에 에러가 발생함\n");
					continue;
				}
				
				server->AddSession(remoteSocket);
			}
			//-----------------------------------------------------------------------------------
		}


		//-----------------------------------------------------------------------------------
		// Receive Packet
		fd_array readSockets;
		const int maxfd = server->MakeFdSet(readSockets);
		const fd_array sockets = readSockets;

		const int ret = select(maxfd, &readSockets, NULL, NULL, &t);
		if (ret != 0 && ret != SOCKET_ERROR)
		{
			for (int i = 0; i < sockets.fd_count; ++i)
			{
				if (!FD_ISSET(sockets.fd_array[i], &readSockets))
					continue;

				const int result = recv(sockets.fd_array[i], buff, maxBuffLen, 0);
				if (result == SOCKET_ERROR || result == 0) // 받은 패킷사이즈가 0이면 서버와 접속이 끊겼다는 의미다.
				{
					server->RemoveSession(sockets.fd_array[i]);
				}
				else
				{
// 					cout << "packet recv from = " << sockets.fd_array[i] << endl;
// 					cout << "packet recv size = " << result << endl;
// 					cout << "packet data = " << buff << endl;
					server->m_recvQueue.Push(sockets.fd_array[i], (BYTE*)buff, result, true);
				}
			}
		}
		//-----------------------------------------------------------------------------------


		//-----------------------------------------------------------------------------------
		// Send Packet
		server->m_sendQueue.SendAll();
		//-----------------------------------------------------------------------------------
	}

 	delete[] buff;
 	return NULL;
 }
