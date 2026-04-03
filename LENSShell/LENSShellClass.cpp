// LENSShellClass.cpp : Implementation of CLENSShellApp and DLL registration.

#include "LENSShellClass.h"

#include <string>

#include "../Engine/Core/Config.h"
#include "LENSShell.h"
#include "StdAfx.h"
#include "metrics_collector.h"

HRESULT CLENSShell::FinalConstruct(void)
{
    ATLTRACE("CLENSShell::FinalConstruct\n");
    DT_LOG_INFO(ExplorerLens::LogCategory::COM, "CLENSShell COM object constructed");

    m_LENS.LoadRegistrySettings();

    // Initialize Engine adapter (v7.0.0 - PRIMARY PATH)
    m_useEngine = false;  // Will be set to true if Engine initializes successfully
    try {
        m_engineAdapter = std::make_unique<ExplorerLens::EngineAdapter>();
        if (m_engineAdapter->Initialize()) {
            m_useEngine = true;
            DT_LOG_INFO(ExplorerLens::LogCategory::ENGINE, "✓ Engine pipeline ACTIVE (v7.0.0) - Legacy fallback "
                                                           "controlled by registry");
        } else {
            DT_LOG_WARNING(ExplorerLens::LogCategory::ENGINE, "Engine adapter initialization failed - check Engine "
                                                              "build and dependencies");
            m_useEngine = false;  // Engine unavailable
        }
    } catch (const std::exception& ex) {
        DT_LOG_ERROR(ExplorerLens::LogCategory::ENGINE,
                     std::string("Engine adapter exception during init: ") + ex.what());
        m_useEngine = false;
    }

    // Log configuration status
    auto& config = ExplorerLens::Engine::GetEngineConfig();
    if (m_useEngine) {
        DT_LOG_INFO(ExplorerLens::LogCategory::ENGINE,
                    std::string("Engine enabled. Legacy fallback: ")
                        + (config.allowLegacyFallback ? "ENABLED" : "DISABLED (Engine-only)"));
    } else {
        DT_LOG_WARNING(ExplorerLens::LogCategory::ENGINE, "Engine unavailable - using legacy implementation only");
    }

    return S_OK;
}

void CLENSShell::FinalRelease(void)
{
    ATLTRACE("CLENSShell::FinalRelease\n");

    // Shutdown Engine adapter
    if (m_engineAdapter) {
        m_engineAdapter->Shutdown();
        m_engineAdapter.reset();
    }

    DT_LOG_INFO(ExplorerLens::LogCategory::COM, "CLENSShell COM object released");
}

// ============================================================================
// Internal implementation without SEH (for C++ RAII objects)
// ============================================================================
HRESULT CLENSShell::GetThumbnail_Internal(UINT cx, HBITMAP* phBmpThumbnail, WTS_ALPHATYPE* pdwAlpha)
{
    PROFILE_FUNCTION();  // Profiling happens here inside SEH-protected block

    // Get file extension for metrics
    std::string fileExt;
    const WCHAR* filePath = m_LENS.GetFilePath();
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
    ExplorerLens::ThumbnailMetricsScope metricsScope(fileExt);

    if (!phBmpThumbnail || !pdwAlpha) {
        DT_LOG_ERROR(ExplorerLens::LogCategory::COM, "GetThumbnail: Invalid parameters");
        return E_POINTER;
    }

    *phBmpThumbnail = nullptr;
    *pdwAlpha = WTSAT_ARGB;  // Use alpha channel for modern Windows

    // Try Engine path first (v7.0.0 - PRIMARY PATH)
    if (m_useEngine && m_engineAdapter && m_engineAdapter->IsInitialized()) {
        DT_LOG_DEBUG(ExplorerLens::LogCategory::ENGINE, std::string("Using Engine pipeline for: ") + fileExt);

        // Use Engine architecture
        HRESULT hr = m_engineAdapter->GenerateThumbnail(m_LENS.GetFilePath(), cx, cx,
                                                        true,  // useGPU
                                                        phBmpThumbnail);

        if (SUCCEEDED(hr)) {
            DT_LOG_DEBUG(ExplorerLens::LogCategory::ENGINE, "Engine thumbnail generation successful");
            metricsScope.SetSuccess(true);
            return hr;
        }

        // Engine failed - check if legacy fallback is allowed
        auto& config = ExplorerLens::Engine::GetEngineConfig();
        if (!config.allowLegacyFallback) {
            // No fallback - Engine is the only path
            DT_LOG_ERROR(ExplorerLens::LogCategory::ENGINE, std::string("Engine thumbnail failed for: ") + fileExt
                                                                + " - Legacy fallback disabled (Engine-only mode)");
            metricsScope.SetSuccess(false);
            return hr;  // Return Engine error
        }

        DT_LOG_WARNING(ExplorerLens::LogCategory::ENGINE,
                       std::string("Engine thumbnail failed for: ") + fileExt + " - Attempting legacy fallback");
    } else {
        DT_LOG_WARNING(ExplorerLens::LogCategory::ENGINE, "Engine not initialized - using legacy implementation");
    }

    // Legacy path (DEPRECATED v7.0.0 - Legacy decoders excluded from build)
    // Only reachable if: Engine unavailable AND LENSShell_LEGACY_DECODERS
    // compiled in In normal builds, only archive/cache helpers remain from legacy
    // code.
    DT_LOG_DEBUG(ExplorerLens::LogCategory::DECODER, std::string("Using legacy decoder for: ") + fileExt);

    SIZE size = {static_cast<LONG>(cx), static_cast<LONG>(cx)};
    DWORD dwFlags = 0;
    m_LENS.OnGetLocation(&size, &dwFlags);

    // Extract thumbnail using legacy logic
    HRESULT hr = m_LENS.OnExtract(phBmpThumbnail);

    if (FAILED(hr)) {
        DT_LOG_HRESULT(ExplorerLens::LogLevel::LVL_ERROR, ExplorerLens::LogCategory::DECODER,
                       std::string("Legacy thumbnail extraction failed for: ") + fileExt, hr);
        metricsScope.SetSuccess(false);
    } else {
        DT_LOG_DEBUG(ExplorerLens::LogCategory::DECODER, std::string("Legacy thumbnail successful for: ") + fileExt);
        metricsScope.SetSuccess(true);
    }

    return hr;
}

// ============================================================================
// IThumbnailProvider::GetThumbnail - Modern Windows 10/11 interface
// SEH wrapper for crash protection
// ============================================================================
STDMETHODIMP CLENSShell::GetThumbnail(UINT cx, HBITMAP* phBmpThumbnail, WTS_ALPHATYPE* pdwAlpha)
{
    // NOTE: PROFILE_FUNCTION() removed from SEH wrapper (incompatible with __try)
    //       Profiling happens inside GetThumbnail_Internal instead

    // ========================================================================
    // CRITICAL: SEH wrapper to prevent Explorer crashes
    // Uses structured exception handling to isolate decoder faults
    // ========================================================================
    __try {
        return GetThumbnail_Internal(cx, phBmpThumbnail, pdwAlpha);

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        // ========================================================================
        // SEH Exception Handler - Prevent Explorer crash
        // ========================================================================
        DWORD exceptionCode = GetExceptionCode();

        // Output to debugger (no C++ objects allowed here)
        char errorMsg[256];
        sprintf_s(errorMsg, sizeof(errorMsg),
                  "[ExplorerLens] CRITICAL: SEH exception in GetThumbnail - Code: "
                  "0x%08X\n",
                  exceptionCode);
        OutputDebugStringA(errorMsg);

        // Clean up any partial resources
        if (phBmpThumbnail && *phBmpThumbnail) {
            DeleteObject(*phBmpThumbnail);
            *phBmpThumbnail = nullptr;
        }

        // Return graceful failure - Explorer continues running
        return E_FAIL;
    }
}

// IInitializeWithStream::Initialize - Stream-based loading for better
// performance
STDMETHODIMP CLENSShell::Initialize(IStream* pstream, DWORD grfMode)
{
    PROFILE_FUNCTION();

    if (!pstream) {
        DT_LOG_ERROR(ExplorerLens::LogCategory::COM, "Initialize: NULL stream pointer");
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
            std::string streamInfo = std::string("Initializing with stream: ") + std::string(CW2A(statstg.pwcsName));
            DT_LOG_DEBUG(ExplorerLens::LogCategory::COM, streamInfo);
            hr = m_LENS.OnLoad(statstg.pwcsName);
            CoTaskMemFree(statstg.pwcsName);
        }
    } else {
        DT_LOG_HRESULT(ExplorerLens::LogLevel::LVL_WARNING, ExplorerLens::LogCategory::COM, "Stream Stat", hr);
    }

    // Note: For true stream-based loading, we would need to refactor
    // CLENSArchive to work with IStream instead of file paths.
    // This is a transitional implementation.

    return hr;
}

// ============================================================================
// IPropertyStore — Explorer Details Pane metadata
// ============================================================================

STDMETHODIMP CLENSShell::GetCount(DWORD* cProps)
{
    // Lazy-initialize properties on first access
    const WCHAR* fp = m_LENS.GetFilePath();
    if (fp) {
        m_propertyStore.InitializeProperties(fp, m_LENS.GetFileType());
    }
    return m_propertyStore.PropertyStore_GetCount(cProps);
}

STDMETHODIMP CLENSShell::GetAt(DWORD iProp, PROPERTYKEY* pkey)
{
    return m_propertyStore.PropertyStore_GetAt(iProp, pkey);
}

STDMETHODIMP CLENSShell::GetValue(REFPROPERTYKEY key, PROPVARIANT* pv)
{
    const WCHAR* fp = m_LENS.GetFilePath();
    if (fp) {
        m_propertyStore.InitializeProperties(fp, m_LENS.GetFileType());
    }
    return m_propertyStore.PropertyStore_GetValue(key, pv);
}

STDMETHODIMP CLENSShell::SetValue(REFPROPERTYKEY key, REFPROPVARIANT propvar)
{
    return m_propertyStore.PropertyStore_SetValue(key, propvar);
}

STDMETHODIMP CLENSShell::Commit()
{
    return m_propertyStore.PropertyStore_Commit();
}

// IPropertyStoreCapabilities
STDMETHODIMP CLENSShell::IsPropertyWritable(REFPROPERTYKEY key)
{
    return m_propertyStore.PropertyStoreCapabilities_IsPropertyWritable(key);
}
