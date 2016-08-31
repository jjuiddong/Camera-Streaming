
#include "streamingreceiver.h"

using namespace cv;
using namespace cvproc;


cStreamingReceiver::cStreamingReceiver()
	: m_rcvBuffer(NULL)
{
}

cStreamingReceiver::~cStreamingReceiver()
{
	SAFE_DELETEA(m_rcvBuffer);
}


bool cStreamingReceiver::Init(const bool isUDP, const string &ip, const int port, const int networkCardIdx,
	const int gray, const int compressed, const int jpgComQuality, const int fps)
{
	m_isUDP = isUDP;

	m_udpServer.Close();
	m_tcpClient.Close();

	if (!m_tcpClient.Init(ip, port, g_maxStreamSize, 10, 10))
		return false;

	// UDP로 전송 받기위해, 현재 컴퓨터의 IP와 포트번호를 전송한다.
	// 포트번호는 tcp/ip포트의 +1 한 값이다.
	if (isUDP)
	{
		const string rcvIp = GetHostIP(networkCardIdx);
		std::cout << "UDP Receive IP = " << rcvIp << std::endl;

		sStreamingProtocol data;
		data.protocol = 100;
		data.type = 1;
		data.port = port + 1;
		data.uip = inet_addr(rcvIp.c_str());
		m_tcpClient.Send((BYTE*)&data, sizeof(data));

		m_udpServer.SetMaxBufferLength(g_maxStreamSize);
		if (!m_udpServer.Init(0, port + 1))
		{
			// udp로 수신되는 것이 실패했다면, tcp/ip로 받는다.
			m_isUDP = false;
		}
	}
	else
	{
		sStreamingProtocol data;
		data.protocol = 100;
		data.type = 0;
		m_tcpClient.Send((BYTE*)&data, sizeof(data));
	}

	{
		// gray, compressed, jpgCompQuality 설정 프로토콜
		sleep(1);
		sStreamingProtocol data;
		data.protocol = 101;
		data.gray = gray == 1 ? true : false;
		data.compressed = compressed == 1 ? true : false;
		data.compQuality = jpgComQuality;
		data.fps = fps;
		m_tcpClient.Send((BYTE*)&data, sizeof(data));
	}

	if (!m_rcvBuffer)
		m_rcvBuffer = new BYTE[g_maxStreamSize];

	return true;
}


cv::Mat& cStreamingReceiver::Update()
{
	RETV(!IsConnect(), m_finalImage);

	const static int sizePerChunk = g_maxStreamSize - sizeof(sStreamingData) - sizeof(network::cPacketQueue::sHeader); // size per chunk

	int len = 0;
	if (m_isUDP)
	{
		len = m_udpServer.GetRecvData(m_rcvBuffer, g_maxStreamSize);
	}
	else
 	{
		network::sPacket packet;
		if (m_tcpClient.m_recvQueue.Front(packet))
		{
			len = packet.actualLen;
			memcpy(m_rcvBuffer, packet.buffer, packet.actualLen);
			m_tcpClient.m_recvQueue.Pop();
		}
	}
	
	//std::cout << "update len=" << len << ", udp=" << m_isUDP << std::endl;

	if (len <= 0)
		return m_finalImage;

	sStreamingData *packet = (sStreamingData*)m_rcvBuffer;
	static int oldCompressed = -1;
	static int oldGray = -1;
	if (oldCompressed != (int)packet->isCompressed)
	{
		oldCompressed = (int)packet->isCompressed;
		std::cout << "compressed = " << (int)packet->isCompressed << std::endl;
	}
	if (oldGray != (int)packet->isGray)
	{
		oldGray = (int)packet->isGray;
		std::cout << "gray = " << (int)packet->isGray << std::endl;

		if (packet->isGray)
		{
			m_src = Mat(480, 640, CV_8UC1);
			m_finalImage = Mat(480, 640, CV_8UC1);
		}
		else
		{
			m_src = Mat(480, 640, CV_8UC3);
			m_finalImage = Mat(480, 640, CV_8UC3);
		}
	}


	if (packet->chunkSize == 1)
	{
		packet->data = m_rcvBuffer + sizeof(sStreamingData);

		if (packet->isCompressed)
		{
			if (packet->imageBytes <= len)
			{
				memcpy((char*)m_src.data, packet->data, packet->imageBytes);
				cv::imdecode(m_src, 1, &m_finalImage);
			}
		}
		else
		{
			// 바로 decode 에 복사해서 리턴한다.
			memcpy((char*)m_finalImage.data, packet->data, packet->imageBytes);
		}
	}
	else if (packet->chunkSize > 1)
	{
		static unsigned char oldId = 0;

		if (oldId != packet->id)
		{
			if (packet->isCompressed)
			{
				if (!m_compBuffer.empty())
					cv::imdecode(m_compBuffer, 1, &m_finalImage);
			}
			else
			{
				// 바로 decode 에 복사해서 리턴한다.
				if (!m_compBuffer.empty())
					memcpy((char*)m_finalImage.data, (char*)&m_compBuffer[0], m_src.total() * m_src.elemSize());
			}

			m_compBuffer.resize(packet->imageBytes);
			bzero(m_checkRcv, sizeof(m_checkRcv));			
		
			oldId = packet->id;
		}

		packet->data = m_rcvBuffer + sizeof(sStreamingData);
		char *dst = (char*)&m_compBuffer[0] + packet->chunkIndex * sizePerChunk;
		const int copyLen = max(0, (len - (int)sizeof(sStreamingData)));
		memcpy(dst, packet->data, copyLen);

		if (packet->chunkIndex < 32)
			m_checkRcv[ packet->chunkIndex] = true;
	}

	return m_finalImage;
}


bool cStreamingReceiver::IsConnect()
{
//  	if (m_isUDP)
// 		return m_udpServer.IsConnect();
	return m_tcpClient.IsConnect();
}


void cStreamingReceiver::Close()
{
	m_udpServer.Close();
	m_tcpClient.Close();
}
