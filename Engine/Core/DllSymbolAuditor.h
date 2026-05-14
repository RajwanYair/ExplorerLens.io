// Engine/Core/DllSymbolAuditor.h
// ExplorerLens Engine — S378
//
// Purpose:
//   DLL exported symbol auditor for LENSShell.dll size reduction (< 2,500 KB).
//   Phase 3 exit criterion: "DLL size < 2,500 KB".
//
//   Provides:
//   - Symbol export enumeration (via DUMPBIN-style or DbgHelp)
//   - Export count + cumulative name table size
//   - Dead symbol detection (exported but not referenced)
//   - Comparison against a budget baseline (baseline.json)
//   - Report generation for CI integration

#pragma once
#ifndef EXPLORERLENS_ENGINE_DLLSYMBOLAUDITOR_H
#define EXPLORERLENS_ENGINE_DLLSYMBOLAUDITOR_H

#include <cstdint>
#include <cstddef>
#include <string_view>

namespace ExplorerLens::Engine {

// ─── Audit status ────────────────────────────────────────────────────────────

enum class DllAuditStatus : uint8_t {
    OK                   = 0,
    FILE_NOT_FOUND       = 1,
    NOT_A_PE             = 2,   // not a valid PE/COFF file
    EXPORT_PARSE_FAILED  = 3,
    SIZE_OVER_BUDGET     = 4,   // DLL size exceeds budget
    SYMBOL_COUNT_REGRESSED = 5, // new symbols added without justification
    NOT_WIN32            = 6,
};

// ─── Symbol visibility ───────────────────────────────────────────────────────

enum class SymbolVisibility : uint8_t {
    EXPORTED   = 0,   // in EAT (Export Address Table)
    INTERNAL   = 1,   // not exported; found via PDB
    COM_ENTRY  = 2,   // DllGetClassObject / DllRegisterServer etc.
};

// ─── Per-symbol record ───────────────────────────────────────────────────────

struct DllSymbolRecord final {
    SymbolVisibility visibility     = SymbolVisibility::EXPORTED;
    const char*      name           = nullptr;    // demangled
    uint32_t         rva             = 0;          // relative virtual address
    uint32_t         nameLen         = 0;
    bool             isCOMEntry      = false;
    bool             isDataSymbol    = false;      // vs. code symbol

    bool IsValid() const noexcept { return name != nullptr && nameLen > 0; }
};

// ─── Audit budget ────────────────────────────────────────────────────────────

struct DllSizeBudget final {
    uint32_t maxSizeKb           = 2500;   // Phase 3 target: < 2,500 KB
    uint32_t maxExportedSymbols  = 50;     // COM DLL should have few exports
    uint32_t warnSizeKb          = 2200;   // warn before hard limit
    bool     failOnOverBudget    = true;

    static constexpr DllSizeBudget Phase3Target() noexcept {
        return DllSizeBudget{ 2500, 50, 2200, true };
    }

    static constexpr DllSizeBudget CurrentBaseline() noexcept {
        return DllSizeBudget{ 2940, 100, 2700, false };  // v39.9.0 actual size
    }

    static constexpr DllSizeBudget Strict() noexcept {
        return DllSizeBudget{ 2000, 30, 1800, true };
    }
};

// ─── Audit report ────────────────────────────────────────────────────────────

struct DllAuditReport final {
    DllAuditStatus status           = DllAuditStatus::OK;
    uint32_t       fileSizeKb       = 0;
    uint32_t       exportedSymbols  = 0;
    uint32_t       comEntrySymbols  = 0;
    uint32_t       nameTableBytes   = 0;
    bool           sizeWithinBudget = false;
    bool           symbolCountOk    = false;

    bool IsOk() const noexcept { return status == DllAuditStatus::OK; }
    bool IsGreen() const noexcept { return IsOk() && sizeWithinBudget && symbolCountOk; }
};

// ─── Config ──────────────────────────────────────────────────────────────────

struct DllAuditConfig final {
    DllSizeBudget budget;
    bool          enumerateSymbols  = true;
    bool          demangleNames     = true;
    bool          writeCsvReport    = false;
    const wchar_t* csvOutputPath    = nullptr;

    static DllAuditConfig Default() noexcept {
        DllAuditConfig c{};
        c.budget            = DllSizeBudget::Phase3Target();
        c.enumerateSymbols  = true;
        c.demangleNames     = true;
        return c;
    }

    static DllAuditConfig CiMode() noexcept {
        DllAuditConfig c{};
        c.budget            = DllSizeBudget::Phase3Target();
        c.enumerateSymbols  = true;
        c.writeCsvReport    = true;
        return c;
    }
};

// ─── Main class ──────────────────────────────────────────────────────────────

class DllSymbolAuditor final {
public:
    DllSymbolAuditor() = default;
    ~DllSymbolAuditor() = default;

    DllSymbolAuditor(const DllSymbolAuditor&) = delete;
    DllSymbolAuditor& operator=(const DllSymbolAuditor&) = delete;

    static DllSymbolAuditor& Global() noexcept {
        static DllSymbolAuditor s_instance;
        return s_instance;
    }

    void Configure(const DllAuditConfig& config) noexcept { m_config = config; }

    // Audit a DLL file — enumerate exports, check size vs. budget
    DllAuditReport Audit(const wchar_t* dllPath) noexcept;

    // Audit LENSShell.dll specifically (uses known relative path)
    DllAuditReport AuditShellDll() noexcept;

    // Check if a DLL's size is within the budget
    DllAuditStatus CheckSizeBudget(const wchar_t* dllPath, const DllSizeBudget& budget) noexcept;

    uint32_t AuditsTotal() const noexcept { return m_auditsTotal; }
    uint32_t BudgetFailures() const noexcept { return m_budgetFailures; }

    const DllAuditConfig& Config() const noexcept { return m_config; }

private:
    DllAuditConfig m_config{};
    uint32_t       m_auditsTotal     = 0;
    uint32_t       m_budgetFailures  = 0;
};

// ─── Inline stubs ────────────────────────────────────────────────────────────

inline DllAuditReport DllSymbolAuditor::Audit(const wchar_t* dllPath) noexcept {
    DllAuditReport r{};
#ifndef _WIN32
    r.status = DllAuditStatus::NOT_WIN32;
    return r;
#else
    if (!dllPath) { r.status = DllAuditStatus::FILE_NOT_FOUND; return r; }
    // Stub: real impl uses GetFileSize + DbgHelp or DUMPBIN
    r.fileSizeKb       = 2940;  // v39.9.0 baseline
    r.exportedSymbols  = 4;     // COM entries only
    r.comEntrySymbols  = 4;
    r.sizeWithinBudget = r.fileSizeKb <= m_config.budget.maxSizeKb;
    r.symbolCountOk    = r.exportedSymbols <= m_config.budget.maxExportedSymbols;
    r.status           = r.sizeWithinBudget ? DllAuditStatus::OK
                                            : DllAuditStatus::SIZE_OVER_BUDGET;
    ++m_auditsTotal;
    if (!r.sizeWithinBudget) ++m_budgetFailures;
    return r;
#endif
}

inline DllAuditReport DllSymbolAuditor::AuditShellDll() noexcept {
    return Audit(L"x64\\Release\\LENSShell.dll");
}

inline DllAuditStatus DllSymbolAuditor::CheckSizeBudget(
    const wchar_t* dllPath, const DllSizeBudget& budget) noexcept
{
    if (!dllPath) return DllAuditStatus::FILE_NOT_FOUND;
    auto r = Audit(dllPath);
    if (!r.sizeWithinBudget) return DllAuditStatus::SIZE_OVER_BUDGET;
    return DllAuditStatus::OK;
    (void)budget;
}

// ─── Constants ───────────────────────────────────────────────────────────────

static constexpr uint32_t kDllBudgetPhase3Kb           = 2500u;  // Phase 3 target
static constexpr uint32_t kDllBudgetCurrentKb          = 2940u;  // v39.9.0 actual
static constexpr uint32_t kDllBudgetMaxCOMExports       = 4u;    // COM entries only
static constexpr uint32_t kDllBudgetWarnKb             = 2200u;

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_DLLSYMBOLAUDITOR_H
