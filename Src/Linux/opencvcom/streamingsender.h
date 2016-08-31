//
// 이미지 정보를 UDP를 통해 전달한다.
// cStreamingReceiver와 같이 동작한다.
// 이미지를 압축하거나, 원본 그대로 전송할 수 있다.
// 이미지 용량이 크면, 나눠서 전송한다.
//
// Protocol
// - (byte) id (이미지를 나타내는 id)
//	- (byte) chunk size (이미지 용량 때문에 나눠진 데이타 청크의 갯수)
//	- (byte) chunk index
//	- (byte) gray 0 : 1
//	- (byte) compressed 0 : 1
//	- (int) image size
//
// 2016-02-09
//		- linux 작업
//		- 처음 접속은 tcp/ip로 이뤄진다. 이 후, udp로 통신할지의 여부는 
//		sStreamingProtocol 패킷을 클라이언트가 전송하면서 결정한다.
//
// 2016-08-30
//		- 코드 정리, 버그 해결
//		- linux 작업
//
#pragma once

#include "../netcom/network.h"
#include "opencvcom.h"
#include "streaming.h"


namespace cvproc
{
	class cStreamingSender : public network::iSessionListener
	{
	public:
		cStreamingSender();
		virtual ~cStreamingSender();

		bool Init(const int port, 
			const bool isConvertGray = true, const bool isCompressed = true, const int jpgQuality = 40);
		void CheckPacket();
		int Send(const cv::Mat &image);
		bool IsConnect(const bool isUdp);
		bool IsExistClient();
		void Close();


	protected:
		int SendImage(const BYTE *data, const int len, 
			const unsigned short width, const unsigned short height, const unsigned int flag,
			const bool isGray, const bool isCompressed);
		bool SendSplit();
		virtual void RemoveSession(const SOCKET remoteSock) override;
		virtual void AddSession(const SOCKET remoteSock) override;


	public:
		network::cTCPServer m_tcpServer;

		enum {MAX_UDP_COUNT=2};
		map<SOCKET,int> m_users; // socket = udp index matching, default = -1
		network::cUDPClient m_udpClient[MAX_UDP_COUNT]; // 최대 갯수가 제한 됨
		bool m_udpUsed[MAX_UDP_COUNT];							// m_udpClient[] 가 사용되는 중이면,  true로 설정.

		bool m_isConvertGray;
		bool m_isCompressed;
		int m_jpgCompressQuality;
		bool m_tempIsConvertGray;
		bool m_tempIsCompressed;
		int m_tempJpgCompressQuality;
		int m_fps;
		int m_deltaTime;
		int m_lastSendTime;
		vector<uchar> m_compressBuffer;
		cv::Mat m_gray;
		BYTE *m_sndBuffer;
		int m_rows;
		int m_cols;
		int m_flag;


	protected:
		enum STATE {
			NOTCONNECT,	// 서버에 접속되어 있지 않은 상태
			READY,	// 이미지 전송이 가능한 상태
			SPLIT,	// 이미지를 여러개의 청크로 나누어서 보내는 상태
		};
		STATE m_state;
		bool m_isUDP;	// udp/ip or tcp/ip

		// Send Split Parameters
		BYTE *m_imagePtr;
		int m_chunkId;
		int m_sendChunkIndex;
		int m_chunkCount;
		int m_imageBytes;
		int m_sendBytes;
		int m_totalSendBytes; // image + streamingdata + packet header
	};
}
