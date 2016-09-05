
#pragma once


namespace network
{

	namespace PROTOCOL
	{
		enum TYPE
		{
			LOGIN,
			CHATTING,
		};
	}


	struct sPacketHeader
	{
		PROTOCOL::TYPE protocol;
		int size;
	};


	struct sLoginProtocol
	{
		sPacketHeader header;
		char name[ 32];
		char pass[ 8];
	};


	struct sChatProtocol
	{
		sPacketHeader header;
		char msg[ 64];
	};

}