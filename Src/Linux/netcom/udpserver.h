//
// v.1
// udp server, receive 기능
//
// v.2
// close(), waitforsingleobject
// isConnect()
// add variable m_isReceivData
// thread restart bug fix
//
// 2016-02-09
//		- linux용 작업
//
// 2016-09-01
//		- linux용 작업2
//
#pragma once

#include "network.h"


namespace network
{

	class cUDPServer
	{
	public:
		cUDPServer();
		virtual ~cUDPServer();

		bool Init(const int id, const int port);
		int GetRecvData(OUT BYTE *dst, const int maxSize);
		void SetMaxBufferLength(const int length);
		void Close(const bool isWait = false);
		bool IsConnect() const;


	public:
		int m_id;
		SOCKET m_socket;
		int m_port;
		bool m_isConnect;
		int m_maxBuffLen;
		cPacketQueue m_recvQueue;

		pthread_t m_handle;
		bool m_threadLoop;
		int m_sleepMillis; // default = 1
	};


	inline bool cUDPServer::IsConnect() const { return m_isConnect; }
}
