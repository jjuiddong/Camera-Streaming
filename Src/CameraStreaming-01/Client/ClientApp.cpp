
#include "stdafx.h"
#include "resource.h"		// main symbols
#include "afxcmn.h"
#include "afxwin.h"
#pragma comment(lib, "winmm.lib")

using namespace cv;


class CClientApp : public CWinApp
{
public:
	CClientApp();
public:
	virtual BOOL InitInstance();
	DECLARE_MESSAGE_MAP()
};


class CClientDlg : public CDialogEx
{
public:
	CClientDlg(CWnd* pParent = NULL);	// standard constructor
	enum { IDD = IDD_CLIENT_DIALOG };
	void Run();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	void MainLoop(const float deltaSeconds);
	void Log(const string &str);

protected:
	enum STATE { STOP, START };

	STATE m_state;
	HICON m_hIcon;
	bool m_loop;
	cvproc::cStreamingReceiver m_streamRcv;
	CImageFrameWnd *m_imgWindow;
	int m_recvImageCount;

	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	int m_port;
	CIPAddressCtrl m_IP;
	CButton m_buttonConnect;
	afx_msg void OnBnClickedButtonConnect();
	CListBox m_listLog;
	BOOL m_checkGray;
	BOOL m_checkCompressed;
	CComboBox m_comboJpegQuality;
	afx_msg void OnBnClickedCheckComp();
	CComboBox m_comboProtocol;
	CComboBox m_comboNetCardIndex;
	CStatic m_staticIps;
	CComboBox m_comboFPS;
};


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(CClientApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()
CClientApp::CClientApp()
{
}

CClientApp theApp;
BOOL CClientApp::InitInstance()
{
	CWinApp::InitInstance();
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	CClientDlg *dlg = new CClientDlg();
	dlg->Create(CClientDlg::IDD);
	m_pMainWnd = dlg;
	dlg->ShowWindow(SW_SHOW);
	dlg->Run();
	dlg->DestroyWindow();
	delete dlg;
	ControlBarCleanUp();
	return FALSE;
}


CClientDlg::CClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CLIENT_DIALOG, pParent)
	, m_loop(true)
	, m_port(11000)
	, m_state(STOP)
	, m_imgWindow(NULL)
	, m_checkGray(FALSE)
	, m_checkCompressed(FALSE)
	, m_recvImageCount(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PORT, m_port);
	DDX_Control(pDX, IDC_IPADDRESS_IP, m_IP);
	DDX_Control(pDX, IDC_BUTTON_CONNECT, m_buttonConnect);
	DDX_Control(pDX, IDC_LIST_LOG, m_listLog);
	DDX_Check(pDX, IDC_CHECK_GRAY, m_checkGray);
	DDX_Check(pDX, IDC_CHECK_COMP, m_checkCompressed);
	DDX_Control(pDX, IDC_COMBO_COMP_RATE, m_comboJpegQuality);
	DDX_Control(pDX, IDC_COMBO_PROTOCOL, m_comboProtocol);
	DDX_Control(pDX, IDC_COMBO_NETCARD_INDEX, m_comboNetCardIndex);
	DDX_Control(pDX, IDC_STATIC_FPS, m_staticIps);
	DDX_Control(pDX, IDC_COMBO_FPS, m_comboFPS);
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CClientDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CClientDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CClientDlg::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_CHECK_COMP, &CClientDlg::OnBnClickedCheckComp)
END_MESSAGE_MAP()


BOOL CClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	//m_IP.SetAddress(127, 0, 0, 1);
	m_IP.SetAddress(192,168,0,13);

	m_comboJpegQuality.AddString(L"100");
	m_comboJpegQuality.AddString(L"90");
	m_comboJpegQuality.AddString(L"80");
	m_comboJpegQuality.AddString(L"70");
	m_comboJpegQuality.AddString(L"60");
	m_comboJpegQuality.SetCurSel(0);
	m_comboJpegQuality.EnableWindow(FALSE);

	m_comboProtocol.AddString(L"TCP");
	m_comboProtocol.AddString(L"UDP");
	m_comboProtocol.SetCurSel(0);

	m_comboNetCardIndex.AddString(L"0");
	m_comboNetCardIndex.AddString(L"1");
	m_comboNetCardIndex.AddString(L"2");
	m_comboNetCardIndex.AddString(L"3");
	m_comboNetCardIndex.SetCurSel(0);

	m_comboFPS.AddString(L"50");
	m_comboFPS.AddString(L"40");
	m_comboFPS.AddString(L"30");
	m_comboFPS.AddString(L"20");
	m_comboFPS.AddString(L"10");
	m_comboFPS.SetCurSel(2);

	return TRUE;
}

void CClientDlg::OnPaint()
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


void CClientDlg::OnBnClickedOk()
{
	CDialogEx::OnOK();
}


void CClientDlg::OnBnClickedCancel()
{
	m_loop = false;
}


void CClientDlg::Run()
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
		const float deltaSeconds = (curT - oldT) * 0.001f;
		oldT = curT;
		MainLoop(deltaSeconds);

		Sleep(1);
	}
}


void CClientDlg::MainLoop(const float deltaSeconds)
{
	RET(m_state == STOP);
	RET(!m_streamRcv.IsConnect());

	if (m_streamRcv.Update())
	{
		if (!m_imgWindow)
		{
			CGdiPlus::Init();
			m_imgWindow = new CImageFrameWnd();
			m_imgWindow->Init("camera", Mat(), CRect(0, 0, 300, 300), this, true);
		}

		m_imgWindow->ShowImage(m_streamRcv.m_cloneImage);
		++m_recvImageCount;
	}

	// 속도 측정
	static float elapseTime = 0;
	elapseTime += deltaSeconds;
	if (elapseTime > 1.f)
	{
		m_staticIps.SetWindowTextW(common::formatw("Receive Image per Second = %d", m_recvImageCount).c_str());
		elapseTime = 0;
		m_recvImageCount = 0;
	}
}


void CClientDlg::OnBnClickedButtonConnect()
{
	UpdateData();

	if (STOP == m_state)
	{
		CString strJpegQuality;
		m_comboJpegQuality.GetWindowText(strJpegQuality);
		const int jpegQuality = _ttoi((LPCTSTR)strJpegQuality);

		CString strNetCardIdx;
		m_comboNetCardIndex.GetWindowText(strNetCardIdx);
		const int netCardIdx = _ttoi((LPCTSTR)strNetCardIdx);

		CString strFPS;
		m_comboFPS.GetWindowText(strFPS);
		const int fps = _ttoi((LPCTSTR)strFPS);

		CString strProtocol;
		m_comboProtocol.GetWindowTextW(strProtocol);
		const bool isUDP = (strProtocol == L"UDP");

		const string ip = GetIP(m_IP);
		if (m_streamRcv.Init(isUDP, ip, m_port, netCardIdx, m_checkGray, m_checkCompressed, jpegQuality, fps))
		{
			Log("Success Connect Server");
			if (isUDP)
				Log(common::format("    - Receive UDP IP = %s ", m_streamRcv.m_rcvUDPIp.c_str()));

			m_streamRcv.m_isLog = true;
			m_state = START;
			m_buttonConnect.SetWindowTextW(L"Stop");
			SetBackgroundColor(g_blueColor);
			if (m_imgWindow)
				m_imgWindow->ShowWindow(SW_SHOW);
		}
		else
		{
			Log("Fail Connect Server");
		}
	}
	else // START
	{
		m_state = STOP;
		m_buttonConnect.SetWindowTextW(L"Connect");
		m_streamRcv.Close();
		SetBackgroundColor(g_grayColor);
		Log("Stop Receive");
	}
}


void CClientDlg::Log(const string &str)
{
	m_listLog.InsertString(m_listLog.GetCount(), str2wstr(str).c_str());
	m_listLog.SetCurSel(m_listLog.GetCount() - 1);
}


void CClientDlg::OnBnClickedCheckComp()
{
	UpdateData();
	m_comboJpegQuality.EnableWindow(m_checkCompressed);
}
