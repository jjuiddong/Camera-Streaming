
#include "stdafx.h"
#include "ImageFrameWnd.h"



//-----------------------------------------------------------------------------------------------
// cImageView

BEGIN_MESSAGE_MAP(CImageFrameWnd::cImageView, CView)
	ON_WM_KEYDOWN()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


CImageFrameWnd::cImageView::cImageView()
	: m_image(NULL) 
	, m_imgFlags(-1)
{
}

CImageFrameWnd::cImageView::~cImageView()
{
	SAFE_DELETE(m_image);
}

bool CImageFrameWnd::cImageView::Init(CWnd *parentWnd, const CRect &rect)
{
	if (!Create(NULL, L"ImageWnd", WS_VISIBLE | WS_CHILDWINDOW, rect, parentWnd, 100))
		return false;
	return true;
}

bool CImageFrameWnd::cImageView::ShowImage(const string &fileName)
{
	SAFE_DELETE(m_image);
	m_image = Gdiplus::Image::FromFile(str2wstr(fileName).c_str());
	InvalidateRect(0);
	return true;
}

bool CImageFrameWnd::cImageView::ShowImage(const cv::Mat &img)
{
	if (img.data)
	{
		if (m_image && (m_imgFlags == img.flags))
		{
			CGdiPlus::CopyMatToBmp(img, (Gdiplus::Bitmap*)m_image);
		}
		else
		{
			SAFE_DELETE(m_image);
			m_image = CGdiPlus::CopyMatToBmp(img);
			m_imgFlags = img.flags;
		}
		InvalidateRect(0);
	}
	return true;
}

void CImageFrameWnd::cImageView::OnDraw(CDC* pDC)
{
	Gdiplus::Graphics g(*pDC);
	if (m_image)
	{
		CRect cr;
		GetClientRect(cr);
		// 윈도우 크기가 이미지 보다 크다면, 이미지 크기대로 출력한다.
		// 이미지 크기가 윈도우 보다 크다면, 윈도우 크기대로 출력한다.
		const int w = MIN((int)m_image->GetWidth(), cr.Width());
		const int h = MIN((int)m_image->GetHeight(), cr.Height());
		g.DrawImage(m_image, 0, 0, w, h);
	}
}

void CImageFrameWnd::cImageView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_ESCAPE)
	{
		CImageFrameWnd *parent = (CImageFrameWnd*)GetParent();
		parent->ShowWindow(SW_HIDE);
	}
}

BOOL CImageFrameWnd::cImageView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}



//-----------------------------------------------------------------------------------------------
// CImageFrameWnd

CImageFrameWnd::CImageFrameWnd() 
	: m_isAutoSize(false)
	, m_imageView(NULL)
{
}

CImageFrameWnd::~CImageFrameWnd()
{
}


BEGIN_MESSAGE_MAP(CImageFrameWnd, CMiniFrameWnd)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CImageFrameWnd2 message handlers

bool CImageFrameWnd::Init(const string &windowName, const string &fileName, const CRect &initialRect, 
	CWnd *parentWnd, const bool isAutoSize, const bool isTopmost)
{
	Initialize(windowName, initialRect, parentWnd, isAutoSize, isTopmost);
	ShowImage(fileName);
	return true;
}


bool CImageFrameWnd::Init(const string &windowName, const cv::Mat &img, const CRect &initialRect, 
	CWnd *parentWnd, const bool isAutoSize, const bool isTopmost)
{
	Initialize(windowName, initialRect, parentWnd, isAutoSize, isTopmost);
	ShowImage(img);
	return true;
}


// 윈도우 초기화, 
void CImageFrameWnd::Initialize(const string &windowName, const CRect &initialRect, 
	CWnd *parentWnd, const bool isAutoSize, const bool isTopmost)
{
	const CString strClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, 0, (HBRUSH)(COLOR_WINDOW + 1));
	CreateEx(0, strClass,
		str2wstr(windowName).c_str(),
		WS_POPUP | WS_CAPTION | WS_SYSMENU 
		| WS_MAXIMIZEBOX // to use double click full screen window
		//MFS_BLOCKSYSMENU
		,
		initialRect);

	CRect cr;
	GetClientRect(cr);
	m_imageView = new cImageView();
	m_imageView->Init(this, cr);
	m_imageView->ShowWindow(SW_SHOW);

	m_isAutoSize = isAutoSize;
	m_windowName = windowName;

	::SetWindowPos(m_hWnd, isTopmost? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
}


bool CImageFrameWnd::ShowImage(const string &fileName)
{
	SetWindowText(str2wstr(m_windowName + " - " + fileName).c_str());
	if (!m_imageView->ShowImage(fileName))
		return false;
	ReCalcWindowSize();
	return true;
}


bool CImageFrameWnd::ShowImage(const cv::Mat &img)
{
	SetWindowText(str2wstr(m_windowName).c_str());
	if (!m_imageView->ShowImage(img))
		return false;
	ReCalcWindowSize();
	return true;
}


void CImageFrameWnd::ReCalcWindowSize()
{
	if (m_isAutoSize)
	{
		const int sideEdgeSize = 20;
		const int captionSize = 40;
		const int w = MAX(300, (m_imageView->m_image)? m_imageView->m_image->GetWidth() + sideEdgeSize : 0);
		const int h = MAX(100, (m_imageView->m_image)? m_imageView->m_image->GetHeight() + captionSize : 0);
		CRect wr;
		GetWindowRect(wr);
		MoveWindow(CRect(wr.left, wr.top, wr.left + w, wr.top + h));
	}
}


void CImageFrameWnd::OnSize(UINT nType, int cx, int cy)
{
	CMiniFrameWnd::OnSize(nType, cx, cy);
	if (m_imageView)
	{
		m_imageView->MoveWindow(CRect(0, 0, cx, cy));
	}	
}


void CImageFrameWnd::OnClose()
{
	ShowWindow(SW_HIDE);
}

void CImageFrameWnd::SetAutoSize(const bool isAutoSize)
{
	m_isAutoSize = isAutoSize;
}


BOOL CImageFrameWnd::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}
