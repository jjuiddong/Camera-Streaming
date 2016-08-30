//
// v.1
// udp client, send, receive 기능
//
// v.2
// close(), waitforsingleobject
// isConnect()
// thread restart bug fix
//
// 2016-08-30
//		- packetqueue 추가
//
#pragma once


namespace network
{

	class cUDPClient
	{
	public:
		cUDPClient();
		virtual ~cUDPClient();

		bool Init(const string &ip, const int port, const int sleepMillis=30);
		void SendData(const BYTE *buff, const int buffLen);
		void SetMaxBufferLength(const int length);
		void Close();
		bool IsConnect() const;


	public:
		string m_ip;
		int m_port;

		bool m_isConnect;
		SOCKET m_socket;
		sockaddr_in m_sockaddr;
		int m_maxBuffLen;
		cPacketQueue m_sndQueue;

		std::thread m_thread;
		CriticalSection m_sndCriticalSection;
		bool m_threadLoop;
		int m_sleepMillis; // Sleep(m_sleepMillis)
	};


	inline bool cUDPClient::IsConnect() const { return m_isConnect; }

}
