
#include <iostream>
#include "../netcom/network.h"

using namespace std;


int main(int argc, char **argv)
{
	
	network::cUDPClient udpClient;
	udpClient.SetMaxBufferLength(307200);
	if (!udpClient.Init("169.254.172.62", 8888))
	{
		cout << "udp init error" << endl;
		return -1;
	}

	BYTE *data = new BYTE[10000];

	while (1)
	{
		//udpClient.SendData((BYTE*)"hello", 6);
		udpClient.SendData(data, 1000);
		sleep(1);
	}
	
	delete[] data;
	return 0;
}
