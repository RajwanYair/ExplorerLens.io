// EngineTestsMacros.h — Shared test infrastructure for EngineTests split files.
// Copyright (c) 2026 ExplorerLens Project
//
// Provides extern declarations for global test counters, the TEST/ASSERT/RUN_TEST
// macros, and the MockDecoder stub — shared between EngineTests.cpp and EngineTests_Mid.cpp.
//
#pragma once

#include "../Core/IThumbnailDecoder.h"

#include <windows.h>
#include <cstddef>   // size_t
#include <cstdint>   // uint32_t
#include <cwchar>    // wcsrchr, _wcsicmp (MSVC extension)
#include <iostream>  // IWYU pragma: keep -- used in TEST() macro (std::cout/wcout)

using namespace ExplorerLens::Engine;  // NOLINT(google-build-using-namespace)

//==============================================================================
// Test Counters (defined in EngineTests.cpp)
//==============================================================================

extern int g_testsRun;
extern int g_testsPassed;
extern int g_testsFailed;

//==============================================================================
// Test Macros
//==============================================================================

#define TEST(name)                                                      \
    void name();                                                        \
    void name##_Runner()                                                \
    {                                                                   \
        std::wcout << L"Running: " << L"" #name << L"..." << std::endl; \
        g_testsRun++;                                                   \
        try {                                                           \
            name();                                                     \
            g_testsPassed++;                                            \
            std::wcout << L" [PASS]" << std::endl;                      \
        } catch (const char* msg) {                                     \
            g_testsFailed++;                                            \
            std::cout << " [FAIL] " << msg << std::endl;                \
        }                                                               \
    }                                                                   \
    void name()

#define ASSERT(expr)                      \
    if (!(expr)) {                        \
        throw "Assertion failed: " #expr; \
    }

#define ASSERT_EQ(a, b)                          \
    if ((a) != (b)) {                            \
        throw "Assertion failed: " #a " == " #b; \
    }

#define ASSERT_NE(a, b)                          \
    if ((a) == (b)) {                            \
        throw "Assertion failed: " #a " != " #b; \
    }

#define ASSERT_NULL(ptr)                               \
    if ((ptr) != nullptr) {                            \
        throw "Assertion failed: " #ptr " == nullptr"; \
    }

#define ASSERT_NOT_NULL(ptr)                           \
    if ((ptr) == nullptr) {                            \
        throw "Assertion failed: " #ptr " != nullptr"; \
    }

#define RUN_TEST(name) name##_Runner()

//==============================================================================
// Mock Decoder for Testing
//==============================================================================

class MockDecoder : public IThumbnailDecoder
{
  public:
    MockDecoder(const wchar_t* name, const wchar_t* ext1, const wchar_t* ext2 = nullptr)
        : m_name(name), m_extensionCount(ext2 ? 2U : 1U)
    {
        m_extensions[0] = ext1;
        m_extensions[1] = ext2;
        m_extensions[2] = nullptr;
    }

    bool CanDecode(const wchar_t* filePath) override
    {
        if (!filePath) {
            return false;
        }
        const wchar_t* ext = wcsrchr(filePath, L'.');
        if (!ext) {
            return false;
        }

        for (size_t i = 0; i < m_extensionCount; i++) {
            if (_wcsicmp(ext, m_extensions[i]) == 0) {
                return true;
            }
        }
        return false;
    }

    HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override
    {
        (void)request;
        result.status = S_OK;
        return S_OK;
    }

    DecoderInfo GetInfo() const override
    {
        DecoderInfo info{};
        info.name = m_name;
        info.version = L"1.0.0";
        info.supportedExtensions = const_cast<const wchar_t**>(m_extensions);
        info.extensionCount = m_extensionCount;
        info.supportsGPU = false;
        info.isArchiveDecoder = false;
        return info;
    }

    const wchar_t* GetName() const override
    {
        return m_name;
    }

    const wchar_t** GetSupportedExtensions() const override
    {
        return const_cast<const wchar_t**>(m_extensions);
    }

    uint32_t GetExtensionCount() const override
    {
        return m_extensionCount;
    }

    bool SupportsGPU() const override
    {
        return false;
    }

    bool IsArchiveDecoder() const override
    {
        return false;
    }

  private:
    const wchar_t* m_name;
    const wchar_t* m_extensions[3] = {};  // NOLINT(cppcoreguidelines-avoid-c-arrays)
    uint32_t m_extensionCount;
};
