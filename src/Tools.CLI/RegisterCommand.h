// RegisterCommand.h — lens register / unregister: COM Shell Extension Management
// Copyright (c) 2026 ExplorerLens Project
//
// Sprint 21 (v15.4.0 "Zenith-U"): Implements 'lens register' and 'lens unregister'
// subcommands. Detects whether the DLL is registered, requires admin elevation for
// actual registration, and allows status queries without admin privileges.
//
#pragma once
#include "CommandRouter.h"

namespace ExplorerLens {
namespace CLI {

class RegisterCommand final : public ISubCommand {
public:
    int Execute(const ParsedArgs& args) override;
    std::wstring_view Name()      const noexcept override { return L"register"; }
    std::wstring_view ShortDesc() const noexcept override {
        return L"Register or unregister the ExplorerLens shell extension";
    }
    std::wstring_view Usage() const noexcept override {
        return L"lens register [--status] [--dll <path>] | lens unregister [--dll <path>]";
    }

    // Public helpers used by unit tests and the --status flag.
    static bool IsAdminProcess() noexcept;
    static bool IsRegistered()  noexcept;

private:
    static std::wstring FindDll(const std::wstring& explicitPath);
    int DoRegister  (const std::wstring& dllPath, bool verbose);
    int DoUnregister(const std::wstring& dllPath, bool verbose);
    int DoStatus();
};

class UnregisterCommand final : public ISubCommand {
public:
    int Execute(const ParsedArgs& args) override;
    std::wstring_view Name()      const noexcept override { return L"unregister"; }
    std::wstring_view ShortDesc() const noexcept override {
        return L"Unregister the ExplorerLens shell extension (admin required)";
    }
    std::wstring_view Usage() const noexcept override {
        return L"lens unregister [--dll <path>]";
    }
};

} // namespace CLI
} // namespace ExplorerLens
