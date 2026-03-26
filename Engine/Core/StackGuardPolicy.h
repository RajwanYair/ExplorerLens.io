// StackGuardPolicy.h — CFG + Shadow Stack Enforcement Checker
// Copyright (c) 2026 ExplorerLens Project
//
// Verifies at startup that Control Flow Guard (CFG), Hardware-enforced
// Stack Protection (HSP / CET Shadow Stack), and Data Execution Prevention
// (DEP/NX) are active for the host process and all loaded plugin DLLs.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct ModuleSecurityFlags {
    std::wstring dllPath;
    bool         cfgEnabled{false};       // /guard:cf
    bool         shadowStackEnabled{false}; // CET / CETCOMPAT PE flag
    bool         depEnabled{false};        // NX bit / /NXCOMPAT
    bool         aslrEnabled{false};       // DYNAMICBASE
    bool         safeExceptionsEnabled{false}; // SafeSEH / SEHOP
    bool         rfgEnabled{false};        // /guard:rf (return-flow guard)
};

struct ProcessSecurityReport {
    bool                              cfgEnabled{false};
    bool                              shadowStackEnabled{false};
    bool                              depEnabled{false};
    bool                              aslrEnabled{false};
    std::vector<ModuleSecurityFlags>  modules;
    std::vector<std::wstring>         violatingModules;  // modules missing required flags
    bool                              allPoliciesMet{false};
};

class StackGuardPolicy {
public:
    struct Requirements {
        bool requireCFG{true};
        bool requireShadowStack{false};  // requires Win11 22H2+ hardware CET
        bool requireDEP{true};
        bool requireASLR{true};
        bool blockOnViolation{false};    // if true, refuse to load violating plugins
    };

    explicit StackGuardPolicy(Requirements req = {}) : m_req(req) {}

    // Audit the current process and all loaded modules.
    [[nodiscard]] ProcessSecurityReport AuditProcess() const;

    // Audit a specific plugin DLL before loading it.
    [[nodiscard]] ModuleSecurityFlags AuditModule(const std::wstring& dllPath) const noexcept;

    // Check if a module meets all configured requirements.
    [[nodiscard]] bool ModuleMeetsRequirements(const ModuleSecurityFlags& flags) const noexcept;

    static bool IsCFGEnabled() noexcept;
    static bool IsShadowStackEnabled() noexcept;
    static bool IsDEPEnabled() noexcept;

private:
    static uint32_t GetPEDllCharacteristics(const std::wstring& dllPath) noexcept;
    static bool     HasCETCompatFlag(const std::wstring& dllPath) noexcept;

    Requirements m_req;
};

} // namespace Engine
} // namespace ExplorerLens
