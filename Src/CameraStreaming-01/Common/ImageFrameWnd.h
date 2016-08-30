//
// 2016-07-03, jjuiddong
// 이미지 출력 전용 윈도우
//
#pragma once


class CImageFrameWnd : public CMiniFrameWnd
{
protected:
	class cImageView : public CView
	{
	public:
		cImageView();
		virtual ~cImageView();

		bool Init(CWnd *parentWnd, const CRect &rect);
		bool ShowImage(const string &fileName);
		bool ShowImage(const cv::Mat &img);
		virtual void OnDraw(CDC* pDC);
		afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		BOOL OnEraseBkgnd(CDC* pDC);


	public:
		string m_fileName;
		Gdiplus::Image *m_image;
		int m_imgFlags;
		DECLARE_MESSAGE_MAP()
	};



public:
	CImageFrameWnd();
	virtual ~CImageFrameWnd();

	bool Init(const string &windowName, const string &fileName, const CRect &initialRect, CWnd *parentWnd, const bool isAutoSize=false);
	bool Init(const string &windowName, const cv::Mat &img, const CRect &initialRect, CWnd *parentWnd, const bool isAutoSize = false);
	bool ShowImage(const string &fileName);
	bool ShowImage(const cv::Mat &img);
	void SetAutoSize(const bool isAutoSize);


protected:
	void Initialize(const string &windowName, const CRect &initialRect, CWnd *parentWnd, const bool isAutoSize);
	void ReCalcWindowSize();


protected:
	string m_windowName;
	bool m_isAutoSize;
	cImageView *m_imageView;


protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
