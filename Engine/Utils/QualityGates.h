#pragma once
// QualityGates.h — Consolidated Quality Assurance Gates
// Copyright (c) 2026 ExplorerLens Project
//
// Unified header for quality gate and validation concerns:
// - Final QA matrix: aggregates test results into ship/hold/block signals
// - Static analysis quality gate: clang-tidy/MSVC /analyze/CppCheck enforcement
// - Cross-validation driver: runs all decoder/cache/memory validators

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

// ─── MatrixValidationFramework ──────────────────────────────────────────────

namespace ExplorerLens::Utils {

// ─── Validation domain ───────────────────────────────────────────────────────

enum class ValidationDomain : uint32_t {
    Decoder = 0,
    Memory = 1,
    Cache = 2,
    Plugin = 3,
    ARM64 = 4,
    Pipeline = 5,
    Integration = 6,
};

inline std::string ToString(ValidationDomain d) {
    switch (d) {
    case ValidationDomain::Decoder: return "Decoder";
    case ValidationDomain::Memory: return "Memory";
    case ValidationDomain::Cache: return "Cache";
    case ValidationDomain::Plugin: return "Plugin";
    case ValidationDomain::ARM64: return "ARM64";
    case ValidationDomain::Pipeline: return "Pipeline";
    case ValidationDomain::Integration: return "Integration";
    default: return "Unknown";
    }
}

// ─── Validation case ─────────────────────────────────────────────────────────

enum class ValidationStatus : uint32_t {
    Pass = 0,
    Fail = 1,
    Skip = 2, // not applicable (e.g., ARM64 skipped on x64)
    Warning = 3, // passed but below SLA threshold
};

inline std::string ToString(ValidationStatus s) {
    switch (s) {
    case ValidationStatus::Pass: return "PASS";
    case ValidationStatus::Fail: return "FAIL";
    case ValidationStatus::Skip: return "SKIP";
    case ValidationStatus::Warning: return "WARN";
    default: return "????";
    }
}

struct ValidationCase {
    std::string name;
    ValidationDomain domain{ ValidationDomain::Decoder };
    ValidationStatus status{ ValidationStatus::Skip };
    double durationMs{ 0.0 };
    std::string detail;

    bool IsPass() const { return status == ValidationStatus::Pass || status == ValidationStatus::Warning; }
};

// ─── Cell in the matrix ───────────────────────────────────────────────────────

struct MatrixCell {
    std::string subSystem; // column header
    ValidationDomain domain; // row header
    ValidationStatus status{ ValidationStatus::Skip };
    uint32_t caseCount{ 0 };
    uint32_t passCount{ 0 };
    double totalMs{ 0.0 };

    double PassRate() const { return caseCount > 0 ? 100.0 * passCount / caseCount : 0.0; }
};

// ─── Matrix report ───────────────────────────────────────────────────────────

struct MatrixValidationReport {
    std::vector<MatrixCell> cells;
    std::vector<ValidationCase> failures;
    uint32_t totalCases{ 0 };
    uint32_t totalPassed{ 0 };
    double totalMs{ 0.0 };
    bool gatePassed{ false };

    double OverallPassRate() const {
        return totalCases > 0 ? 100.0 * totalPassed / totalCases : 0.0;
    }

    static constexpr double kPassGateThreshold = 95.0; // 95% required for gate

    static MatrixValidationReport CreateMock(bool allPass = true) {
        MatrixValidationReport r;
        std::vector<std::string> systems = { "Decoders", "Memory", "Cache", "Plugins", "Pipeline" };
        const ValidationDomain domains[] = {
        ValidationDomain::Decoder, ValidationDomain::Memory,
        ValidationDomain::Cache, ValidationDomain::Plugin,
        ValidationDomain::Pipeline
        };

        for (int i = 0; i < 5; ++i) {
            MatrixCell cell;
            cell.subSystem = systems[i];
            cell.domain = domains[i];
            cell.caseCount = 20;
            cell.passCount = allPass ? 20 : (i == 2 ? 18 : 20);
            cell.status = (cell.passCount == cell.caseCount) ? ValidationStatus::Pass : ValidationStatus::Warning;
            cell.totalMs = 42.0;
            r.cells.push_back(cell);
            r.totalCases += cell.caseCount;
            r.totalPassed += cell.passCount;
        }
        r.totalMs = 210.0;
        r.gatePassed = r.OverallPassRate() >= kPassGateThreshold;
        return r;
    }
};

// ─── Framework runner ────────────────────────────────────────────────────────

using ValidatorFn = std::function<std::vector<ValidationCase>()>;

class MatrixValidationFramework {
public:
    void RegisterValidator(ValidationDomain domain, const std::string& subSystem, ValidatorFn fn) {
        m_validators.push_back({ domain, subSystem, std::move(fn) });
    }

    MatrixValidationReport RunAll() {
        MatrixValidationReport report;
        for (auto& entry : m_validators) {
            auto cases = entry.fn();
            MatrixCell cell;
            cell.subSystem = entry.subSystem;
            cell.domain = entry.domain;
            cell.caseCount = static_cast<uint32_t>(cases.size());
            for (const auto& c : cases) {
                cell.totalMs += c.durationMs;
                if (c.IsPass()) ++cell.passCount;
                else report.failures.push_back(c);
            }
            cell.status = (cell.passCount == cell.caseCount) ? ValidationStatus::Pass
                : (cell.passCount > 0) ? ValidationStatus::Warning
                : ValidationStatus::Fail;
            report.cells.push_back(cell);
            report.totalCases += cell.caseCount;
            report.totalPassed += cell.passCount;
            report.totalMs += cell.totalMs;
        }
        report.gatePassed = report.OverallPassRate() >= MatrixValidationReport::kPassGateThreshold;
        return report;
    }

private:
    struct ValidatorEntry {
        ValidationDomain domain;
        std::string subSystem;
        ValidatorFn fn;
    };
    std::vector<ValidatorEntry> m_validators;
};

} // namespace ExplorerLens::Utils

// ─── QualityAssuranceV2 ─────────────────────────────────────────────────────

namespace ExplorerLens {
namespace Engine {

enum class QATestCategory : uint8_t { Unit = 0, Integration, Performance, Fuzz, Regression, Accessibility, COUNT };
enum class QADefectSeverity : uint8_t { Critical = 0, High, Medium, Low, Info, COUNT };
enum class QAShipSignal : uint8_t { Ship = 0, ConditionalShip, Hold, Block, COUNT };

struct QASuiteResult {
    QATestCategory category = QATestCategory::Unit;
    uint32_t testsTotal = 0;
    uint32_t testsPassed = 0;
    uint32_t testsFailed = 0;
    float passPct = 0.0f;
    bool suitePass = false;
};

struct QAFinalMatrix {
    uint32_t totalTests = 0;
    uint32_t totalPassed = 0;
    float overallPassPct = 0.0f;
    uint32_t criticalDefects = 0;
    float defectDensity = 0.0f; // defects per 1K lines
    float escapeRate = 0.0f; // % defects found post-release
    QAShipSignal shipSignal = QAShipSignal::Hold;
};

class QualityAssuranceV2 {
public:
    static const wchar_t* TestCategoryName(QATestCategory c) {
        switch (c) {
        case QATestCategory::Unit: return L"Unit Tests";
        case QATestCategory::Integration: return L"Integration Tests";
        case QATestCategory::Performance: return L"Performance Tests";
        case QATestCategory::Fuzz: return L"Fuzz Tests";
        case QATestCategory::Regression: return L"Regression Tests";
        case QATestCategory::Accessibility: return L"Accessibility Tests";
        default: return L"Unknown";
        }
    }
    static const wchar_t* DefectSeverityName(QADefectSeverity s) {
        switch (s) {
        case QADefectSeverity::Critical: return L"Critical";
        case QADefectSeverity::High: return L"High";
        case QADefectSeverity::Medium: return L"Medium";
        case QADefectSeverity::Low: return L"Low";
        case QADefectSeverity::Info: return L"Info";
        default: return L"Unknown";
        }
    }
    static const wchar_t* ShipSignalName(QAShipSignal s) {
        switch (s) {
        case QAShipSignal::Ship: return L"SHIP";
        case QAShipSignal::ConditionalShip: return L"CONDITIONAL SHIP";
        case QAShipSignal::Hold: return L"HOLD";
        case QAShipSignal::Block: return L"BLOCK";
        default: return L"Unknown";
        }
    }
    static constexpr size_t TestCategoryCount() { return static_cast<size_t>(QATestCategory::COUNT); }
    static constexpr size_t DefectSeverityCount() { return static_cast<size_t>(QADefectSeverity::COUNT); }
    static constexpr size_t ShipSignalCount() { return static_cast<size_t>(QAShipSignal::COUNT); }
    static QAShipSignal Evaluate(const QAFinalMatrix& m) {
        if (m.criticalDefects > 0) return QAShipSignal::Block;
        if (m.overallPassPct < 99.0f) return QAShipSignal::Hold;
        if (m.defectDensity > 0.5f) return QAShipSignal::ConditionalShip;
        return QAShipSignal::Ship;
    }
};

}
} // namespace ExplorerLens::Engine

// ─── StaticAnalysisGate ─────────────────────────────────────────────────────

namespace ExplorerLens {
namespace Engine {

/// Static analysis tool
enum class AnalysisTool : uint8_t {
    None = 0,
    ClangTidy = 1,
    MSVCAnalyze = 2,
    CppCheck = 3,
    PVSStudio = 4,
    CodeQL = 5,
    COUNT = 6
};

/// Warning severity
enum class WarningSeverity : uint8_t {
    Note = 0,
    Warning = 1,
    Error = 2,
    Fatal = 3
};

/// Warning category
enum class WarningCategory : uint8_t {
    Security = 0, ///< Buffer overflows, use-after-free
    Performance = 1, ///< Unnecessary copies, cache unfriendly
    Correctness = 2, ///< Logic errors, undefined behavior
    Readability = 3, ///< Naming, complexity, magic numbers
    Modernization = 4, ///< C++17/20 upgrades available
    Portability = 5, ///< Platform-specific assumptions
    Concurrency = 6, ///< Race conditions, deadlocks
    CategoryCount = 7
};

/// Warning suppression entry
struct WarningSuppression {
    const char* warningId =
        nullptr; ///< e.g., "C26495" or "cppcoreguidelines-init-variables"
    const char* file = nullptr; ///< File pattern (nullptr = global)
    const char* reason = nullptr; ///< Why suppressed
    WarningSeverity severity = WarningSeverity::Warning;
    WarningCategory category = WarningCategory::Correctness;
    bool isTemporary = false; ///< Will be fixed later
    const char* fixTarget = nullptr; ///< Target version to fix in
};

/// Quality gate thresholds per category
struct AnalysisThresholds {
    uint32_t maxSecurityWarnings = 0; ///< Zero tolerance
    uint32_t maxCorrectnessWarnings = 0; ///< Zero tolerance
    uint32_t maxPerformanceWarnings = 10; ///< Soft limit
    uint32_t maxReadabilityWarnings = 50; ///< Info only
    uint32_t maxModernizationWarnings = 100;
    uint32_t maxConcurrencyWarnings = 0; ///< Zero tolerance
    bool failOnSecurityWarning = true;
    bool failOnCorrectnessWarning = true;
    bool failOnConcurrencyWarning = true;
};

/// Static analysis quality gate
class StaticAnalysisGate {
public:
    static StaticAnalysisGate& Instance() {
        static StaticAnalysisGate inst;
        return inst;
    }

    /// Get clang-tidy checks string
    static const char* GetClangTidyChecks() {
        return "bugprone-*,"
            "cert-*,"
            "clang-analyzer-*,"
            "concurrency-*,"
            "cppcoreguidelines-*,"
            "misc-*,"
            "modernize-*,"
            "performance-*,"
            "readability-*,"
            "-modernize-use-trailing-return-type," // Style preference
            "-readability-magic-numbers," // Too noisy for constants
            "-cppcoreguidelines-avoid-magic-numbers," // Same
            "-readability-identifier-length," // Short names OK in loops
            "-cppcoreguidelines-pro-type-reinterpret-cast," // Needed for Windows
            // API
            "-cppcoreguidelines-pro-type-union-access"; // COM interop
    }

    /// Get MSVC /analyze warning level
    static const char* GetMSVCAnalyzeFlags() {
        return "/analyze "
            "/analyze:WX- " // Warnings don't fail (we handle separately)
            "/analyze:log analyze.xml "
            "/analyze:ruleset NativeRecommendedRules.ruleset";
    }

    /// Get CppCheck command line
    static const char* GetCppCheckCommand() {
        return "cppcheck "
            "--enable=all "
            "--suppress=missingInclude "
            "--suppress=unusedFunction "
            "--std=c++20 "
            "--platform=win64 "
            "--inline-suppr "
            "--xml "
            "--output-file=cppcheck-report.xml "
            "Engine/ LENSShell/";
    }

    /// Check if a gate passes
    bool CheckGate(uint32_t securityWarnings, uint32_t correctnessWarnings,
        uint32_t concurrencyWarnings) const {
        if (m_thresholds.failOnSecurityWarning &&
            securityWarnings > m_thresholds.maxSecurityWarnings)
            return false;
        if (m_thresholds.failOnCorrectnessWarning &&
            correctnessWarnings > m_thresholds.maxCorrectnessWarnings)
            return false;
        if (m_thresholds.failOnConcurrencyWarning &&
            concurrencyWarnings > m_thresholds.maxConcurrencyWarnings)
            return false;
        return true;
    }

    /// Known suppressions
    static constexpr uint32_t SUPPRESSION_COUNT = 5;
    static const WarningSuppression* GetSuppressions() {
        static const WarningSuppression suppressions[] = {
        {"C4996", nullptr,
        "Deprecated CRT functions used intentionally (fopen_s used instead "
        "where possible)",
        WarningSeverity::Warning, WarningCategory::Security, false, nullptr},
        {"C26812", nullptr,
        "Prefer enum class — already using enum class everywhere in new code",
        WarningSeverity::Warning, WarningCategory::Modernization, false,
        nullptr},
        {"C6387", "LENSShell/*",
        "SAL annotations sometimes over-report on COM interfaces",
        WarningSeverity::Warning, WarningCategory::Correctness, true,
        ""},
        {"cppcoreguidelines-owning-memory", nullptr,
        "COM raw pointers managed by AddRef/Release", WarningSeverity::Note,
        WarningCategory::Readability, false, nullptr},
        {"performance-no-int-to-ptr", nullptr,
        "MAKEINTRESOURCE and Windows API macros require int-to-ptr",
        WarningSeverity::Note, WarningCategory::Performance, false, nullptr},
        };
        return suppressions;
    }

    /// Category name
    static const char* CategoryName(WarningCategory c) {
        switch (c) {
        case WarningCategory::Security:
            return "Security";
        case WarningCategory::Performance:
            return "Performance";
        case WarningCategory::Correctness:
            return "Correctness";
        case WarningCategory::Readability:
            return "Readability";
        case WarningCategory::Modernization:
            return "Modernization";
        case WarningCategory::Portability:
            return "Portability";
        case WarningCategory::Concurrency:
            return "Concurrency";
        default:
            return "Unknown";
        }
    }

    const AnalysisThresholds& GetThresholds() const { return m_thresholds; }
    void SetThresholds(const AnalysisThresholds& t) { m_thresholds = t; }

    /// Tool queries
    static constexpr size_t ToolCount() {
        return static_cast<size_t>(AnalysisTool::COUNT);
    }
    static const wchar_t* ToolName(AnalysisTool t) {
        switch (t) {
        case AnalysisTool::None:
            return L"None";
        case AnalysisTool::ClangTidy:
            return L"clang-tidy";
        case AnalysisTool::MSVCAnalyze:
            return L"MSVC /analyze";
        case AnalysisTool::CppCheck:
            return L"cppcheck";
        case AnalysisTool::PVSStudio:
            return L"PVS-Studio";
        case AnalysisTool::CodeQL:
            return L"CodeQL";
        default:
            return L"Unknown";
        }
    }

private:
    StaticAnalysisGate() = default;
    AnalysisThresholds m_thresholds;
};

} // namespace Engine
} // namespace ExplorerLens
