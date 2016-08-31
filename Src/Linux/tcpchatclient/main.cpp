
#include <iostream>
#include "../netcom/network.h"

using namespace std;

bool g_threadLoop = true;
network::cTCPClient client;
void* PrintRecv(void* arg);


int main(int argc, char **argv)
{
	if (argc < 3)
	{
		cout << "command line <ip> <port>" << endl;
		return 0;
	}

	const string ip = argv[1];
	const int port = atoi(argv[2]);
	if (!client.Init(ip, port))
	{
		cout << "connect error!!" << endl;
		return false;
	}

	cout << "connect success .. start loop.." << endl;

	pthread_t handle;
	g_threadLoop = true;
	pthread_create(&handle, NULL, PrintRecv, NULL);

	while (1)
	{
		cout << "send data : ";
		char buff[512];
		fgets(buff, sizeof(buff), stdin);

		client.Send((BYTE*)buff, sizeof(buff));
		sleep(1);
	}

	g_threadLoop = false;
	pthread_join(handle, NULL);

	return 0;
}


void* PrintRecv(void* arg)
{
	char buff[512];

	while (g_threadLoop)
	{
		network::sPacket packet;
		if (client.m_recvQueue.Front(packet))
		{
			memcpy(buff, packet.buffer, packet.actualLen);
			client.m_recvQueue.Pop();
			cout << "recv data : " << packet.buffer << endl;
		}
	}

	return 0;
}
