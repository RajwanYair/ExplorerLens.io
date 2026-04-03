/******************************************************************************
 * video_thumbnail.cpp
 * Video Frame Extraction Implementation for ExplorerLens
 * Extracts thumbnails from video files using DirectShow
 ******************************************************************************/

#include "video_thumbnail.h"

#include <atlbase.h>
#include <dshow.h>

#include "StdAfx.h"
// qedit.h was removed from Windows SDK 7.0+. The ISampleGrabber interfaces
// are defined inline below as a replacement. Do not attempt to re-add qedit.h.

// ISampleGrabber interface (from qedit.h)
interface ISampleGrabberCB : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE SampleCB(double SampleTime, IMediaSample* pSample) = 0;
    virtual HRESULT STDMETHODCALLTYPE BufferCB(double SampleTime, BYTE* pBuffer, long BufferLen) = 0;
};

interface ISampleGrabber : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE SetOneShot(BOOL OneShot) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetMediaType(const AM_MEDIA_TYPE* pType) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType(AM_MEDIA_TYPE * pType) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetBufferSamples(BOOL BufferThem) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer(long* pBufferSize, long* pBuffer) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetCurrentSample(IMediaSample * *ppSample) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetCallback(ISampleGrabberCB * pCallback, long WhichMethodToCallback) = 0;
};

static const CLSID CLSID_SampleGrabber = {0xC1F400A0, 0x3F08, 0x11d3, {0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37}};
static const IID IID_ISampleGrabber = {0x6B652FFF, 0x11FE, 0x4fce, {0x92, 0xAD, 0x02, 0x66, 0xB5, 0xD7, 0xC7, 0x8F}};

#pragma comment(lib, "strmiids.lib")

namespace ExplorerLens {

bool VideoThumbnail::InitializeCOM()
{
    static bool comInitialized = false;
    if (!comInitialized) {
        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        if (SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE) {
            comInitialized = true;
        }
    }
    return comInitialized;
}

bool VideoThumbnail::IsDirectShowAvailable()
{
    if (!InitializeCOM())
        return false;

    IGraphBuilder* pGraph = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&pGraph);

    if (SUCCEEDED(hr) && pGraph) {
        pGraph->Release();
        return true;
    }

    return false;
}

double VideoThumbnail::GetVideoDuration(const std::wstring& videoPath)
{
    if (!InitializeCOM())
        return 0.0;

    IGraphBuilder* pGraph = nullptr;
    IMediaControl* pControl = nullptr;
    IMediaSeeking* pSeeking = nullptr;

    HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&pGraph);

    if (FAILED(hr) || !pGraph)
        return 0.0;

    hr = pGraph->QueryInterface(IID_IMediaSeeking, (void**)&pSeeking);
    if (FAILED(hr)) {
        pGraph->Release();
        return 0.0;
    }

    hr = pGraph->RenderFile(videoPath.c_str(), NULL);
    if (FAILED(hr)) {
        pSeeking->Release();
        pGraph->Release();
        return 0.0;
    }

    LONGLONG duration = 0;
    hr = pSeeking->GetDuration(&duration);

    pSeeking->Release();
    pGraph->Release();

    if (SUCCEEDED(hr)) {
        // Convert from 100-nanosecond units to seconds
        return static_cast<double>(duration) / 10000000.0;
    }

    return 0.0;
}

HBITMAP VideoThumbnail::ExtractFrame(const std::wstring& videoPath, double position)
{
    if (!InitializeCOM())
        return nullptr;
    if (position < 0.0)
        position = 0.0;
    if (position > 1.0)
        position = 1.0;

    // Declare all variables at function scope to avoid goto issues
    IGraphBuilder* pGraph = nullptr;
    IMediaControl* pControl = nullptr;
    IMediaSeeking* pSeeking = nullptr;
    IMediaEvent* pEvent = nullptr;
    IBaseFilter* pGrabberFilter = nullptr;
    ISampleGrabber* pGrabber = nullptr;
    IPin* pSourcePin = nullptr;
    IPin* pGrabberPin = nullptr;
    HBITMAP hBitmap = nullptr;
    AM_MEDIA_TYPE mt;
    LONGLONG duration = 0;
    LONGLONG seekPos = 0;
    VIDEOINFOHEADER* pVih = nullptr;
    int width = 0;
    int height = 0;
    long bufferSize = 0;
    BYTE* pBuffer = nullptr;
    HDC hdcScreen = nullptr;

    ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));

    HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&pGraph);
    if (FAILED(hr) || !pGraph)
        goto cleanup;

    // Get media control and seeking interfaces
    hr = pGraph->QueryInterface(IID_IMediaControl, (void**)&pControl);
    if (FAILED(hr))
        goto cleanup;

    hr = pGraph->QueryInterface(IID_IMediaSeeking, (void**)&pSeeking);
    if (FAILED(hr))
        goto cleanup;

    // Create Sample Grabber filter
    hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pGrabberFilter);
    if (FAILED(hr))
        goto cleanup;

    hr = pGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&pGrabber);
    if (FAILED(hr))
        goto cleanup;

    // Set media type for RGB24
    mt.majortype = MEDIATYPE_Video;
    mt.subtype = MEDIASUBTYPE_RGB24;

    hr = pGrabber->SetMediaType(&mt);
    if (FAILED(hr))
        goto cleanup;

    // Add grabber to graph
    hr = pGraph->AddFilter(pGrabberFilter, L"Sample Grabber");
    if (FAILED(hr))
        goto cleanup;

    // Render file
    hr = pGraph->RenderFile(videoPath.c_str(), NULL);
    if (FAILED(hr))
        goto cleanup;

    // Set grabber to buffer mode (don't callback)
    hr = pGrabber->SetBufferSamples(TRUE);
    if (FAILED(hr))
        goto cleanup;

    hr = pGrabber->SetOneShot(FALSE);
    if (FAILED(hr))
        goto cleanup;

    // Get duration and seek to position
    hr = pSeeking->GetDuration(&duration);
    if (FAILED(hr))
        goto cleanup;

    seekPos = (LONGLONG)(duration * position);
    hr = pSeeking->SetPositions(&seekPos, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
    if (FAILED(hr))
        goto cleanup;

    // Run the graph briefly to grab a frame
    hr = pControl->Run();
    if (FAILED(hr))
        goto cleanup;

    // Wait for frame using event-based approach (timeout 250ms)
    // This is more efficient than hardcoded 1000ms Sleep
    hr = pControl->QueryInterface(IID_IMediaEvent, (void**)&pEvent);
    if (SUCCEEDED(hr) && pEvent) {
        long evCode;
        // Wait for frame or timeout (250ms is usually enough for first frame)
        pEvent->WaitForCompletion(250, &evCode);
        // Even if timeout, we might have a frame buffered
    }

    hr = pControl->Pause();

    // Get the video format
    hr = pGrabber->GetConnectedMediaType(&mt);
    if (FAILED(hr))
        goto cleanup;

    pVih = (VIDEOINFOHEADER*)mt.pbFormat;
    if (!pVih)
        goto cleanup;

    width = pVih->bmiHeader.biWidth;
    height = pVih->bmiHeader.biHeight;

    // Get buffer size
    hr = pGrabber->GetCurrentBuffer(&bufferSize, NULL);
    if (FAILED(hr) || bufferSize <= 0)
        goto cleanup;

    // Allocate buffer
    pBuffer = new (std::nothrow) BYTE[bufferSize];
    if (!pBuffer)
        goto cleanup;

    // Get the actual buffer
    hr = pGrabber->GetCurrentBuffer(&bufferSize, (long*)pBuffer);
    if (FAILED(hr)) {
        delete[] pBuffer;
        goto cleanup;
    }

    // Create HBITMAP from buffer
    hdcScreen = GetDC(NULL);
    if (hdcScreen) {
        BITMAPINFO bmi;
        ZeroMemory(&bmi, sizeof(BITMAPINFO));
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = height;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 24;
        bmi.bmiHeader.biCompression = BI_RGB;

        void* pBits = NULL;
        hBitmap = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);

        if (hBitmap && pBits) {
            memcpy(pBits, pBuffer, bufferSize);
        }

        ReleaseDC(NULL, hdcScreen);
    }

    delete[] pBuffer;

    // Free media type
    if (mt.cbFormat != 0) {
        CoTaskMemFree((PVOID)mt.pbFormat);
        mt.cbFormat = 0;
        mt.pbFormat = NULL;
    }
    if (mt.pUnk != NULL) {
        mt.pUnk->Release();
        mt.pUnk = NULL;
    }

cleanup:
    if (pControl) {
        pControl->Stop();
        pControl->Release();
    }
    if (pEvent)
        pEvent->Release();
    if (pSeeking)
        pSeeking->Release();
    if (pGrabber)
        pGrabber->Release();
    if (pGrabberFilter)
        pGrabberFilter->Release();
    if (pGraph)
        pGraph->Release();

    return hBitmap;
}

}  // namespace ExplorerLens
