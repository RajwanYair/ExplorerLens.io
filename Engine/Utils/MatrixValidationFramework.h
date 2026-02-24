#pragma once
// Matrix Validation Framework
// Cross-validation driver: runs all decoder, cache, and memory sub-system validators
// and renders a pass/fail matrix report for CI gate and release readiness.

#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace ExplorerLens::Utils {

// ─── Validation domain ───────────────────────────────────────────────────────

enum class ValidationDomain : uint32_t {
    Decoder         = 0,
    Memory          = 1,
    Cache           = 2,
    Plugin          = 3,
    ARM64           = 4,
    Pipeline        = 5,
    Integration     = 6,
};

inline std::string ToString(ValidationDomain d) {
    switch (d) {
        case ValidationDomain::Decoder:      return "Decoder";
        case ValidationDomain::Memory:       return "Memory";
        case ValidationDomain::Cache:        return "Cache";
        case ValidationDomain::Plugin:       return "Plugin";
        case ValidationDomain::ARM64:        return "ARM64";
        case ValidationDomain::Pipeline:     return "Pipeline";
        case ValidationDomain::Integration:  return "Integration";
        default: return "Unknown";
    }
}

// ─── Validation case ─────────────────────────────────────────────────────────

enum class ValidationStatus : uint32_t {
    Pass        = 0,
    Fail        = 1,
    Skip        = 2,   // not applicable (e.g., ARM64 skipped on x64)
    Warning     = 3,   // passed but below SLA threshold
};

inline std::string ToString(ValidationStatus s) {
    switch (s) {
        case ValidationStatus::Pass:    return "PASS";
        case ValidationStatus::Fail:    return "FAIL";
        case ValidationStatus::Skip:    return "SKIP";
        case ValidationStatus::Warning: return "WARN";
        default: return "????";
    }
}

struct ValidationCase {
    std::string         name;
    ValidationDomain    domain          { ValidationDomain::Decoder };
    ValidationStatus    status          { ValidationStatus::Skip };
    double              durationMs      { 0.0 };
    std::string         detail;

    bool IsPass() const { return status == ValidationStatus::Pass || status == ValidationStatus::Warning; }
};

// ─── Cell in the matrix ───────────────────────────────────────────────────────

struct MatrixCell {
    std::string         subSystem;      // column header
    ValidationDomain    domain;         // row header
    ValidationStatus    status { ValidationStatus::Skip };
    uint32_t            caseCount { 0 };
    uint32_t            passCount { 0 };
    double              totalMs   { 0.0 };

    double PassRate() const { return caseCount > 0 ? 100.0 * passCount / caseCount : 0.0; }
};

// ─── Matrix report ───────────────────────────────────────────────────────────

struct MatrixValidationReport {
    std::vector<MatrixCell>         cells;
    std::vector<ValidationCase>     failures;
    uint32_t                        totalCases  { 0 };
    uint32_t                        totalPassed { 0 };
    double                          totalMs     { 0.0 };
    bool                            gatePassed  { false };

    double OverallPassRate() const {
        return totalCases > 0 ? 100.0 * totalPassed / totalCases : 0.0;
    }

    static constexpr double kPassGateThreshold = 95.0;  // 95% required for gate

    static MatrixValidationReport CreateMock(bool allPass = true) {
        MatrixValidationReport r;
        std::vector<std::string> systems = { "Decoders", "Memory", "Cache", "Plugins", "Pipeline" };
        const ValidationDomain domains[] = {
            ValidationDomain::Decoder, ValidationDomain::Memory,
            ValidationDomain::Cache,   ValidationDomain::Plugin,
            ValidationDomain::Pipeline
        };

        for (int i = 0; i < 5; ++i) {
            MatrixCell cell;
            cell.subSystem  = systems[i];
            cell.domain     = domains[i];
            cell.caseCount  = 20;
            cell.passCount  = allPass ? 20 : (i == 2 ? 18 : 20);
            cell.status     = (cell.passCount == cell.caseCount) ? ValidationStatus::Pass : ValidationStatus::Warning;
            cell.totalMs    = 42.0;
            r.cells.push_back(cell);
            r.totalCases   += cell.caseCount;
            r.totalPassed  += cell.passCount;
        }
        r.totalMs    = 210.0;
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
            cell.domain    = entry.domain;
            cell.caseCount = static_cast<uint32_t>(cases.size());
            for (const auto& c : cases) {
                cell.totalMs += c.durationMs;
                if (c.IsPass()) ++cell.passCount;
                else report.failures.push_back(c);
            }
            cell.status = (cell.passCount == cell.caseCount) ? ValidationStatus::Pass
                        : (cell.passCount > 0)               ? ValidationStatus::Warning
                        :                                      ValidationStatus::Fail;
            report.cells.push_back(cell);
            report.totalCases  += cell.caseCount;
            report.totalPassed += cell.passCount;
            report.totalMs     += cell.totalMs;
        }
        report.gatePassed = report.OverallPassRate() >= MatrixValidationReport::kPassGateThreshold;
        return report;
    }

private:
    struct ValidatorEntry {
        ValidationDomain domain;
        std::string      subSystem;
        ValidatorFn      fn;
    };
    std::vector<ValidatorEntry> m_validators;
};

} // namespace ExplorerLens::Utils

