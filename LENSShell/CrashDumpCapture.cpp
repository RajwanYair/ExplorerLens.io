// CrashDumpCapture.cpp — Unhandled Exception → MiniDump Writer
// Copyright (c) 2026 ExplorerLens Project
//
#include "StdAfx.h"
#include "CrashDumpCapture.h"

#include <dbghelp.h>
#include <pathcch.h>
#include <shlobj.h>
#include <strsafe.h>

#include <ctime>
#include <string>

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "pathcch.lib")

namespace ExplorerLens {
namespace Shell {

bool CrashDumpCapture::s_installed = false;
LPTOP_LEVEL_EXCEPTION_FILTER CrashDumpCapture::s_previousFilter = nullptr;
CrashDumpObserver CrashDumpCapture::s_observer = nullptr;

static std::wstring BuildDumpPath() noexcept
{
    wchar_t tempDir[MAX_PATH] = {};
    if (GetTempPathW(MAX_PATH, tempDir) == 0)
        return L"";

    std::wstring dir = tempDir;
    dir += L"ExplorerLens\\crashes\\";
    CreateDirectoryW((dir.substr(0, dir.size() - 9)).c_str(), nullptr);
    CreateDirectoryW(dir.c_str(), nullptr);

    wchar_t ts[32] = {};
    SYSTEMTIME st{};
    GetLocalTime(&st);
    StringCchPrintfW(ts, ARRAYSIZE(ts), L"%04d%02d%02d_%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour,
                     st.wMinute, st.wSecond);

    return dir + L"ExplorerLens_" + ts + L".dmp";
}

bool CrashDumpCapture::WriteDumpNow(EXCEPTION_POINTERS* ep) noexcept
{
    std::wstring dumpPath = BuildDumpPath();
    if (dumpPath.empty())
        return false;

    HANDLE hFile =
        CreateFileW(dumpPath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (hFile == INVALID_HANDLE_VALUE)
        return false;

    MINIDUMP_EXCEPTION_INFORMATION info{};
    if (ep) {
        info.ThreadId = GetCurrentThreadId();
        info.ExceptionPointers = ep;
        info.ClientPointers = FALSE;
    }

    bool ok = MiniDumpWriteDump(
                  GetCurrentProcess(), GetCurrentProcessId(), hFile,
                  static_cast<MINIDUMP_TYPE>(MiniDumpWithDataSegs | MiniDumpWithHandleData | MiniDumpWithThreadInfo),
                  ep ? &info : nullptr, nullptr, nullptr)
              != FALSE;

    CloseHandle(hFile);

    if (ok && s_observer)
        s_observer(dumpPath);

    return ok;
}

LONG WINAPI CrashDumpCapture::UnhandledExceptionFilter(EXCEPTION_POINTERS* ep) noexcept
{
    WriteDumpNow(ep);
    if (s_previousFilter)
        return s_previousFilter(ep);
    return EXCEPTION_CONTINUE_SEARCH;
}

bool CrashDumpCapture::Install(CrashDumpObserver observer) noexcept
{
    if (s_installed)
        return true;
    s_observer = std::move(observer);
    s_previousFilter = SetUnhandledExceptionFilter(UnhandledExceptionFilter);
    s_installed = true;
    return true;
}

void CrashDumpCapture::Uninstall() noexcept
{
    if (!s_installed)
        return;
    SetUnhandledExceptionFilter(s_previousFilter);
    s_previousFilter = nullptr;
    s_installed = false;
}

bool CrashDumpCapture::IsInstalled() noexcept
{
    return s_installed;
}

std::wstring CrashDumpCapture::GetDumpDirectory() noexcept
{
    wchar_t tempDir[MAX_PATH] = {};
    if (GetTempPathW(MAX_PATH, tempDir) == 0)
        return L"";
    return std::wstring(tempDir) + L"ExplorerLens\\crashes\\";
}

}  // namespace Shell
}  // namespace ExplorerLens
