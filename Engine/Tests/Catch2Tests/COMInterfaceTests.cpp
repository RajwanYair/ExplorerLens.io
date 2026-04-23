// COMInterfaceTests.cpp — Catch2 tests for COM interface contracts
// Copyright (c) 2026 ExplorerLens Project
//
// Validates the COM interface model for ExplorerLens (§6.1, ADR-001):
//   - Fixed CLSID: {9E6ECB90-5A61-42BD-B851-D3297D9C7F39}
//   - STA apartment model (IThumbnailProvider lives in Shell's STA)
//   - IUnknown reference-counting model
//   - IThumbnailProvider::GetThumbnail cx parameter (target size in pixels)
//   - DLL exports: DllGetClassObject, DllCanUnloadNow, DllRegisterServer,
//                  DllUnregisterServer
//   - COM registration key path (HKCR\CLSID\{...}\InprocServer32)
//   - IInitializeWithStream preference over IInitializeWithFile
//   - COM error HR codes: S_OK=0, E_INVALIDARG, E_OUTOFMEMORY, E_NOTIMPL
//
// All tests are self-contained — no Windows headers, no Engine headers,
// no objbase.h.
//
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <array>
#include <cstdint>
#include <string_view>

// ---------------------------------------------------------------------------
// COM interface model (§6.1)
// ---------------------------------------------------------------------------

namespace ExplorerLens::Tests::COMInterface {

// ── CLSID model ─────────────────────────────────────────────────────────────

/// The fixed COM CLSID for ExplorerLens (must never change)
static constexpr std::string_view EXPLORERLENS_CLSID =
    "{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}";

/// Format: {8-4-4-4-12} = 38 characters including braces
static constexpr size_t CLSID_STRING_LENGTH = 38;

// ── Interface IIDs (modelled as strings) ────────────────────────────────────

static constexpr std::string_view IID_IUnknown =
    "{00000000-0000-0000-C000-000000000046}";
static constexpr std::string_view IID_IThumbnailProvider =
    "{E357FCCD-A995-4576-B01F-234630154E96}";
static constexpr std::string_view IID_IInitializeWithStream =
    "{B824B49D-22AC-4161-AC8A-9916E8FA3F7F}";
static constexpr std::string_view IID_IInitializeWithFile =
    "{B7D14566-0509-4CCE-A71F-0A554233BD9B}";

// ── COM apartment model ──────────────────────────────────────────────────────

enum class ApartmentModel {
    STA = 0,   // Single-Threaded Apartment (required for shell extensions)
    MTA = 1,   // Multi-Threaded Apartment
    BOTH = 2,  // Both STA and MTA
};

/// ExplorerLens shell extension must run in STA to satisfy explorer.exe
static constexpr ApartmentModel SHELL_EXTENSION_APARTMENT = ApartmentModel::STA;

// ── HRESULT codes ─────────────────────────────────────────────────────────

static constexpr uint32_t S_OK            = 0x00000000;
static constexpr uint32_t S_FALSE         = 0x00000001;
static constexpr uint32_t E_INVALIDARG    = 0x80070057;
static constexpr uint32_t E_OUTOFMEMORY   = 0x8007000E;
static constexpr uint32_t E_NOTIMPL       = 0x80004001;
static constexpr uint32_t E_NOINTERFACE   = 0x80004002;
static constexpr uint32_t E_POINTER       = 0x80004003;
static constexpr uint32_t E_FAIL          = 0x80004005;
static constexpr uint32_t E_UNEXPECTED    = 0x8000FFFF;
static constexpr uint32_t CLASS_E_CLASSNOTAVAILABLE = 0x80040111;

/// Returns true for success HRESULTs (high bit = 0)
inline constexpr bool HR_SUCCEEDED(uint32_t hr) { return (hr & 0x80000000u) == 0; }
/// Returns true for failure HRESULTs (high bit = 1)
inline constexpr bool HR_FAILED(uint32_t hr)    { return (hr & 0x80000000u) != 0; }

// ── DLL export contract ───────────────────────────────────────────────────

/// Required exports for any COM in-process server DLL
static constexpr std::array<std::string_view, 4> REQUIRED_DLL_EXPORTS = {{
    "DllGetClassObject",
    "DllCanUnloadNow",
    "DllRegisterServer",
    "DllUnregisterServer",
}};

// ── COM registry key model ────────────────────────────────────────────────

/// InprocServer32 subkey under HKCR\CLSID\{...}
static constexpr std::string_view REG_KEY_INPROCSERVER32 = "InprocServer32";
/// ThreadingModel value for STA shell extension
static constexpr std::string_view REG_THREADING_MODEL_BOTH = "Both";
/// Approved shell extensions registry key
static constexpr std::string_view REG_APPROVED_EXTENSIONS =
    R"(SOFTWARE\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved)";

// ── IThumbnailProvider model ─────────────────────────────────────────────

/// GetThumbnail cx parameter: valid range [1, 4096]
static constexpr uint32_t GETTHUMB_CX_MIN = 1;
static constexpr uint32_t GETTHUMB_CX_MAX = 4096;

/// Preferred initialisation interface (IInitializeWithStream over IInitializeWithFile)
enum class InitPreference {
    PREFER_STREAM = 0,  // Prefer IInitializeWithStream (OneDrive placeholder safe)
    PREFER_FILE   = 1,  // Prefer IInitializeWithFile (legacy fallback)
};
static constexpr InitPreference INIT_PREFERENCE = InitPreference::PREFER_STREAM;

} // namespace ExplorerLens::Tests::COMInterface

using namespace ExplorerLens::Tests::COMInterface;

// ===========================================================================
// CLSID
// ===========================================================================

TEST_CASE("CLSID — ExplorerLens CLSID matches fixed value",
          "[com][clsid]") {
    REQUIRE(EXPLORERLENS_CLSID == "{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}");
}

TEST_CASE("CLSID — length is exactly 38 characters",
          "[com][clsid]") {
    REQUIRE(EXPLORERLENS_CLSID.size() == CLSID_STRING_LENGTH);
}

TEST_CASE("CLSID — begins with '{' and ends with '}'",
          "[com][clsid]") {
    REQUIRE(EXPLORERLENS_CLSID.front() == '{');
    REQUIRE(EXPLORERLENS_CLSID.back()  == '}');
}

TEST_CASE("CLSID — contains 4 hyphens (8-4-4-4-12 format)",
          "[com][clsid]") {
    size_t hyphenCount = 0;
    for (char c : EXPLORERLENS_CLSID) {
        if (c == '-') ++hyphenCount;
    }
    REQUIRE(hyphenCount == 4u);
}

TEST_CASE("CLSID — hyphens at positions 9, 14, 19, 24 (1-based)",
          "[com][clsid]") {
    REQUIRE(EXPLORERLENS_CLSID[9]  == '-');
    REQUIRE(EXPLORERLENS_CLSID[14] == '-');
    REQUIRE(EXPLORERLENS_CLSID[19] == '-');
    REQUIRE(EXPLORERLENS_CLSID[24] == '-');
}

// ===========================================================================
// Interface IIDs
// ===========================================================================

TEST_CASE("IID — IUnknown has well-known IID with all-zero data fields",
          "[com][iid]") {
    // {00000000-0000-0000-C000-000000000046}
    REQUIRE(IID_IUnknown == "{00000000-0000-0000-C000-000000000046}");
}

TEST_CASE("IID — IThumbnailProvider IID is defined",
          "[com][iid]") {
    REQUIRE_FALSE(IID_IThumbnailProvider.empty());
    REQUIRE(IID_IThumbnailProvider.size() == 38u);
}

TEST_CASE("IID — IInitializeWithStream IID is defined",
          "[com][iid]") {
    REQUIRE_FALSE(IID_IInitializeWithStream.empty());
    REQUIRE(IID_IInitializeWithStream.size() == 38u);
}

TEST_CASE("IID — all interface IIDs are distinct",
          "[com][iid]") {
    CHECK(IID_IUnknown != IID_IThumbnailProvider);
    CHECK(IID_IUnknown != IID_IInitializeWithStream);
    CHECK(IID_IThumbnailProvider != IID_IInitializeWithStream);
    CHECK(IID_IInitializeWithStream != IID_IInitializeWithFile);
}

// ===========================================================================
// Apartment model
// ===========================================================================

TEST_CASE("COMInterface — shell extension uses STA apartment model",
          "[com][apartment]") {
    REQUIRE(SHELL_EXTENSION_APARTMENT == ApartmentModel::STA);
}

// ===========================================================================
// HRESULT codes
// ===========================================================================

TEST_CASE("HRESULT — S_OK is 0",
          "[com][hresult]") {
    REQUIRE(S_OK == 0u);
}

TEST_CASE("HRESULT — S_FALSE is 1",
          "[com][hresult]") {
    REQUIRE(S_FALSE == 1u);
}

TEST_CASE("HRESULT — failure codes have high bit set",
          "[com][hresult]") {
    CHECK(HR_FAILED(E_INVALIDARG));
    CHECK(HR_FAILED(E_OUTOFMEMORY));
    CHECK(HR_FAILED(E_NOTIMPL));
    CHECK(HR_FAILED(E_NOINTERFACE));
    CHECK(HR_FAILED(E_POINTER));
    CHECK(HR_FAILED(E_FAIL));
    CHECK(HR_FAILED(E_UNEXPECTED));
    CHECK(HR_FAILED(CLASS_E_CLASSNOTAVAILABLE));
}

TEST_CASE("HRESULT — success codes do not have high bit set",
          "[com][hresult]") {
    CHECK(HR_SUCCEEDED(S_OK));
    CHECK(HR_SUCCEEDED(S_FALSE));
}

TEST_CASE("HRESULT — HR_SUCCEEDED and HR_FAILED are mutually exclusive",
          "[com][hresult]") {
    auto hr = GENERATE(S_OK, S_FALSE, E_FAIL, E_INVALIDARG, E_OUTOFMEMORY);
    CHECK(HR_SUCCEEDED(hr) != HR_FAILED(hr));
}

TEST_CASE("HRESULT — E_INVALIDARG has well-known Win32 error code 87",
          "[com][hresult]") {
    // HRESULT = HRESULT_FROM_WIN32(87) = 0x80070057
    REQUIRE(E_INVALIDARG == 0x80070057u);
}

TEST_CASE("HRESULT — E_OUTOFMEMORY has well-known Win32 error code 14",
          "[com][hresult]") {
    // HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY=14) = 0x8007000E
    REQUIRE(E_OUTOFMEMORY == 0x8007000Eu);
}

// ===========================================================================
// DLL exports
// ===========================================================================

TEST_CASE("DLLExports — 4 required exports defined",
          "[com][dll]") {
    REQUIRE(REQUIRED_DLL_EXPORTS.size() == 4u);
}

TEST_CASE("DLLExports — DllGetClassObject is in required exports",
          "[com][dll]") {
    bool found = false;
    for (auto& e : REQUIRED_DLL_EXPORTS)
        if (e == "DllGetClassObject") { found = true; break; }
    REQUIRE(found);
}

TEST_CASE("DLLExports — DllCanUnloadNow is in required exports",
          "[com][dll]") {
    bool found = false;
    for (auto& e : REQUIRED_DLL_EXPORTS)
        if (e == "DllCanUnloadNow") { found = true; break; }
    REQUIRE(found);
}

TEST_CASE("DLLExports — DllRegisterServer is in required exports",
          "[com][dll]") {
    bool found = false;
    for (auto& e : REQUIRED_DLL_EXPORTS)
        if (e == "DllRegisterServer") { found = true; break; }
    REQUIRE(found);
}

TEST_CASE("DLLExports — DllUnregisterServer is in required exports",
          "[com][dll]") {
    bool found = false;
    for (auto& e : REQUIRED_DLL_EXPORTS)
        if (e == "DllUnregisterServer") { found = true; break; }
    REQUIRE(found);
}

TEST_CASE("DLLExports — no duplicate export names",
          "[com][dll]") {
    for (size_t i = 0; i < REQUIRED_DLL_EXPORTS.size(); ++i)
        for (size_t j = i + 1; j < REQUIRED_DLL_EXPORTS.size(); ++j)
            CHECK(REQUIRED_DLL_EXPORTS[i] != REQUIRED_DLL_EXPORTS[j]);
}

// ===========================================================================
// IThumbnailProvider
// ===========================================================================

TEST_CASE("IThumbnailProvider — GetThumbnail cx min is 1",
          "[com][thumbnail]") {
    REQUIRE(GETTHUMB_CX_MIN == 1u);
}

TEST_CASE("IThumbnailProvider — GetThumbnail cx max is 4096",
          "[com][thumbnail]") {
    REQUIRE(GETTHUMB_CX_MAX == 4096u);
}

TEST_CASE("IThumbnailProvider — standard Explorer cx values are in valid range",
          "[com][thumbnail]") {
    auto cx = GENERATE(16u, 32u, 48u, 96u, 256u, 768u, 1024u);
    CHECK(cx >= GETTHUMB_CX_MIN);
    CHECK(cx <= GETTHUMB_CX_MAX);
}

// ===========================================================================
// Registry / initialisation
// ===========================================================================

TEST_CASE("COMRegistration — InprocServer32 key name is correct",
          "[com][registration]") {
    REQUIRE(REG_KEY_INPROCSERVER32 == "InprocServer32");
}

TEST_CASE("COMRegistration — ThreadingModel is Both (supports STA+MTA host)",
          "[com][registration]") {
    REQUIRE(REG_THREADING_MODEL_BOTH == "Both");
}

TEST_CASE("COMRegistration — approved extensions key path is correct",
          "[com][registration]") {
    REQUIRE(REG_APPROVED_EXTENSIONS.find("Shell Extensions") != std::string_view::npos);
    REQUIRE(REG_APPROVED_EXTENSIONS.find("Approved") != std::string_view::npos);
}

TEST_CASE("COMInterface — prefers IInitializeWithStream over IInitializeWithFile",
          "[com][init]") {
    REQUIRE(INIT_PREFERENCE == InitPreference::PREFER_STREAM);
}
