// StoreReadinessChecker.h — Windows Store / MSIX Certification Validator
// Copyright (c) 2026 ExplorerLens Project
//
// Validates the ExplorerLens MSIX package against Windows Store submission
// requirements and the Windows App Certification Kit (WACK) rules.
// Runs as part of the CI pipeline before any Store submission attempt.
//
#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

// Category of a Store readiness check.
enum class StoreCheckCategory : uint8_t {
    Manifest      = 0x01,  // MSIX manifest validation
    Binary        = 0x02,  // Binary signing + SafeSEH/DEP/ASLR
    API           = 0x04,  // Prohibited Win32 API usage
    Capability    = 0x08,  // Declared capabilities match actual usage
    Performance   = 0x10,  // Startup time + responsiveness budget
    Privacy       = 0x20,  // Privacy policy + data handling
    All           = 0xFF,
};

// Severity of a Store readiness finding.
enum class StoreCheckSeverity : uint8_t {
    Info     = 0,  // Informational — will not block submission
    Warning  = 1,  // Recommended fix
    Error    = 2,  // Must fix before submission
    Blocking = 3,  // Will result in immediate cert failure
};

// A single readiness finding.
struct StoreReadinessFinding {
    StoreCheckSeverity severity;
    StoreCheckCategory category;
    std::string        ruleId;       // WACK rule ID or Store policy ref
    std::string        component;    // Which file or section failed
    std::string        description;
    std::string        remediation;  // How to fix
};

// Summary of a readiness check run.
struct StoreReadinessResult {
    std::vector<StoreReadinessFinding> findings;
    uint32_t  blockingCount { 0 };
    uint32_t  errorCount    { 0 };
    uint32_t  warningCount  { 0 };
    bool      storeReady    { false };

    std::string Summary() const;
};

// StoreReadinessChecker — Pre-submission MSIX/WACK validator.
class StoreReadinessChecker {
public:
    StoreReadinessChecker() noexcept  = default;
    ~StoreReadinessChecker() noexcept = default;

    StoreReadinessChecker(const StoreReadinessChecker&)            = delete;
    StoreReadinessChecker& operator=(const StoreReadinessChecker&) = delete;

    // Run all checks against a built MSIX package path.
    StoreReadinessResult Check(const std::string& msixPath,
                                StoreCheckCategory cats = StoreCheckCategory::All) noexcept;

    // Validate MSIX manifest (capabilities, extensions, CLSID registration).
    StoreReadinessResult CheckManifest(const std::string& manifestPath) noexcept;

    // Check binary security features (signing, DEP, ASLR, SafeSEH, CFG).
    StoreReadinessResult CheckBinaries(const std::string& binDir) noexcept;

    // Write HTML report to path.
    bool WriteHtmlReport(const StoreReadinessResult& result,
                          const std::string& outputPath) const noexcept;

    // Write JUnit XML for CI integration.
    bool WriteJunitXml(const StoreReadinessResult& result,
                        const std::string& outputPath) const noexcept;
};

}} // namespace ExplorerLens::Engine
