//
// 2015-11-29, jjuiddong
//
// - 패킷을 저장한다. Thread Safe, 최대한 심플하게 만들었다.
// - 큐가 가득차면, 가장 오래된 패킷을 제거하고, 추가한다.
// - 각 소켓마다 packetSize 크기만큼 채울 때까지 다음으로 넘어가지 않는다.
// - 패킷을 큐에 저장할 때, 헤더(sHeader)가 추가된다.
// - 소켓마다 하나 이상의 큐를 가질 수 있다.
//
// 2016-06-29, jjuiddong
//		- sHeader가 없는 패킷을 받을 수 있게함
//		- Init(),  isIgnoreHeader 플래그 추가
//
// 2016-05-26, jjuiddong
//		- 큰 패킷이 나눠져서 온 경우 sHeader 정보를 토대로, 패킷을 차례차례 쌓는다.
//		- 네트워크로 부터 온 패킷이 2개 이상의 프로토콜을 포함할 경우, 나눠서 처리한다.
//
#pragma once


namespace network
{
	// 한 패킷의 정보를 저장한다.
	// 하나의 패킷이 여러개로 나눠졌을 때, 한 sSockBuffer 에 모두 저장된다.
	struct sSockBuffer
	{
		SOCKET sock; // 세션 소켓
		BYTE *buffer;
		int totalLen;
		bool full; // 버퍼가 다 채워지면 true가 된다.
		int readLen;
		int actualLen; // 실제 패킷의 크기를 나타낸다. totalLen - sizeof(sHeader)
	};

	struct sSession;
	class cPacketQueue
	{
	public:
		cPacketQueue();
		virtual ~cPacketQueue();

		struct sHeader
		{
			BYTE head[2]; // $@
			BYTE protocol; // protocol id
			int length;	// packet length (byte)
		};

		bool Init(const int packetSize, const int maxPacketCount, const bool isIgnoreHeader=false);
		void Push(const SOCKET sock, const BYTE *data, const int len, const bool fromNetwork=false);
		bool Front(OUT sSockBuffer &out);
		void Pop();
		void SendAll();
		void SendAll(const sockaddr_in &sockAddr);
		void SendBroadcast(vector<sSession> &sessions, const bool exceptOwner = true);
		void Lock();
		void Unlock();
		int GetSize();

		vector<sSockBuffer> m_queue;

		// 임시 버퍼
		BYTE *m_tempHeaderBuffer; // 임시로 Header 저장하는 버퍼
		int m_tempHeaderBufferSize;
		bool m_isStoreTempHeaderBuffer; // 임시로 저장하고 있을 때 true
		BYTE *m_tempBuffer; // 임시로 저장될 버퍼
		int m_tempBufferSize;
		bool m_isIgnoreHeader; // sHeader 데이타가 없는 패킷을 받을 때, true


	protected:
		sSockBuffer* FindSockBuffer(const SOCKET sock);
		int CopySockBuffer(sSockBuffer *dst, const BYTE *data, const int len);
		int AddSockBuffer(const SOCKET sock, const BYTE *data, const int len, const bool fromNetwork);


		//---------------------------------------------------------------------
		// Simple Queue Memory Pool
		BYTE* Alloc();
		void Free(BYTE*ptr);
		void ClearMemPool();
		void Clear();

		struct sChunk
		{
			bool used;
			BYTE *p;
		};
		vector<sChunk> m_memPool;
		BYTE *m_memPoolPtr;
		int m_packetBytes;	// 순수한 패킷 크기
		int m_chunkBytes;	// sHeader 헤더를 포함한 패킷 크기
		int m_totalChunkCount;
		CRITICAL_SECTION m_criticalSection;
	};
}
