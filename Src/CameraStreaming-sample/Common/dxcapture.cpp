
#include "stdafx.h"
#include "dxcapture.h"
#include <assert.h>
#include <process.h>
#include <mmsystem.h>
#include <gdiplus.h>

using namespace cv;
using namespace cvproc;

// Application-defined message to notify app of filtergraph events
//#define WM_GRAPHNOTIFY  WM_APP+1

#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "gdiplus.lib")

unsigned WINAPI DxCaptureThreadFunction(void* arg);


cDxCapture::cDxCapture() :
	m_width(0)
	, m_height(0)
	, m_pFrameBuffer(NULL)
	, m_frameBufferSize(0)
	, m_pCloneFrameBuffer(NULL)
	, m_isThreadMode(false)
	, m_threadLoop(true)
	, m_isUpdateBuffer(false)
	, m_cameraIndex(0)
	, m_showFrame(false)
	, m_handle(NULL)
	, m_isUnpluggedInit(false)
	, m_gdiBitmap(NULL)
{
	m_dwGraphRegister = 0;
	m_pVW = NULL;
	m_pMC = NULL;
	m_pME = NULL;
	m_pGraph = NULL;
	m_pCapture = NULL;
	m_pGrabberF = NULL;
	m_pGrabber = NULL;
	m_pNullF = NULL;
	m_psCurrent = STOPPED;
	ZeroMemory(&m_mt, sizeof(m_mt));

	InitializeCriticalSectionAndSpinCount(&m_criticalSection, 0x00000400);

}

cDxCapture::~cDxCapture()
{
	Close();

	DeleteCriticalSection(&m_criticalSection);
 
	SAFE_DELETE(m_gdiBitmap);
 	// Release COM
// 	CoUninitialize();
}


// DirectX Show 초기화, 인터페이스 생성
bool cDxCapture::Init(const int cameraIndex, const int width, const int height, const bool isThreadMode)
{
	// Initialize COM
	if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
	{
		Msg(TEXT("CoInitialize Failed!\r\n"));
		exit(1);
	}

	m_cameraIndex = cameraIndex;
	m_width = width;
	m_height = height;

	// Create DirectShow graph and start capturing video
	HRESULT hr;
	hr = CaptureVideo(cameraIndex);
	if (FAILED(hr))
		return false;

	m_isThreadMode = isThreadMode;

	m_threadLoop = true;

	if (isThreadMode && !m_handle)
	{
		m_handle = (HANDLE)_beginthreadex(NULL, 0, DxCaptureThreadFunction, this, 0, (unsigned*)&m_threadId);
	}

	return true;
}


HRESULT cDxCapture::CaptureVideo(const int cameraIndex) //cameraIndex = 0
{
	HRESULT hr;
	IBaseFilter *pSrcFilter = NULL;

	// Get DirectShow interfaces
	hr = GetInterfaces();
	if (FAILED(hr))
	{
		Msg(TEXT("Failed to get video interfaces!  hr=0x%x"), hr);
		return hr;
	}

	// Attach the filter graph to the capture graph
	hr = m_pCapture->SetFiltergraph(m_pGraph);
	if (FAILED(hr))
	{
		Msg(TEXT("Failed to set capture filter graph!  hr=0x%x"), hr);
		return hr;
	}

	// Use the system device enumerator and class enumerator to find
	// a video capture/preview device, such as a desktop USB video camera.
	hr = FindCaptureDevice(cameraIndex, &pSrcFilter);
	if (FAILED(hr))
	{
		// Don't display a message because FindCaptureDevice will handle it
		return hr;
	}

	// Add Capture filter to our graph.
	hr = m_pGraph->AddFilter(pSrcFilter, L"Video Capture");
	if (FAILED(hr))
	{
		Msg(TEXT("Couldn't add the capture filter to the graph!  hr=0x%x\r\n\r\n")
			TEXT("If you have a working video capture device, please make sure\r\n")
			TEXT("that it is connected and is not being used by another application.\r\n\r\n")
			TEXT("The sample will now close."), hr);
		pSrcFilter->Release();
		return hr;
	}


	IEnumPins *pEnum = NULL;
	IPin *pPin = NULL;
	hr = pSrcFilter->EnumPins(&pEnum);
	if (FAILED(hr))
		return hr;


	while (S_OK == pEnum->Next(1, &pPin, NULL))
	{
		hr = ConnectFilters(m_pGraph, pPin, m_pGrabberF);
		SafeRelease(&pPin);
		if (SUCCEEDED(hr))
		{
			break;
		}
	}

	if (FAILED(hr))
		return hr;


	hr = m_pGraph->AddFilter(m_pNullF, L"Null Filter");
	if (FAILED(hr))
		return hr;

	hr = ConnectFilters(m_pGraph, m_pGrabberF, m_pNullF);
	if (FAILED(hr))
		return hr;


	hr = m_pGrabber->SetOneShot(TRUE);
	if (FAILED(hr))
		return hr;

	hr = m_pGrabber->SetBufferSamples(TRUE);
	if (FAILED(hr))
		return hr;

	// Now that the filter has been added to the graph and we have
	// rendered its stream, we can release this reference to the filter.
	pSrcFilter->Release();

	// Start previewing video data
	hr = m_pMC->Run();
	if (FAILED(hr))
	{
		Msg(TEXT("Couldn't run the graph!  hr=0x%x"), hr);
		return hr;
	}

	// Remember current state
	m_psCurrent = RUNNING;

	long evCode;
	hr = m_pME->WaitForCompletion(INFINITE, &evCode);

	return S_OK;
}


HRESULT cDxCapture::FindCaptureDevice(const int cameraIndex, IBaseFilter ** ppSrcFilter)
{
	HRESULT hr = S_OK;
	IBaseFilter * pSrc = NULL;
	IMoniker* pMoniker = NULL;
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pClassEnum = NULL;

	if (!ppSrcFilter)
	{
		return E_POINTER;
	}

	// Create the system device enumerator
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
		IID_ICreateDevEnum, (void **)&pDevEnum);
	if (FAILED(hr))
	{
		Msg(TEXT("Couldn't create system enumerator!  hr=0x%x"), hr);
	}

	// Create an enumerator for the video capture devices

	if (SUCCEEDED(hr))
	{
		hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
		if (FAILED(hr))
		{
			Msg(TEXT("Couldn't create class enumerator!  hr=0x%x"), hr);
		}
	}

	if (SUCCEEDED(hr))
	{
		// If there are no enumerators for the requested type, then 
		// CreateClassEnumerator will succeed, but pClassEnum will be NULL.
		if (pClassEnum == NULL)
		{
			// 			MessageBox(ghApp, TEXT("No video capture device was detected.\r\n\r\n")
			// 				TEXT("This sample requires a video capture device, such as a USB WebCam,\r\n")
			// 				TEXT("to be installed and working properly.  The sample will now close."),
			// 				TEXT("No Video Capture Hardware"), MB_OK | MB_ICONINFORMATION);
			hr = E_FAIL;
		}
	}

	// Use the first video capture device on the device list.
	// Note that if the Next() call succeeds but there are no monikers,
	// it will return S_FALSE (which is not a failure).  Therefore, we
	// check that the return code is S_OK instead of using SUCCEEDED() macro.

	if (SUCCEEDED(hr))
	{
		// cameraIndex번째 카메라를 선택한다.
		int cnt = 0;
		while (cameraIndex >= cnt++)
		{
			hr = pClassEnum->Next(1, &pMoniker, NULL);
			if (hr == S_FALSE)
			{
				Msg(TEXT("Unable to access video capture device!"));
				hr = E_FAIL;
			}
		}
	}

	if (SUCCEEDED(hr))
	{
		// Bind Moniker to a filter object
		hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pSrc);
		if (FAILED(hr))
		{
			Msg(TEXT("Couldn't bind moniker to filter object!  hr=0x%x"), hr);
		}
	}

	// Copy the found filter pointer to the output parameter.
	if (SUCCEEDED(hr))
	{
		*ppSrcFilter = pSrc;
		(*ppSrcFilter)->AddRef();
	}

	SAFE_RELEASE(pSrc);
	SAFE_RELEASE(pMoniker);
	SAFE_RELEASE(pDevEnum);
	SAFE_RELEASE(pClassEnum);

	return hr;
}


HRESULT cDxCapture::GetInterfaces(void)
{
	HRESULT hr;

	// Create the filter graph
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
		IID_IGraphBuilder, (void **)&m_pGraph);
	if (FAILED(hr))
		return hr;

	// Create the capture graph builder
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC,
		IID_ICaptureGraphBuilder2, (void **)&m_pCapture);
	if (FAILED(hr))
		return hr;

	// Create the Sample Grabber filter.
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&m_pGrabberF));
	if (FAILED(hr))
		return hr;

	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&m_pNullF));
	if (FAILED(hr))
		return hr;


	// Obtain interfaces for media control and Video Window
	hr = m_pGraph->QueryInterface(IID_IMediaControl, (LPVOID *)&m_pMC);
	if (FAILED(hr))
		return hr;

	hr = m_pGraph->QueryInterface(IID_IVideoWindow, (LPVOID *)&m_pVW);
	if (FAILED(hr))
		return hr;

	hr = m_pGraph->QueryInterface(IID_IMediaEvent, (LPVOID *)&m_pME);
	if (FAILED(hr))
		return hr;


	hr = m_pGraph->AddFilter(m_pGrabberF, L"Sample Grabber");
	if (FAILED(hr))
		return hr;

	hr = m_pGrabberF->QueryInterface(IID_PPV_ARGS(&m_pGrabber));
	if (FAILED(hr))
		return hr;

	// Set the window handle used to process graph events
	//	hr = m_pME->SetNotifyWindow((OAHWND)ghApp, WM_GRAPHNOTIFY, 0);


	AM_MEDIA_TYPE mt;
	ZeroMemory(&mt, sizeof(mt));
	mt.majortype = MEDIATYPE_Video;
	//mt.subtype = MEDIASUBTYPE_RGB8;
	mt.subtype = MEDIASUBTYPE_RGB24;
	hr = m_pGrabber->SetMediaType(&mt);
	if (FAILED(hr))
		return hr;

	return hr;
}


void cDxCapture::CloseInterfaces(void)
{
	// Stop previewing data
	if (m_pMC)
		m_pMC->StopWhenReady();

	m_psCurrent = STOPPED;

	// Stop receiving events
// 	if (m_pME)
// 		m_pME->SetNotifyWindow(NULL, WM_GRAPHNOTIFY, 0);

	// Relinquish ownership (IMPORTANT!) of the video window.
	// Failing to call put_Owner can lead to assert failures within
	// the video renderer, as it still assumes that it has a valid
	// parent window.
	if (m_pVW)
	{
		m_pVW->put_Visible(OAFALSE);
		m_pVW->put_Owner(NULL);
	}

	// Remove filter graph from the running object table   
	if (m_dwGraphRegister)
		RemoveGraphFromRot(m_dwGraphRegister);

	// Release DirectShow interfaces
	SAFE_RELEASE(m_pMC);
	SAFE_RELEASE(m_pME);
	SAFE_RELEASE(m_pVW);
	SAFE_RELEASE(m_pGraph);
	SAFE_RELEASE(m_pCapture);
	SAFE_RELEASE(m_pGrabber);
	SAFE_RELEASE(m_pGrabberF);
	SAFE_RELEASE(m_pNullF);
}



HRESULT cDxCapture::ChangePreviewState(int nShow)
{
	HRESULT hr = S_OK;

	// If the media control interface isn't ready, don't call it
	if (!m_pMC)
		return S_OK;

	if (nShow)
	{
		if (m_psCurrent != RUNNING)
		{
			// Start previewing video data
			hr = m_pMC->Run();
			m_psCurrent = RUNNING;
		}
	}
	else
	{
		// Stop previewing video data
		hr = m_pMC->StopWhenReady();
		m_psCurrent = STOPPED;
	}

	return hr;
}


HRESULT cDxCapture::AddGraphToRot(IUnknown *pUnkGraph, DWORD *pdwRegister)
{
	IMoniker * pMoniker;
	IRunningObjectTable *pROT;
	WCHAR wsz[128];
	HRESULT hr;

	if (!pUnkGraph || !pdwRegister)
		return E_POINTER;

	if (FAILED(GetRunningObjectTable(0, &pROT)))
		return E_FAIL;

	hr = StringCchPrintfW(wsz, NUMELMS(wsz), L"FilterGraph %08x pid %08x\0", (DWORD_PTR)pUnkGraph,
		GetCurrentProcessId());

	hr = CreateItemMoniker(L"!", wsz, &pMoniker);
	if (SUCCEEDED(hr))
	{
		// Use the ROTFLAGS_REGISTRATIONKEEPSALIVE to ensure a strong reference
		// to the object.  Using this flag will cause the object to remain
		// registered until it is explicitly revoked with the Revoke() method.
		//
		// Not using this flag means that if GraphEdit remotely connects
		// to this graph and then GraphEdit exits, this object registration 
		// will be deleted, causing future attempts by GraphEdit to fail until
		// this application is restarted or until the graph is registered again.
		hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph,
			pMoniker, pdwRegister);
		pMoniker->Release();
	}

	pROT->Release();
	return hr;
}


// Removes a filter graph from the Running Object Table
void cDxCapture::RemoveGraphFromRot(DWORD pdwRegister)
{
	IRunningObjectTable *pROT;

	if (SUCCEEDED(GetRunningObjectTable(0, &pROT)))
	{
		pROT->Revoke(pdwRegister);
		pROT->Release();
	}
}


void cDxCapture::Msg(TCHAR *szFormat, ...)
{
	TCHAR szBuffer[1024];  // Large buffer for long filenames or URLs
	const size_t NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
	const int LASTCHAR = NUMCHARS - 1;

	// Format the input string
	va_list pArgs;
	va_start(pArgs, szFormat);

	// Use a bounded buffer size to prevent buffer overruns.  Limit count to
	// character size minus one to allow for a NULL terminating character.
	(void)StringCchVPrintf(szBuffer, NUMCHARS - 1, szFormat, pArgs);
	va_end(pArgs);

	// Ensure that the formatted string is NULL-terminated
	szBuffer[LASTCHAR] = TEXT('\0');

	MessageBox(NULL, szBuffer, TEXT("PlayCap Message"), MB_OK | MB_ICONERROR);
}


HRESULT cDxCapture::HandleGraphEvent(void)
{
	LONG evCode;
	LONG_PTR evParam1, evParam2;
	HRESULT hr = S_OK;

	if (!m_pME)
		return E_POINTER;

	while (SUCCEEDED(m_pME->GetEvent(&evCode, &evParam1, &evParam2, 0)))
	{
		//
		// Free event parameters to prevent memory leaks associated with
		// event parameter data.  While this application is not interested
		// in the received events, applications should always process them.
		//
		hr = m_pME->FreeEventParams(evCode, evParam1, evParam2);

		// Insert event processing code here, if desired
		switch (evCode)
		{
		case EC_DEVICE_LOST:
		{
			if (evParam2 == 1)
				m_isUnpluggedInit = true;
		}
		break;
		}
	}

	return hr;
}


// 카메라 영상 이미지 버퍼를 리턴한다.
BYTE* cDxCapture::GetCurrentBuffer(long &size)
{
	AutoCS cs(&m_criticalSection);

	if (!m_pGrabber)
		return NULL;

	if (m_isUnpluggedInit)
	{
		// 카메라가 언플러그 된 후, 다시 플러그 되었을 때, 재초기화 를 한다.
		CloseInterfaces();
		Init(m_cameraIndex, m_width, m_height, m_isThreadMode);
		m_isUnpluggedInit = false;
	}

	// Find the required buffer size.
	HRESULT hr;

	long cbBuffer;
	hr = m_pGrabber->GetCurrentBuffer(&cbBuffer, NULL);
	if (FAILED(hr))
		return NULL;

	// 영상 크기가 다르다면, 새 버퍼를 생성한다.
	if (m_frameBufferSize != cbBuffer)
	{
		CoTaskMemFree(m_pFrameBuffer);

		m_pFrameBuffer = (BYTE*)CoTaskMemAlloc(cbBuffer);
		if (!m_pFrameBuffer)
			return NULL;

		m_frameBufferSize = cbBuffer;
	}

	hr = m_pGrabber->GetCurrentBuffer(&cbBuffer, (long*)m_pFrameBuffer);
	if (FAILED(hr))
	{
		CoTaskMemFree(m_pFrameBuffer);
		return NULL;
	}

	m_isUpdateBuffer = true;
	size = cbBuffer;
	return m_pFrameBuffer;
}


// 현재 버퍼의 복사 버퍼를 생성해 리턴한다.
// 이미 생성된 상태라면, 정보를 복사해 리턴한다.
BYTE* cDxCapture::GetCloneBuffer(long &size)
{
	AutoCS cs(&m_criticalSection);

	if (!m_pCloneFrameBuffer)
	{
		long cbSize;
		GetCurrentBuffer(cbSize);

		if (cbSize <= 0)
			return 0;

		m_pCloneFrameBuffer = (BYTE*)CoTaskMemAlloc(cbSize);
		if (!m_pCloneFrameBuffer)
			return NULL;
	}
	
	// 버퍼가 업데이트 되지 않았다면, 업데이트 한다.
	if (!m_isUpdateBuffer)
	{
		long cbSize;
		GetCurrentBuffer(cbSize);
		m_isUpdateBuffer = false;
	}

	size = m_frameBufferSize;
	memcpy(m_pCloneFrameBuffer, m_pFrameBuffer, m_frameBufferSize);
	return m_pCloneFrameBuffer;
}


void cDxCapture::WriteBitmapFromCurrentBuffer(const char* fileName)
{
	AutoCS cs(&m_criticalSection);

	long cbBuffer;
	BYTE* pBuffer = GetCurrentBuffer(cbBuffer);
	if (!pBuffer)
		return;

	HRESULT hr;
	if (!m_mt.pbFormat)
	{
		m_pGrabber->GetConnectedMediaType(&m_mt);
	}

	// Examine the format block.
	if ((m_mt.formattype == FORMAT_VideoInfo) &&
		(m_mt.cbFormat >= sizeof(VIDEOINFOHEADER)) &&
		(m_mt.pbFormat != NULL))
	{
		WriteBitmapFromBuffer(fileName, pBuffer, cbBuffer);
	}
	else
	{
		// Invalid format.
		hr = VFW_E_INVALIDMEDIATYPE;
	}
}


HRESULT cDxCapture::WriteBitmapFromBuffer(const char* fileName, const BYTE *buffer, const long bufferSize)
{
	if (!m_mt.pbFormat)
		m_pGrabber->GetConnectedMediaType(&m_mt);

	VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)m_mt.pbFormat;
	HRESULT hr = WriteBitmap(fileName, &pVih->bmiHeader,
		m_mt.cbFormat - SIZE_PREHEADER, buffer, bufferSize);

	return hr;
}


// 현재 버퍼를 openCV 의 iplImage 로 변환해 리턴한다.
Mat& cDxCapture::GetCloneBufferToImage(const bool cpyImage)
{
	if (m_iplImage.empty())
	{
		// 24bit bitmap 이라고 가정함.
		m_iplImage = Mat(Size(m_width, m_height), CV_8UC(3));
		m_matImage = Mat(Size(m_width, m_height), CV_8UC(3));
	}

	long size;
	BYTE *buffer = GetCloneBuffer(size);
	if (!buffer)
		return m_matImage;

	CopyMemory(m_iplImage.data, buffer, size);
	flip(m_iplImage, m_matImage, 0);

	return m_matImage;
}


Gdiplus::Bitmap* cDxCapture::GetCloneBufferToBitmap()
{
	if (!m_gdiBitmap)
	{
		m_gdiBitmap = ::new Gdiplus::Bitmap(640, 480, PixelFormat24bppRGB);
	}

	long size;
	BYTE *buffer = GetCloneBuffer(size);

	Gdiplus::BitmapData bitmapData;
	bitmapData.Width = m_gdiBitmap->GetWidth();
	bitmapData.Height = m_gdiBitmap->GetHeight();
	bitmapData.Stride = 3 * bitmapData.Width;
	bitmapData.PixelFormat = m_gdiBitmap->GetPixelFormat();
	bitmapData.Scan0 = (VOID*)buffer;
	bitmapData.Reserved = NULL;

	Gdiplus::Status s = m_gdiBitmap->LockBits(
		&Gdiplus::Rect(0, 0, m_gdiBitmap->GetWidth(), m_gdiBitmap->GetHeight()),
		Gdiplus::ImageLockModeWrite | Gdiplus::ImageLockModeUserInputBuf,
		PixelFormat24bppRGB, &bitmapData);
	if (s == Gdiplus::Ok)
		m_gdiBitmap->UnlockBits(&bitmapData);

	m_gdiBitmap->RotateFlip(Gdiplus::Rotate180FlipX);

	return m_gdiBitmap;
}


void cDxCapture::Close()
{
	m_threadLoop = false;
	if (m_handle)
		::WaitForSingleObject(m_handle, 1000);
	m_handle = NULL;

	m_frameBufferSize = 0;

	CoTaskMemFree(m_pFrameBuffer);
	m_pFrameBuffer = NULL;

	CoTaskMemFree(m_pCloneFrameBuffer);
	m_pCloneFrameBuffer = NULL;

	FreeMediaType(m_mt);
	ZeroMemory(&m_mt, sizeof(m_mt));

	CloseInterfaces();
}


// DxShow 쓰레드
unsigned WINAPI DxCaptureThreadFunction(void* arg)
{
	cDxCapture *capture = (cDxCapture*)arg;

	int curT = timeGetTime();
	int oldT = curT;
	int fps = 0;
	while (capture->m_threadLoop)
	{
		// FPS 출력
		fps++;
		curT = timeGetTime();
		if (capture->m_showFrame && (curT - oldT > 1000))
		{
			printf("DxCapture Thread FPS = %d\n", fps);
			oldT = curT;
			fps = 0;
		}

		long size;
		capture->GetCurrentBuffer(size);
		capture->HandleGraphEvent();

		//Sleep(15); // 60 frame 정도의 속도가 나오기위한 딜레이
		Sleep(10); // 60 frame 정도의 속도가 나오기위한 딜레이
	}

	return 0;
}
