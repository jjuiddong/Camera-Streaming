
#include <iostream>
#include "../netcom/network.h"

using namespace std;


int main(int argc, char **argv)
{
	cout << "TCPChatServer.." << endl;

	int bindPort = 5100;
	if (argc > 1)
		bindPort = atoi(argv[1]);

	network::cTCPServer server;
	if (!server.Init(bindPort))
	{
		cout << "server bind error" << endl;
		return 0;
	}

	cout << "Start Loop.." << endl;

	BYTE *recvBuffer = new BYTE[512];
	u_int sessionCount = 0;

	while (1)
	{
		if (sessionCount != server.m_sessions.size())
		{
			cout << "session count = " << server.m_sessions.size() << endl;
			sessionCount = server.m_sessions.size();
		}

		//-----------------------------------------------------------------------------------------------
		// Send Broadcast
		network::sPacket packet;
		if (server.m_recvQueue.Front(packet))
		{
			memcpy(recvBuffer, packet.buffer, packet.actualLen);
			server.m_recvQueue.Pop();
			for (u_int i = 0; i < server.m_sessions.size(); ++i)
			{
				//if (server.m_sessions[i].socket != packet.sock)
				{
					//cout << "send data : " << recvBuffer << endl;
					server.m_sendQueue.Push(server.m_sessions[i].socket, recvBuffer, packet.actualLen);
				}
			}
		}
		//-----------------------------------------------------------------------------------------------
	}

	SAFE_DELETEA(recvBuffer);
	return 0;
}
