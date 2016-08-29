
#include "stdafx.h"
#include "resource.h"		// main symbols
#include <mmsystem.h>
#include "afxwin.h"
#pragma comment(lib, "winmm.lib")

using namespace cv;


class CServerApp : public CWinApp
{
public:
	CServerApp();
public:
	virtual BOOL InitInstance();
	DECLARE_MESSAGE_MAP()
};


class CServerDlg : public CDialogEx
{
public:
	CServerDlg(CWnd* pParent = NULL);	// standard constructor
	enum { IDD = IDD_SERVER_DIALOG };
	void Run();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	void MainLoop(const float deltaSeconds);
	void Log(const string &str);


protected:
	enum STATE {STOP, START};

	STATE m_state;
	HICON m_hIcon;
	bool m_loop;
	cvproc::cStreamingSender m_streamSender;
	cvproc::cDxCapture m_camera;
	CImageFrameWnd *m_imgWindow;

	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	int m_serverPort;
	CButton m_buttonStart;
	afx_msg void OnBnClickedButtonShowcam();
	afx_msg void OnBnClickedButtonStart();
	CListBox m_listLog;
	CComboBox m_comboCamIndex;
	CComboBox m_comboCamWH;
};

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CServerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CServerApp::CServerApp()
{
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
}

CServerApp theApp;

BOOL CServerApp::InitInstance()
{
	CWinApp::InitInstance();
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
	CServerDlg *dlg = new CServerDlg();
	dlg->Create(CServerDlg::IDD);
	m_pMainWnd = dlg;
	dlg->ShowWindow(SW_SHOW);
	dlg->Run();
	dlg->DestroyWindow();
	delete dlg;
	ControlBarCleanUp();
	return FALSE;
}


CServerDlg::CServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SERVER_DIALOG, pParent)
	, m_state(STOP)
	, m_loop(true)
	, m_serverPort(11000)
	, m_imgWindow(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PORT, m_serverPort);
	DDX_Control(pDX, IDC_BUTTON_START, m_buttonStart);
	DDX_Control(pDX, IDC_LIST_LOG, m_listLog);
	DDX_Control(pDX, IDC_COMBO_CAM_INDEX, m_comboCamIndex);
	DDX_Control(pDX, IDC_COMBO_CAM_WH, m_comboCamWH);
}

BEGIN_MESSAGE_MAP(CServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CServerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CServerDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_SHOWCAM, &CServerDlg::OnBnClickedButtonShowcam)
	ON_BN_CLICKED(IDC_BUTTON_START, &CServerDlg::OnBnClickedButtonStart)
END_MESSAGE_MAP()

BOOL CServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	m_comboCamIndex.AddString(L"0");
	m_comboCamIndex.AddString(L"1");
	m_comboCamIndex.AddString(L"2");
	m_comboCamIndex.SetCurSel(0);

	m_comboCamWH.AddString(L"640 x 480");
	m_comboCamWH.SetCurSel(0);

	return TRUE;
}

void CServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}


void CServerDlg::OnBnClickedOk()
{
	CDialogEx::OnOK();
}

void CServerDlg::OnBnClickedCancel()
{
	m_loop = false;
}

void CServerDlg::Run()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	int oldT = timeGetTime();
	while (m_loop)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		const int curT = timeGetTime();
		if (curT - oldT > 20)
		{
			const float deltaSeconds = (curT - oldT) * 0.001f;
			oldT = curT;
			MainLoop(deltaSeconds);
		}
		Sleep(1);
	}
}


void CServerDlg::MainLoop(const float deltaSeconds)
{
	RET(m_state == STOP);

	m_streamSender.CheckPacket();

	Mat &src = m_camera.GetCloneBufferToImage();
	if (m_imgWindow && m_imgWindow->IsWindowVisible())
		m_imgWindow->ShowImage(src);

	m_streamSender.Send(src);	
}


void CServerDlg::OnBnClickedButtonShowcam()
{
	if (!m_imgWindow)
	{
		CGdiPlus::Init();
		m_imgWindow = new CImageFrameWnd();
		m_imgWindow->Init("camera", Mat(), CRect(0, 0, 300, 300), this, true);		
	}

	m_imgWindow->ShowWindow(SW_SHOW);
}


void CServerDlg::OnBnClickedButtonStart()
{
	UpdateData();

	if (STOP == m_state)
	{
		if (m_streamSender.Init(m_serverPort, false, false))
		{
			Log("Success Server Initialize");
		}
		else
		{
			Log("Fail Server Initialize");
			return;
		}

		CString strCamIdx;
		m_comboCamIndex.GetWindowText(strCamIdx);
		const int camIdx = _ttoi((LPCTSTR)strCamIdx);
		CString strCamWh;
		m_comboCamWH.GetWindowTextW(strCamWh);
		int camW = 0, camH = 0;
		swscanf((LPCTSTR)strCamWh, L"%d x %d", &camW, &camH);

		if (m_camera.Init(camIdx, camW, camH, true))
		{
			Log("Success Camera Initialize");
		}
		else
		{
			m_streamSender.Close();
			Log("Fail Camera Initialize");
			return;
		}

		m_state = START;
		m_buttonStart.SetWindowTextW(L"Stop");
		SetBackgroundColor(g_blueColor);
	}
	else // START
	{
		m_state = STOP;
		m_buttonStart.SetWindowTextW(L"Server Start");
		m_streamSender.Close();
		m_camera.Close();
		SetBackgroundColor(g_grayColor);
		Log("Server Stop");
	}
}


void CServerDlg::Log(const string &str)
{
	m_listLog.InsertString(m_listLog.GetCount(), str2wstr(str).c_str());
	m_listLog.SetCurSel(m_listLog.GetCount() - 1);
}
