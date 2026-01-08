// CBXShellClass.cpp : Implementation of CCBXshellApp and DLL registration.

#include "StdAfx.h"
#include "CBXShellClass.h"
#include "CBXShell.h"
#include <string>

HRESULT CCBXShell::FinalConstruct(void) {
  ATLTRACE("CCBXShell::FinalConstruct\n");
  DT_LOG_INFO(DarkThumbs::LogCategory::COM, "CCBXShell COM object constructed");

  m_cbx.LoadRegistrySettings();

  // Initialize Engine adapter (v5.3.0 - ACTIVE)
  m_useEngine = false; // Default to legacy fallback
  try {
    m_engineAdapter = std::make_unique<DarkThumbs::EngineAdapter>();
    if (m_engineAdapter->Initialize()) {
      m_useEngine = true; // ✅ Engine pipeline now active!
      DT_LOG_INFO(DarkThumbs::LogCategory::ENGINE, "✓ Engine adapter initialized - v5.3.0 active");
    } else {
      DT_LOG_WARNING(DarkThumbs::LogCategory::ENGINE, "Engine adapter init failed, using legacy code");
    }
  } catch (const std::exception& ex) {
    DT_LOG_ERROR(DarkThumbs::LogCategory::ENGINE, 
      std::string("Engine adapter exception: ") + ex.what());
    m_useEngine = false; // Fallback to legacy on exception
  }

  return S_OK;
}

void CCBXShell::FinalRelease(void) {
  ATLTRACE("CCBXShell::FinalRelease\n");
  
  // Shutdown Engine adapter
  if (m_engineAdapter) {
    m_engineAdapter->Shutdown();
    m_engineAdapter.reset();
  }
  
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

  // Try Engine path first (v5.3.0)
  if (m_useEngine && m_engineAdapter && m_engineAdapter->IsInitialized()) {
    // Use new Engine architecture
    HRESULT hr = m_engineAdapter->GenerateThumbnail(
      m_cbx.GetFilePath(), 
      cx, 
      cx, 
      true, // useGPU
      phBmpThumbnail);
    
    if (SUCCEEDED(hr)) {
      return hr;
    }
    
    // Fallback to legacy if Engine fails
    DT_LOG_WARNING(DarkThumbs::LogCategory::ENGINE, 
      "Engine thumbnail failed, falling back to legacy");
  }

  // Legacy path (existing implementation)
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
