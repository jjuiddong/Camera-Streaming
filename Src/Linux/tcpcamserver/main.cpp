
#include <iostream>
#include "../opencvcom/streamingsender.h"

using namespace std;
using namespace cv;

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		cout << "commandline <bind port> <width=640> <height=480>" << endl;
		return 0;
	}

	const int bindPort = atoi(argv[1]);

	int width = 640, height = 480;
	if (argc >= 4)
	{
		width = atoi(argv[2]);
		height = atoi(argv[3]);
	}

	cout << "streaming server init... " << endl;
	cvproc::cStreamingSender sender;
	if (!sender.Init(bindPort, true, true))
	{
		cout << "bind error !!" << endl;
		return 0;
	}

	cout << "bind success.. " << endl;

	cout << "check camera...";
	VideoCapture cap(0);
	cap.set(CV_CAP_PROP_FRAME_WIDTH, width);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, height);
	cap.set(CV_CAP_PROP_FPS, 90);
	cap.set(CV_CAP_PROP_CONVERT_RGB, false);

	if (!cap.isOpened())
	{
		cout << " error" << endl;
		perror("OpenCV : open WebCam\n");
		return 0;
	}

	cout << "success" << endl;

	long oldT = GetTickCount();
	int fps = 0;
	Mat frame;

	while (1)
	{
		++fps;

		long curT = GetTickCount();
		if (curT - oldT > 1000)
		{
			//printf("fps = %d\n", fps);
			oldT = curT;
			fps = 0;
		}

		sender.CheckPacket();

		cap.read(frame);
// 		cvGrabFrame(capture);
// 		IplImage *frame = cvRetrieveFrame(capture);
		Mat src = Mat(frame);
		sender.Send(frame);
		//cout << "w = " << frame.cols << ", h = " << frame.rows << endl;
		cvWaitKey(1);
	}

	return 0;
}
