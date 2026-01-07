// CBXShellClass.cpp : Implementation of CCBXshellApp and DLL registration.

#include "CBXShellClass.h"
#include "CBXShell.h"
#include "StdAfx.h"
#include <string>

HRESULT CCBXShell::FinalConstruct(void) {
  ATLTRACE("CCBXShell::FinalConstruct\n");
  DT_LOG_INFO(DarkThumbs::LogCategory::COM, "CCBXShell COM object constructed");

  m_cbx.LoadRegistrySettings();

  return S_OK;
}

void CCBXShell::FinalRelease(void) {
  ATLTRACE("CCBXShell::FinalRelease\n");
  DT_LOG_INFO(DarkThumbs::LogCategory::COM, "CCBXShell COM object released");
}

// IThumbnailProvider::GetThumbnail - Modern Windows 10/11 interface
STDMETHODIMP CCBXShell::GetThumbnail(UINT cx, HBITMAP *phBmpThumbnail,
                                     WTS_ALPHATYPE *pdwAlpha) {
  PROFILE_FUNCTION();

  if (!phBmpThumbnail || !pdwAlpha) {
    DT_LOG_ERROR(DarkThumbs::LogCategory::COM,
                 "GetThumbnail: Invalid parameters");
    return E_POINTER;
  }

  *phBmpThumbnail = nullptr;
  *pdwAlpha = WTSAT_ARGB; // Use alpha channel for modern Windows

  // Set thumbnail size
  SIZE size = {static_cast<LONG>(cx), static_cast<LONG>(cx)};
  DWORD dwFlags = 0;
  m_cbx.OnGetLocation(&size, &dwFlags);

  // Extract thumbnail using existing logic
  HRESULT hr = m_cbx.OnExtract(phBmpThumbnail);

  if (FAILED(hr)) {
    DT_LOG_HRESULT(DarkThumbs::LogLevel::LVL_ERROR,
                   DarkThumbs::LogCategory::DECODER, "Thumbnail extraction",
                   hr);
  }

  return hr;
}

// IInitializeWithStream::Initialize - Stream-based loading for better
// performance
STDMETHODIMP CCBXShell::Initialize(IStream *pstream, DWORD grfMode) {
  PROFILE_FUNCTION();

  if (!pstream) {
    DT_LOG_ERROR(DarkThumbs::LogCategory::COM,
                 "Initialize: NULL stream pointer");
    return E_POINTER;
  }

  // Store stream for later use
  m_spStream = pstream;

  // Get stream statistics to retrieve file information
  STATSTG statstg = {0};
  HRESULT hr = pstream->Stat(&statstg, STATFLAG_DEFAULT);
  if (SUCCEEDED(hr)) {
    // Use the stream name if available
    if (statstg.pwcsName) {
      std::string streamInfo = std::string("Initializing with stream: ") +
                               std::string(CW2A(statstg.pwcsName));
      DT_LOG_DEBUG(DarkThumbs::LogCategory::COM, streamInfo);
      hr = m_cbx.OnLoad(statstg.pwcsName);
      CoTaskMemFree(statstg.pwcsName);
    }
  } else {
    DT_LOG_HRESULT(DarkThumbs::LogLevel::LVL_WARNING,
                   DarkThumbs::LogCategory::COM, "Stream Stat", hr);
  }

  // Note: For true stream-based loading, we would need to refactor
  // CCBXArchive to work with IStream instead of file paths.
  // This is a transitional implementation.

  return hr;
}
