
#include <iostream>
#include "../opencvcom/streamingreceiver.h"

using namespace std;
using namespace cv;

int main(int argc, char **argv)
{
	if (argc < 4)
	{
		cout << "commandline <ip> <port> <udp=1, tcp=0> <networkCardIndex=0>  <gray=1 or 0> <compressed=1 or 0> <jpgCompQuality=40> <fps=20>" << endl;
		return 0;
	}

	const string ip = argv[1];
	const int port = atoi(argv[2]);
	const bool isudp = atoi(argv[3]) == 1 ? true : false;
	int networkCardIndex = 0;
	if (argc >= 5)
		networkCardIndex = atoi(argv[4]);
	int gray = 1;
	if (argc >= 6)
		gray = atoi(argv[5]);
	int compressed = 1;
	if (argc >= 7)
		compressed = atoi(argv[6]);
	int jpgCompQual = 40;
	if (argc >= 8)
		jpgCompQual = atoi(argv[7]);
	int fps = 20;
	if (argc >= 9)
		fps = atoi(argv[8]);
	
	cout << "streaming client init... " << endl;
	cvproc::cStreamingReceiver receiver;
	if (!receiver.Init(isudp, ip, port, networkCardIndex, gray, compressed, jpgCompQual, fps))
	{
		cout << "connect error!!" << endl;
		return 0;
	}

	cout << "success" << endl;
	cout << "start loop" << endl;

	const string windowName = "Camera Streaming Client";
	namedWindow(windowName);

	while (1)
	{
		imshow(windowName, receiver.Update());
		if (27 == waitKey(1))
			break;
	}

	return 0;
}
