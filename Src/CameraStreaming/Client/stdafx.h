
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











#include "../../../../Common/Common/common.h"
#include "../../../../Common/CamCommon/camcommon.h"
#include "../../../../Common/Network/network.h"

#include "../../../../Common/UIComponent/UIUtiltity.h"
#include "../../../../Common/UIComponent/bpctrlanchormap.h"
#include "../../../../Common/UIComponent/ImageFrameWnd.h"


const static COLORREF g_blueColor = RGB(51, 153, 255);
const static COLORREF g_grayColor = RGB(240, 240, 240);
const static COLORREF g_redColor = RGB(255, 127, 80);
const static COLORREF g_yellowColor = RGB(255, 228, 135);
const static COLORREF g_greenColor = RGB(6, 176, 37);
