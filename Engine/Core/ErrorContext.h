//==============================================================================
// ExplorerLens Engine - Error Context Tracker
// Copyright (c) 2026 - ExplorerLens Project
// Task A24: Rich error context for diagnostics
//==============================================================================

#pragma once

#include <windows.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// <summary>
/// Provides detailed error context for debugging
/// </summary>
struct ErrorContext
{
    HRESULT errorCode;
    std::wstring filePath;
    std::wstring decoderName;
    std::wstring operation;
    uint32_t line;
    std::wstring additionalInfo;

    std::wstring ToString() const
    {
        std::wostringstream oss;
        oss << L"[ERROR 0x" << std::hex << std::setw(8) << std::setfill(L'0') << errorCode << L"] ";

        if (!operation.empty()) {
            oss << operation << L" failed";
        }

        if (!decoderName.empty()) {
            oss << L" in " << decoderName;
        }

        if (!filePath.empty()) {
            oss << L"\n File: " << filePath;
        }

        if (line > 0) {
            oss << L"\n Line: " << line;
        }

        if (!additionalInfo.empty()) {
            oss << L"\n Info: " << additionalInfo;
        }

        return oss.str();
    }
};

/// <summary>
/// Thread-local error context stack for tracking error chains
/// </summary>
class ErrorContextManager
{
  public:
    static void PushContext(const wchar_t* operation, const wchar_t* decoderName = nullptr)
    {
        thread_local std::vector<std::pair<std::wstring, std::wstring>> contextStack;
        contextStack.emplace_back(operation ? operation : L"", decoderName ? decoderName : L"");
    }

    static void PopContext()
    {
        thread_local std::vector<std::pair<std::wstring, std::wstring>> contextStack;
        if (!contextStack.empty()) {
            contextStack.pop_back();
        }
    }

    static ErrorContext CreateContext(HRESULT hr, const wchar_t* filePath = nullptr)
    {
        thread_local std::vector<std::pair<std::wstring, std::wstring>> contextStack;

        ErrorContext ctx;
        ctx.errorCode = hr;
        ctx.filePath = filePath ? filePath : L"";
        ctx.line = 0;

        if (!contextStack.empty()) {
            ctx.operation = contextStack.back().first;
            ctx.decoderName = contextStack.back().second;
        }

        // Add Windows error message if available
        if (HRESULT_FACILITY(hr) == FACILITY_WIN32) {
            DWORD win32Error = HRESULT_CODE(hr);
            wchar_t* msgBuf = nullptr;

            FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL, win32Error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&msgBuf, 0, NULL);

            if (msgBuf) {
                ctx.additionalInfo = msgBuf;
                LocalFree(msgBuf);

                // Remove trailing newline
                if (!ctx.additionalInfo.empty() && ctx.additionalInfo.back() == L'\n') {
                    ctx.additionalInfo.pop_back();
                }
                if (!ctx.additionalInfo.empty() && ctx.additionalInfo.back() == L'\r') {
                    ctx.additionalInfo.pop_back();
                }
            }
        }

        return ctx;
    }
};

/// <summary>
/// RAII helper for error context management
/// </summary>
class ScopedErrorContext
{
  public:
    ScopedErrorContext(const wchar_t* operation, const wchar_t* decoderName = nullptr)
    {
        ErrorContextManager::PushContext(operation, decoderName);
    }

    ~ScopedErrorContext()
    {
        ErrorContextManager::PopContext();
    }

    ScopedErrorContext(const ScopedErrorContext&) = delete;
    ScopedErrorContext& operator=(const ScopedErrorContext&) = delete;
};

#define DT_ERROR_SCOPE(op) ExplorerLens::Engine::ScopedErrorContext __dtErrorCtx(op, nullptr)
#define DT_ERROR_SCOPE_DECODER(op, decoder) ExplorerLens::Engine::ScopedErrorContext __dtErrorCtx(op, decoder)

}  // namespace Engine
}  // namespace ExplorerLens
