// ComplianceReportGenerator.h — GDPR/HIPAA/SOC-2/ISO 27001 Compliance Reports
// Copyright (c) 2026 ExplorerLens Project
//
// Generates structured compliance reports for GDPR, HIPAA, SOC-2, ISO 27001,
// and NIST CSF frameworks with evidence collection and multi-format export.
//
#pragma once

#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <cstdint>
#include <optional>

namespace ExplorerLens::Engine {

enum class ComplianceFramework : uint8_t {
    GDPR     = 0,
    HIPAA    = 1,
    SOC2     = 2,
    ISO27001 = 3,
    NIST_CSF = 4
};

enum class ReportFormat : uint8_t {
    PDF     = 0,
    JSON_LD = 1,
    HTML    = 2,
    XLSX    = 3
};

enum class ControlStatus : uint8_t {
    Compliant     = 0,
    NonCompliant  = 1,
    Partial       = 2,
    NotApplicable = 3,
    InReview      = 4
};

struct ComplianceEvidence {
    ComplianceFramework framework{ComplianceFramework::SOC2};
    std::string         controlId;
    ControlStatus       status{ControlStatus::InReview};
    std::string         evidence;
    std::string         collectedBy;
    std::chrono::system_clock::time_point timestamp;
};

struct ComplianceSummary {
    ComplianceFramework framework{ComplianceFramework::SOC2};
    uint32_t            totalControls{0};
    uint32_t            compliantControls{0};
    uint32_t            nonCompliantControls{0};
    uint32_t            partialControls{0};
    double              compliancePercentage{0.0};
};

class ComplianceReportGenerator {
public:
    ComplianceReportGenerator()  = default;
    ~ComplianceReportGenerator() = default;

    ComplianceReportGenerator(const ComplianceReportGenerator&)            = delete;
    ComplianceReportGenerator& operator=(const ComplianceReportGenerator&) = delete;

    // Evidence management
    bool AddEvidence(ComplianceEvidence evidence);
    bool RemoveEvidence(const std::string& controlId, ComplianceFramework framework);
    std::vector<ComplianceEvidence> GetEvidence(ComplianceFramework framework) const;

    // Control status
    ControlStatus GetControlStatus(const std::string& controlId,
                                   ComplianceFramework framework) const;
    bool          SetControlStatus(const std::string& controlId,
                                   ComplianceFramework framework,
                                   ControlStatus       status);

    // Report generation
    bool GenerateReport(ComplianceFramework framework, ReportFormat format,
                        const std::string& outputPath);
    bool ExportToFormat(const std::string& reportId, ReportFormat format,
                        const std::string& outputPath);

    // Summary access
    ComplianceSummary              GetSummary(ComplianceFramework framework) const;
    std::vector<ComplianceSummary> GetAllSummaries() const;

    // Progress notification
    using ReportCallback = std::function<void(const std::string& reportId, bool success)>;
    void SetReportCallback(ReportCallback cb);

private:
    std::vector<ComplianceEvidence> m_evidence;
    ReportCallback                  m_reportCallback;

    ComplianceSummary ComputeSummary(ComplianceFramework framework) const;
    std::string       GenerateReportId() const;
    bool              ValidateFrameworkControls(ComplianceFramework framework) const;
    std::string       SerializeEvidence(const ComplianceEvidence& ev) const;
};

} // namespace ExplorerLens::Engine
