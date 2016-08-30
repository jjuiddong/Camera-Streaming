
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions





#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC support for ribbons and control bars





// 포함 파일들.
#include <iostream>
#include <windows.h>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <set>
#include <queue>
#include <thread>
#include <shlwapi.h>
#include <boost/foreach.hpp>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv/cvaux.h>
#include <opencv2/calib3d.hpp>
#include <opencv2/xfeatures2d.hpp>


using std::string;
using std::wstring;
using std::vector;
using std::map;
using std::list;
using std::set;
using std::queue;
using std::stringstream;
using std::wstringstream;


// 매크로 정의
#ifndef SAFE_DELETE
#define SAFE_DELETE(p) {if (p) { delete p; p=NULL;} }
#endif
#ifndef SAFE_DELETEA
#define SAFE_DELETEA(p) {if (p) { delete[] p; p=NULL;} }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) {if (p) { p->Release(); p=NULL;} }
#endif
#ifndef SAFE_RELEASE2
#define SAFE_RELEASE2(p) {if (p) { p->release(); p=NULL;} }
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

#define RET(exp)		{if((exp)) return; }			// exp가 true이면 리턴
#define RETV(exp,val)	{if((exp)) return val; }
#define ASSERT_RET(exp)		{assert(exp); RET(!(exp) ); }
#define ASSERT_RETV(exp,val)		{assert(exp); RETV(!(exp),val ); }
#define BRK(exp)		{if((exp)) break; }			// exp가 break

typedef unsigned int    u_int;


#include "../Common/global.h"
using namespace common;

#include "../Network/network.h"
#include "../Common/ImageFrameWnd.h"
#include "../Common/dxcaptureutil.h"
#include "../Common/dxcapture.h"
#include "../Common/opencvgdiplus.h"
#include "../Common/streamingreceiver.h"
#include "../Common/streamingsender.h"


const static COLORREF g_blueColor = RGB(51, 153, 255);
const static COLORREF g_grayColor = RGB(240, 240, 240);
const static COLORREF g_redColor = RGB(255, 127, 80);
const static COLORREF g_yellowColor = RGB(255, 228, 135);
const static COLORREF g_greenColor = RGB(6, 176, 37);



#ifdef _DEBUG
#pragma comment(lib, "opencv_core310d.lib")
#pragma comment(lib, "opencv_imgcodecs310d.lib")
#pragma comment(lib, "opencv_features2d310d.lib")
#pragma comment(lib, "opencv_videoio310d.lib")
#pragma comment(lib, "opencv_highgui310d.lib")
#pragma comment(lib, "opencv_imgproc310d.lib")
#pragma comment(lib, "opencv_flann310d.lib")
#pragma comment(lib, "opencv_xfeatures2d310d.lib")
#pragma comment(lib, "opencv_calib3d310d.lib")
#else
#pragma comment(lib, "opencv_core310.lib")
#pragma comment(lib, "opencv_imgcodecs310.lib")
#pragma comment(lib, "opencv_features2d310.lib")
#pragma comment(lib, "opencv_videoio310.lib")
#pragma comment(lib, "opencv_highgui310.lib")
#pragma comment(lib, "opencv_imgproc310.lib")
#pragma comment(lib, "opencv_flann310.lib")
#pragma comment(lib, "opencv_xfeatures2d310.lib")
#pragma comment(lib, "opencv_calib3d310.lib")
#endif

