
#include "stdafx.h"
#include <assert.h>
#include "dxcaptureutil.h"
#include <assert.h>

#pragma include_alias( "dxtrans.h", "qedit.h" )
#define __IDxtCompositor_INTERFACE_DEFINED__
#define __IDxtAlphaSetter_INTERFACE_DEFINED__
#define __IDxtJpeg_INTERFACE_DEFINED__
#define __IDxtKey_INTERFACE_DEFINED__
//#include <C:\Program Files\Microsoft SDKs\Windows\v6.0\Include\qedit.h>
#include "qedit.h"


// Query whether a pin is connected to another pin.
//
// Note: This function does not return a pointer to the connected pin.

HRESULT IsPinConnected(IPin *pPin, BOOL *pResult)
{
	IPin *pTmp = NULL;
	HRESULT hr = pPin->ConnectedTo(&pTmp);
	if (SUCCEEDED(hr))
	{
		*pResult = TRUE;
	}
	else if (hr == VFW_E_NOT_CONNECTED)
	{
		// The pin is not connected. This is not an error for our purposes.
		*pResult = FALSE;
		hr = S_OK;
	}

	SafeRelease(&pTmp);
	return hr;
}


// Query whether a pin has a specified direction (input / output)
HRESULT IsPinDirection(IPin *pPin, PIN_DIRECTION dir, BOOL *pResult)
{
	PIN_DIRECTION pinDir;
	HRESULT hr = pPin->QueryDirection(&pinDir);
	if (SUCCEEDED(hr))
	{
		*pResult = (pinDir == dir);
	}
	return hr;
}


// Match a pin by pin direction and connection state.
HRESULT MatchPin(IPin *pPin, PIN_DIRECTION direction, BOOL bShouldBeConnected, BOOL *pResult)
{
	assert(pResult != NULL);

	BOOL bMatch = FALSE;
	BOOL bIsConnected = FALSE;

	HRESULT hr = IsPinConnected(pPin, &bIsConnected);
	if (SUCCEEDED(hr))
	{
		if (bIsConnected == bShouldBeConnected)
		{
			hr = IsPinDirection(pPin, direction, &bMatch);
		}
	}

	if (SUCCEEDED(hr))
	{
		*pResult = bMatch;
	}
	return hr;
}


// Return the first unconnected input pin or output pin.
HRESULT FindUnconnectedPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin)
{
	IEnumPins *pEnum = NULL;
	IPin *pPin = NULL;
	BOOL bFound = FALSE;

	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (FAILED(hr))
	{
		goto done;
	}

	while (S_OK == pEnum->Next(1, &pPin, NULL))
	{
		hr = MatchPin(pPin, PinDir, FALSE, &bFound);
		if (FAILED(hr))
		{
			goto done;
		}
		if (bFound)
		{
			*ppPin = pPin;
			(*ppPin)->AddRef();
			break;
		}
		SafeRelease(&pPin);
	}

	if (!bFound)
	{
		hr = VFW_E_NOT_FOUND;
	}

done:
	SafeRelease(&pPin);
	SafeRelease(&pEnum);
	return hr;
}


HRESULT ConnectFilters(
	IGraphBuilder *pGraph, // Filter Graph Manager.
	IPin *pOut,            // Output pin on the upstream filter.
	IBaseFilter *pDest)    // Downstream filter.
{
	IPin *pIn = NULL;

	// Find an input pin on the downstream filter.
	HRESULT hr = FindUnconnectedPin(pDest, PINDIR_INPUT, &pIn);
	if (SUCCEEDED(hr))
	{
		// Try to connect them.
		hr = pGraph->Connect(pOut, pIn);
		pIn->Release();
	}
	return hr;
}

// Connect filter to input pin.

HRESULT ConnectFilters(IGraphBuilder *pGraph, IBaseFilter *pSrc, IPin *pIn)
{
	IPin *pOut = NULL;

	// Find an output pin on the upstream filter.
	HRESULT hr = FindUnconnectedPin(pSrc, PINDIR_OUTPUT, &pOut);
	if (SUCCEEDED(hr))
	{
		// Try to connect them.
		hr = pGraph->Connect(pOut, pIn);
		pOut->Release();
	}
	return hr;
}

// Connect filter to filter

HRESULT ConnectFilters(IGraphBuilder *pGraph, IBaseFilter *pSrc, IBaseFilter *pDest)
{
	IPin *pOut = NULL;

	// Find an output pin on the first filter.
	HRESULT hr = FindUnconnectedPin(pSrc, PINDIR_OUTPUT, &pOut);
	if (SUCCEEDED(hr))
	{
		hr = ConnectFilters(pGraph, pOut, pDest);
		pOut->Release();
	}
	return hr;
}


HRESULT GrabVideoBitmap(PCWSTR pszVideoFile, const char *pszBitmapFile)
{
	IGraphBuilder *pGraph = NULL;
	IMediaControl *pControl = NULL;
	IMediaEventEx *pEvent = NULL;
	IBaseFilter *pGrabberF = NULL;
	ISampleGrabber *pGrabber = NULL;
	IBaseFilter *pSourceF = NULL;
	IEnumPins *pEnum = NULL;
	IPin *pPin = NULL;
	IBaseFilter *pNullF = NULL;

	BYTE *pBuffer = NULL;

	HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL,
		CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pGraph));
	if (FAILED(hr))
	{
		goto done;
	}

	hr = pGraph->QueryInterface(IID_PPV_ARGS(&pControl));
	if (FAILED(hr))
	{
		goto done;
	}

	hr = pGraph->QueryInterface(IID_PPV_ARGS(&pEvent));
	if (FAILED(hr))
	{
		goto done;
	}

	// Create the Sample Grabber filter.
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pGrabberF));
	if (FAILED(hr))
	{
		goto done;
	}

	hr = pGraph->AddFilter(pGrabberF, L"Sample Grabber");
	if (FAILED(hr))
	{
		goto done;
	}

	hr = pGrabberF->QueryInterface(IID_PPV_ARGS(&pGrabber));
	if (FAILED(hr))
	{
		goto done;
	}

	AM_MEDIA_TYPE mt;
	ZeroMemory(&mt, sizeof(mt));
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_RGB24;

	hr = pGrabber->SetMediaType(&mt);
	if (FAILED(hr))
	{
		goto done;
	}

	hr = pGraph->AddSourceFilter(pszVideoFile, L"Source", &pSourceF);
	if (FAILED(hr))
	{
		goto done;
	}

	hr = pSourceF->EnumPins(&pEnum);
	if (FAILED(hr))
	{
		goto done;
	}

	while (S_OK == pEnum->Next(1, &pPin, NULL))
	{
		hr = ConnectFilters(pGraph, pPin, pGrabberF);
		SafeRelease(&pPin);
		if (SUCCEEDED(hr))
		{
			break;
		}
	}

	if (FAILED(hr))
	{
		goto done;
	}

	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pNullF));
	if (FAILED(hr))
	{
		goto done;
	}

	hr = pGraph->AddFilter(pNullF, L"Null Filter");
	if (FAILED(hr))
	{
		goto done;
	}

	hr = ConnectFilters(pGraph, pGrabberF, pNullF);
	if (FAILED(hr))
	{
		goto done;
	}

	hr = pGrabber->SetOneShot(TRUE);
	if (FAILED(hr))
	{
		goto done;
	}

	hr = pGrabber->SetBufferSamples(TRUE);
	if (FAILED(hr))
	{
		goto done;
	}

	hr = pControl->Run();
	if (FAILED(hr))
	{
		goto done;
	}

	long evCode;
	hr = pEvent->WaitForCompletion(INFINITE, &evCode);

	// Find the required buffer size.
	long cbBuffer;
	hr = pGrabber->GetCurrentBuffer(&cbBuffer, NULL);
	if (FAILED(hr))
	{
		goto done;
	}

	pBuffer = (BYTE*)CoTaskMemAlloc(cbBuffer);
	if (!pBuffer)
	{
		hr = E_OUTOFMEMORY;
		goto done;
	}

	hr = pGrabber->GetCurrentBuffer(&cbBuffer, (long*)pBuffer);
	if (FAILED(hr))
	{
		goto done;
	}

	hr = pGrabber->GetConnectedMediaType(&mt);
	if (FAILED(hr))
	{
		goto done;
	}

	// Examine the format block.
	if ((mt.formattype == FORMAT_VideoInfo) &&
		(mt.cbFormat >= sizeof(VIDEOINFOHEADER)) &&
		(mt.pbFormat != NULL))
	{
		VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)mt.pbFormat;

		hr = WriteBitmap(pszBitmapFile, &pVih->bmiHeader,
			mt.cbFormat - SIZE_PREHEADER, pBuffer, cbBuffer);
	}
	else
	{
		// Invalid format.
		hr = VFW_E_INVALIDMEDIATYPE;
	}

	FreeMediaType(mt);

done:
	CoTaskMemFree(pBuffer);
	SafeRelease(&pPin);
	SafeRelease(&pEnum);
	SafeRelease(&pNullF);
	SafeRelease(&pSourceF);
	SafeRelease(&pGrabber);
	SafeRelease(&pGrabberF);
	SafeRelease(&pControl);
	SafeRelease(&pEvent);
	SafeRelease(&pGraph);
	return hr;
};

// Writes a bitmap file
//  pszFileName:  Output file name.
//  pBMI:         Bitmap format information (including pallete).
//  cbBMI:        Size of the BITMAPINFOHEADER, including palette, if present.
//  pData:        Pointer to the bitmap bits.
//  cbData        Size of the bitmap, in bytes.

HRESULT WriteBitmap(const char *pszFileName, BITMAPINFOHEADER *pBMI, size_t cbBMI,
	const BYTE *pData, const size_t cbData)
{
	HANDLE hFile = CreateFileA(pszFileName, GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, 0, NULL);
	if (hFile == NULL)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	BITMAPFILEHEADER bmf = {};

	bmf.bfType = 'MB';
	bmf.bfSize = cbBMI + cbData + sizeof(bmf);
	bmf.bfOffBits = sizeof(bmf) + cbBMI;

	DWORD cbWritten = 0;
	BOOL result = WriteFile(hFile, &bmf, sizeof(bmf), &cbWritten, NULL);
	if (result)
	{
		result = WriteFile(hFile, pBMI, cbBMI, &cbWritten, NULL);
	}
	if (result)
	{
		result = WriteFile(hFile, pData, cbData, &cbWritten, NULL);
	}

	HRESULT hr = result ? S_OK : HRESULT_FROM_WIN32(GetLastError());

	CloseHandle(hFile);

	return hr;
}


// Release the format block for a media type.
void FreeMediaType(AM_MEDIA_TYPE& mt)
{
	if (mt.cbFormat != 0)
	{
		CoTaskMemFree((PVOID)mt.pbFormat);
		mt.cbFormat = 0;
		mt.pbFormat = NULL;
	}
	if (mt.pUnk != NULL)
	{
		// pUnk should not be used.
		mt.pUnk->Release();
		mt.pUnk = NULL;
	}
}
