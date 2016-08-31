
#include "network.h"
#include "session.h"
#include "packetqueue.h"

using namespace network;


cPacketQueue::cPacketQueue()
	: m_tempHeaderBuffer(NULL)
	, m_isStoreTempHeaderBuffer(false)
	, m_tempBuffer(NULL)
	, m_isIgnoreHeader(false)
	, m_memPoolPtr(NULL)
	, m_chunkBytes(0)
	, m_totalChunkCount(0)
{
	pthread_mutex_init(&m_criticalSection, NULL);
}

cPacketQueue::~cPacketQueue()
{
	Clear();

	pthread_mutex_destroy(&m_criticalSection);
}


bool cPacketQueue::Init(const int packetSize, const int maxPacketCount, const bool isIgnoreHeader)
// isIgnoreHeader = false
{
	Clear();

	m_isIgnoreHeader = isIgnoreHeader;

	// Init Memory pool
	m_chunkBytes = packetSize;
	m_totalChunkCount = maxPacketCount;
	m_memPoolPtr = new BYTE[(packetSize + sizeof(sHeader)) * maxPacketCount];
	m_memPool.reserve(maxPacketCount);
	for (int i = 0; i < maxPacketCount; ++i)
		m_memPool.push_back({ false, m_memPoolPtr + (i*(packetSize + sizeof(sHeader))) });
	//

	m_queue.reserve(maxPacketCount);
	m_tempBuffer = new BYTE[packetSize + sizeof(sHeader)];
	m_tempBufferSize = packetSize + sizeof(sHeader);
	m_tempHeaderBuffer = new BYTE[sizeof(sHeader)];
	m_tempHeaderBufferSize = 0;

	return true;
}


// 네트워크로 부터 온 패킷이나, 사용자가 전송할 패킷을 추가할 때, 호출한다.
// 여러개로 나눠진 패킷을 처리할 때도 호출할 수 있다.
// *data가 두개 이상의 패킷을 포함할 때도, 이를 처리한다.
void cPacketQueue::Push(const SOCKET sock, const BYTE *data, const int len,
	const bool fromNetwork) // fromNetwork = false
{
	cAutoCS cs(m_criticalSection);

	int totalLen = len;

	// 남은 여분의 버퍼가 있을 때.. 
	// 임시로 저장해뒀던 정보와 인자로 넘어온 정보를 합쳐서 처리한다.
	if (m_isStoreTempHeaderBuffer)
	{
		if (m_tempBufferSize < (len + m_tempHeaderBufferSize))
		{
			SAFE_DELETEA(m_tempBuffer);
			m_tempBuffer = new BYTE[len + m_tempHeaderBufferSize];
			m_tempBufferSize = len + m_tempHeaderBufferSize;
		}

		memcpy(m_tempBuffer, m_tempHeaderBuffer, m_tempHeaderBufferSize);
		memcpy(m_tempBuffer + m_tempHeaderBufferSize, data, len);
		m_isStoreTempHeaderBuffer = false;

		data = m_tempBuffer;
		totalLen = m_tempHeaderBufferSize + len;
	}

	int offset = 0;
	while (offset < totalLen)
	{
		const BYTE *ptr = data + offset;
		const int size = totalLen - offset;
		if (sSockBuffer *buffer = FindSockBuffer(sock))
		{
			offset += CopySockBuffer(buffer, ptr, size);
		}
		else
		{
			// 남은 여분의 데이타가 sHeader 보다 작을 때, 임시로 
			// 저장한 후, 나머지 패킷이 왔을 때 처리한다.
			if (fromNetwork && (size < (int)sizeof(sHeader)))
			{
				m_tempHeaderBufferSize = size;
				m_isStoreTempHeaderBuffer = true;
				memcpy(m_tempHeaderBuffer, ptr, size);
				offset += size;
			}
			else
			{
				offset += AddSockBuffer(sock, ptr, size, fromNetwork);
			}
		}
	}
}



// 크리티컬섹션이 먼저 생성된 후에 호출하자.
// sock 에 해당하는 큐를 리턴한다. 
sSockBuffer* cPacketQueue::FindSockBuffer(const SOCKET sock)
{
	// find specific packet by socket
	for (u_int i = 0; i < m_queue.size(); ++i)
	{
		if (sock != m_queue[i].sock)
			continue;
		// 해당 소켓이 채워지지 않은 버퍼라면, 리턴
		if (m_queue[i].readLen < m_queue[i].totalLen)
			return &m_queue[i];
	}
	return NULL;
}


// cSockBuffer *dst에 data 를 len 만큼 저장한다.
// 이 때, len이 SockBuffer의 버퍼크기 만큼 저장한다.
// 복사 된 크기를 바이트 단위로 리턴한다.
int cPacketQueue::CopySockBuffer(sSockBuffer *dst, const BYTE *data, const int len)
{
	RETV(!dst, 0);
	RETV(!dst->buffer, 0);

	const int copyLen = std::min(dst->totalLen - dst->readLen, len);
	memcpy(dst->buffer + dst->readLen, data, copyLen);
	dst->readLen += copyLen;

	if (dst->readLen == dst->totalLen)
		dst->full = true;

	return copyLen;
}


// 새 패킷 버퍼를 추가한다.
int cPacketQueue::AddSockBuffer(const SOCKET sock, const BYTE *data, const int len, const bool fromNetwork)
{
	// 새로 추가될 소켓의 패킷이라면
	sSockBuffer sockBuffer;
	sockBuffer.sock = sock;
	sockBuffer.totalLen = 0;
	sockBuffer.readLen = 0;
	sockBuffer.full = false;

	bool isMakeHeader = false; // 패킷 헤더가 없는 경우, 생성한다.

	int offset = 0;
	if (fromNetwork)
	{
		// 네트워크를 통해 온 패킷이라면, 이미 패킷헤더가 포함된 상태다.
		// 전체 패킷 크기를 저장해서, 분리된 패킷인지를 판단한다.
		sHeader *pheader = (sHeader*)data;
		if ((pheader->head[0] == '$') && (pheader->head[1] == '@'))
		{
			sockBuffer.totalLen = pheader->length;
			sockBuffer.actualLen = pheader->length - sizeof(sHeader);
			sockBuffer.buffer = Alloc();
		}
		else
		{
			if (m_isIgnoreHeader)
			{
				// 헤더가 없는 패킷을 경우, 생성해서 추가한다.
				isMakeHeader = true;
			}
			else
			{
				// error occur!!
				// 패킷의 시작부가 아닌데, 시작부로 들어왔음.
				// 헤더부가 깨졌거나, 기존 버퍼가 Pop 되었음.
				// 무시하고 종료한다.
				//common::dbg::ErrLog("packetqueue push error!! packet header not found\n");
				return len;
			}
		}
	}

	if (!fromNetwork || isMakeHeader)
	{
		// 송신할 패킷을 추가할 경우, or 헤더가 없는 패킷을 받을 경우.
		// 패킷 헤더를 추가한다.
		sockBuffer.totalLen = len + sizeof(sHeader);
		sockBuffer.actualLen = len;

		sHeader header;
		header.head[0] = '$';
		header.head[1] = '@';
		header.length = len + sizeof(sHeader);

		sockBuffer.buffer = Alloc();
		if (sockBuffer.buffer)
		{
			offset += CopySockBuffer(&sockBuffer, (BYTE*)&header, sizeof(sHeader));
		}
	}

	if (!sockBuffer.buffer)
	{
		//common::dbg::ErrLog("packetqueue push error!! canceled 1.\n");
		return len; // Error!! 
	}

	if ((sockBuffer.totalLen < 0) || (sockBuffer.actualLen < 0))
	{
		//common::dbg::ErrLog("packetqueue push error!! canceled 2.\n");
		return len;
	}

	CopySockBuffer(&sockBuffer, data, len);
	m_queue.push_back(sockBuffer);

	if (fromNetwork)
	{
		return sockBuffer.readLen;
	}
	else
	{
		// 패킷헤더 크기는 제외(순수하게 data에서 복사된 크기를 리턴한다.)
		return sockBuffer.readLen - sizeof(sHeader);
	}
}


bool cPacketQueue::Front(OUT sSockBuffer &out)
{
	RETV(m_queue.empty(), false);

	cAutoCS cs(m_criticalSection);
	RETV(!m_queue[0].full, false);

	out.sock = m_queue[0].sock;
	out.buffer = m_queue[0].buffer + sizeof(sHeader); // 헤더부를 제외한 패킷 데이타를 리턴한다.
	out.readLen = m_queue[0].readLen;
	out.totalLen = m_queue[0].totalLen;
	out.actualLen = m_queue[0].actualLen;

	return true;
}


void cPacketQueue::Pop()
{
 	cAutoCS cs(m_criticalSection);
 	RET(m_queue.empty());

	Free(m_queue.front().buffer);
	common::rotatepopvector(m_queue, 0);
}


// 즉시 모든 패킷을 전송한다.
void cPacketQueue::SendAll()
{
	RET(m_queue.empty());

	cAutoCS cs(m_criticalSection);
	for (u_int i = 0; i < m_queue.size(); ++i)
	{
		if (m_isIgnoreHeader)
		{
			send(m_queue[i].sock, (const char*)m_queue[i].buffer + sizeof(sHeader), m_queue[i].actualLen, 0);
		}
		else
		{
			send(m_queue[i].sock, (const char*)m_queue[i].buffer, m_queue[i].totalLen, 0);
		}
		Free(m_queue[i].buffer);
	}
	m_queue.clear();
}


void cPacketQueue::SendAll(const sockaddr_in &sockAddr)
{
	RET(m_queue.empty());

	cAutoCS cs(m_criticalSection);
	for (u_int i = 0; i < m_queue.size(); ++i)
	{
		if (m_isIgnoreHeader)
		{
			sendto(m_queue[i].sock, (const char*)m_queue[i].buffer + sizeof(sHeader), m_queue[i].actualLen,
				0, (struct sockaddr*) &sockAddr, sizeof(sockAddr));
		}
		else
		{
			sendto(m_queue[i].sock, (const char*)m_queue[i].buffer, m_queue[i].totalLen,
				0, (struct sockaddr*) &sockAddr, sizeof(sockAddr));
		}
		Free(m_queue[i].buffer);
	}
	m_queue.clear();
}


// 큐에 저장된 버퍼의 총 크기 (바이트 단위)
int cPacketQueue::GetSize()
{
	RETV(m_queue.empty(), 0);

	int size = 0;
	cAutoCS cs(m_criticalSection);
	for (u_int i = 0; i < m_queue.size(); ++i)
		size += m_queue[i].totalLen;
	return size;
}


// exceptOwner 가 true 일때, 패킷을 보낸 클라이언트를 제외한 나머지 클라이언트들에게 모두
// 패킷을 보낸다.
void cPacketQueue::SendBroadcast(vector<sSession> &sessions, const bool exceptOwner)
{
	cAutoCS cs(m_criticalSection);

	for (u_int i = 0; i < m_queue.size(); ++i)
	{
		if (!m_queue[i].full)
			continue; // 다 채워지지 않은 패킷은 무시

		for (u_int k = 0; k < sessions.size(); ++k)
		{
			// exceptOwner가 true일 때, 패킷을 준 클라이언트에게는 보내지 않는다.
			const bool isSend = !exceptOwner || (exceptOwner && (m_queue[i].sock != sessions[k].socket));
			if (isSend)
				send(sessions[k].socket, (const char*)m_queue[i].buffer, m_queue[i].totalLen, 0);
		}
	}

	// 모두 전송한 후 큐를 비운다.
	for (u_int i = 0; i < m_queue.size(); ++i)
		Free(m_queue[i].buffer);
	m_queue.clear();

	ClearMemPool();
}


void cPacketQueue::Lock()
{
	pthread_mutex_lock(&m_criticalSection);
}


void cPacketQueue::Unlock()
{
	pthread_mutex_unlock(&m_criticalSection);
}


BYTE* cPacketQueue::Alloc()
{
	for (u_int i = 0; i < m_memPool.size(); ++i)
	{
		if (!m_memPool[i].used)
		{
			m_memPool[i].used = true;
			return m_memPool[i].p;
		}
	}
	return NULL;
}


void cPacketQueue::Free(BYTE*ptr)
{
	for (u_int i = 0; i < m_memPool.size(); ++i)
	{
		if (ptr == m_memPool[i].p)
		{
			m_memPool[i].used = false;
			break;
		}
	}
}


void cPacketQueue::Clear()
{
	SAFE_DELETEA(m_memPoolPtr);
	SAFE_DELETEA(m_tempBuffer);
	SAFE_DELETEA(m_tempHeaderBuffer);
	m_memPool.clear();
	m_queue.clear();
}


// 메모리 풀을 초기화해서, 어쩌다 생길지 모를 버그를 제거한다.
void cPacketQueue::ClearMemPool()
{
	for (u_int i = 0; i < m_memPool.size(); ++i)
		m_memPool[i].used = false;
}
