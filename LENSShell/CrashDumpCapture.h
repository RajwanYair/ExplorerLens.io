// CrashDumpCapture.h — Unhandled Exception → MiniDump Writer
// Copyright (c) 2026 ExplorerLens Project
//
// Sprint 12 (v15.3.0 "Zenith-T"): Installs a process-wide unhandled exception
// filter that writes a MiniDump to %TEMP%\ExplorerLens\crashes\ whenever an
// unhandled SEH exception occurs inside LENSShell.dll.  The dump path is
// reported back to the caller via the registered observer callback so it can
// be forwarded to the observability pipeline.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <functional>
#include <string>

namespace ExplorerLens {
namespace Shell {

using CrashDumpObserver = std::function<void(const std::wstring& dumpPath)>;

class CrashDumpCapture
{
  public:
    static bool Install(CrashDumpObserver observer = nullptr) noexcept;
    static void Uninstall() noexcept;

    static bool IsInstalled() noexcept;

    static std::wstring GetDumpDirectory() noexcept;

    static bool WriteDumpNow(EXCEPTION_POINTERS* ep = nullptr) noexcept;

  private:
    static LONG WINAPI UnhandledExceptionFilter(EXCEPTION_POINTERS* ep) noexcept;

    static bool s_installed;
    static LPTOP_LEVEL_EXCEPTION_FILTER s_previousFilter;
    static CrashDumpObserver s_observer;
};

}  // namespace Shell
}  // namespace ExplorerLens
