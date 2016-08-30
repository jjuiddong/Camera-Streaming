#pragma once


#include <algorithm>
namespace Gdiplus
{
	using std::min;
	using std::max;
};


// IMPORTANT:
// This must be included AFTER gdiplus !!
// (OpenCV #undefine's min(), max())
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

//using namespace cv;

namespace Gdiplus {
	class BitmapData;
	class Bitmap;
}

class CGdiPlus
{
public:
	static void  Init();
	static cv::Mat  ImgRead(const WCHAR* u16_File);
	static void ImgWrite(cv::Mat i_Mat, const WCHAR* u16_File);
	static cv::Mat  CopyBmpToMat(Gdiplus::Bitmap* pi_Bmp);
	static cv::Mat  CopyBmpDataToMat(Gdiplus::BitmapData* pi_Data);
	static Gdiplus::Bitmap* CopyMatToBmp(const cv::Mat& i_Mat);
	static void CopyMatToBmp(const cv::Mat& i_Mat, Gdiplus::Bitmap *pi_Bmp);

	static CLSID GetEncoderClsid(const WCHAR* u16_File);
	static BOOL mb_InitDone;
};

