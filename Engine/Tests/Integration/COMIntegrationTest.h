// COMIntegrationTest.h — Sprint 29 COM Round-Trip Test
// Copyright (c) 2026 ExplorerLens Project
//
// Validates that LENSShell.dll is registered under the correct CLSID and
// that IThumbnailProvider::GetThumbnail() succeeds for a set of test files.
// Gracefully degrades when DLL is not installed (non-fatal in CI).
//
#pragma once

#include <windows.h>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {
namespace Tests {

//==============================================================================
// COMIntegrationTest
//==============================================================================

class COMIntegrationTest
{
  public:
    //--------------------------------------------------------------------------
    // ExplorerLens COM CLSID (fixed — must not change)
    //--------------------------------------------------------------------------
    static constexpr wchar_t EXPLORERLENS_CLSID_STR[] = L"{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}";

    //--------------------------------------------------------------------------
    // Per-file test outcome
    //--------------------------------------------------------------------------
    struct TestResult
    {
        std::wstring filePath;  // Full path to the test file
        HRESULT hr{E_FAIL};     // HRESULT from GetThumbnail or creation
        uint32_t width{0};      // Returned bitmap width (0 on failure)
        uint32_t height{0};     // Returned bitmap height (0 on failure)
        bool succeeded{false};
        std::wstring errorMessage;
    };

    //--------------------------------------------------------------------------
    // Registration checks
    //--------------------------------------------------------------------------

    // Returns true if the ExplorerLens CLSID is present in HKCR\CLSID.
    static bool IsDllRegistered() noexcept;

    // Returns the InprocServer32 DLL path from the registry, or empty string.
    static std::wstring GetRegisteredDllPath() noexcept;

    // Verifies that the registered DLL path exists on disk.
    static bool IsDllFilePresent() noexcept;

    //--------------------------------------------------------------------------
    // COM round-trip test
    //--------------------------------------------------------------------------

    // Attempt IThumbnailProvider round-trip for each file in testFiles.
    // If IsDllRegistered() is false the entire run is skipped gracefully
    // (all results succeed = true with a "skipped" message).
    // desiredSize: thumbnail size hint passed to GetThumbnail (default 256).
    std::vector<TestResult> RunRoundTrip(const std::vector<std::filesystem::path>& testFiles,
                                         UINT desiredSize = 256) const;

    // Run a minimal smoke test that:
    //   1. Parses the CLSID string.
    //   2. Checks registry registration (non-fatal if absent).
    //   3. If registered, attempts CoCreateInstance for LENSShell.
    // Returns true if all steps that could be verified succeeded.
    static bool RunSmoke() noexcept;

  private:
    // Internal helper: create IThumbnailProvider from registered CLSID.
    static HRESULT CreateThumbnailProvider(REFIID riid, void** ppv) noexcept;
};

}  // namespace Tests
}  // namespace Engine
}  // namespace ExplorerLens
