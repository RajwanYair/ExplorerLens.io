// CBXShellClass.cpp : Implementation of CCBXshellApp and DLL registration.

#include "StdAfx.h"
#include "CBXShellClass.h"
#include "CBXShell.h"
#include "metrics_collector.h"
#include "../Engine/Core/Config.h"
#include <string>

HRESULT CCBXShell::FinalConstruct(void) {
  ATLTRACE("CCBXShell::FinalConstruct\n");
  DT_LOG_INFO(DarkThumbs::LogCategory::COM, "CCBXShell COM object constructed");

  m_cbx.LoadRegistrySettings();

  // Initialize Engine adapter (v6.2.0 - PRIMARY PATH)
  m_useEngine = false; // Will be set to true if Engine initializes successfully
  try {
    m_engineAdapter = std::make_unique<DarkThumbs::EngineAdapter>();
    if (m_engineAdapter->Initialize()) {
      m_useEngine = true; 
      DT_LOG_INFO(DarkThumbs::LogCategory::ENGINE, 
        "✓ Engine pipeline ACTIVE (v6.2.0) - Legacy fallback controlled by registry");
    } else {
      DT_LOG_WARNING(DarkThumbs::LogCategory::ENGINE, 
        "Engine adapter initialization failed - check Engine build and dependencies");
      m_useEngine = false; // Engine unavailable
    }
  } catch (const std::exception& ex) {
    DT_LOG_ERROR(DarkThumbs::LogCategory::ENGINE, 
      std::string("Engine adapter exception during init: ") + ex.what());
    m_useEngine = false; 
  }

  // Log configuration status
  auto& config = DarkThumbs::Engine::GetEngineConfig();
  if (m_useEngine) {
    DT_LOG_INFO(DarkThumbs::LogCategory::ENGINE, 
      std::string("Engine enabled. Legacy fallback: ") + 
      (config.allowLegacyFallback ? "ENABLED" : "DISABLED (Engine-only)"));
  } else {
    DT_LOG_WARNING(DarkThumbs::LogCategory::ENGINE,
      "Engine unavailable - using legacy implementation only");
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

  // Get file extension for metrics
  std::string fileExt;
  const WCHAR* filePath = m_cbx.GetFilePath();
  if (filePath) {
    std::wstring wpath(filePath);
    size_t dotPos = wpath.find_last_of(L'.');
    if (dotPos != std::wstring::npos) {
      std::wstring wext = wpath.substr(dotPos + 1);
      // Proper wide-to-narrow conversion for file extensions (ASCII only)
      fileExt.reserve(wext.length());
      for (wchar_t wc : wext) {
        fileExt.push_back(static_cast<char>(wc & 0xFF));
      }
      std::transform(fileExt.begin(), fileExt.end(), fileExt.begin(), ::tolower);
    }
  }
  
  // Track thumbnail generation metrics
  DarkThumbs::ThumbnailMetricsScope metricsScope(fileExt);

  if (!phBmpThumbnail || !pdwAlpha) {
    DT_LOG_ERROR(DarkThumbs::LogCategory::COM,
                 "GetThumbnail: Invalid parameters");
    return E_POINTER;
  }

  *phBmpThumbnail = nullptr;
  *pdwAlpha = WTSAT_ARGB; // Use alpha channel for modern Windows

  // Try Engine path first (v6.2.0 - PRIMARY PATH)
  if (m_useEngine && m_engineAdapter && m_engineAdapter->IsInitialized()) {
    DT_LOG_DEBUG(DarkThumbs::LogCategory::ENGINE, 
      std::string("Using Engine pipeline for: ") + fileExt);
    
    // Use Engine architecture
    HRESULT hr = m_engineAdapter->GenerateThumbnail(
      m_cbx.GetFilePath(), 
      cx, 
      cx, 
      true, // useGPU
      phBmpThumbnail);
    
    if (SUCCEEDED(hr)) {
      DT_LOG_DEBUG(DarkThumbs::LogCategory::ENGINE, 
        "Engine thumbnail generation successful");
      metricsScope.SetSuccess(true);
      return hr;
    }
    
    // Engine failed - check if legacy fallback is allowed
    auto& config = DarkThumbs::Engine::GetEngineConfig();
    if (!config.allowLegacyFallback) {
      // No fallback - Engine is the only path
      DT_LOG_ERROR(DarkThumbs::LogCategory::ENGINE, 
        std::string("Engine thumbnail failed for: ") + fileExt + 
        " - Legacy fallback disabled (Engine-only mode)");
      metricsScope.SetSuccess(false);
      return hr; // Return Engine error
    }
    
    DT_LOG_WARNING(DarkThumbs::LogCategory::ENGINE, 
      std::string("Engine thumbnail failed for: ") + fileExt + 
      " - Attempting legacy fallback");
  } else {
    DT_LOG_WARNING(DarkThumbs::LogCategory::ENGINE,
      "Engine not initialized - using legacy implementation");
  }

  // Legacy path (DEPRECATED - only used if Engine unavailable or explicit fallback enabled)
  DT_LOG_DEBUG(DarkThumbs::LogCategory::DECODER, 
    std::string("Using legacy decoder for: ") + fileExt);
  
  SIZE size = {static_cast<LONG>(cx), static_cast<LONG>(cx)};
  DWORD dwFlags = 0;
  m_cbx.OnGetLocation(&size, &dwFlags);

  // Extract thumbnail using legacy logic
  HRESULT hr = m_cbx.OnExtract(phBmpThumbnail);

  if (FAILED(hr)) {
    DT_LOG_HRESULT(DarkThumbs::LogLevel::LVL_ERROR,
                   DarkThumbs::LogCategory::DECODER, 
                   std::string("Legacy thumbnail extraction failed for: ") + fileExt,
                   hr);
    metricsScope.SetSuccess(false);
  } else {
    DT_LOG_DEBUG(DarkThumbs::LogCategory::DECODER,
      std::string("Legacy thumbnail successful for: ") + fileExt);
    metricsScope.SetSuccess(true);
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
