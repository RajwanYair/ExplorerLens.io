// PrePublishReviewGateway.h — Pre-Publish Review Gateway
// Copyright (c) 2026 ExplorerLens Project
//
// Automated and manual review pipeline for plugin submissions before marketplace publication.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>
#include <optional>

namespace ExplorerLens::Engine {

enum class ReviewStatus : uint8_t {
    Pending      = 0,
    Approved     = 1,
    Rejected     = 2,
    NeedsRevision = 3,
    Suspended    = 4,
};

enum class ReviewCheck : uint8_t {
    StaticAnalysis  = 0,
    SignatureVerify = 1,
    WAsmSandbox     = 2,
    CVEScan         = 3,
    LicenseCheck    = 4,
};

struct CheckResult {
    ReviewCheck check    = ReviewCheck::StaticAnalysis;
    bool        passed   = false;
    std::string detail;
};

struct ReviewReport {
    ReviewStatus             status     = ReviewStatus::Pending;
    std::vector<CheckResult> checks;
    std::vector<std::string> issues;
    std::string              reviewerId;
    std::chrono::system_clock::time_point timestamp{};

    [[nodiscard]] bool AllChecksPassed() const noexcept {
        for (const auto& c : checks) {
            if (!c.passed) return false;
        }
        return true;
    }

    [[nodiscard]] bool HasCheck(ReviewCheck c) const noexcept {
        for (const auto& r : checks) {
            if (r.check == c) return true;
        }
        return false;
    }
};

struct SubmissionPackage {
    std::string pluginId;
    std::string version;
    std::string archivePath;  // local path to the plugin archive
    std::string submitterId;
    std::string manifestJson;
};

using ReviewCallback = std::function<void(const std::string& pluginId, ReviewStatus status)>;

class PrePublishReviewGateway {
public:
    PrePublishReviewGateway()  = default;
    ~PrePublishReviewGateway() = default;

    PrePublishReviewGateway(const PrePublishReviewGateway&)            = delete;
    PrePublishReviewGateway& operator=(const PrePublishReviewGateway&) = delete;
    PrePublishReviewGateway(PrePublishReviewGateway&&)                 = default;
    PrePublishReviewGateway& operator=(PrePublishReviewGateway&&)      = default;

    // Queue a plugin submission; returns a review ticket ID.
    [[nodiscard]] std::string SubmitForReview(const SubmissionPackage& pkg,
                                              ReviewCallback           onComplete = nullptr);

    // Retrieve the latest review report for a plugin version.
    [[nodiscard]] std::optional<ReviewReport> GetStatus(const std::string& pluginId,
                                                        const std::string& version) const;

    // Execute all automated checks synchronously; returns the populated report.
    [[nodiscard]] ReviewReport RunChecks(const SubmissionPackage& pkg) const;

    // Approve a pending submission (requires reviewer credentials).
    bool ApprovePlugin(const std::string& pluginId,
                       const std::string& version,
                       const std::string& reviewerId);

    // Reject a pending submission with a reason string.
    bool RejectPlugin(const std::string& pluginId,
                      const std::string& version,
                      const std::string& reviewerId,
                      const std::string& reason);

    // Suspend a previously-approved plugin (e.g. after a CVE report).
    bool SuspendPlugin(const std::string& pluginId, const std::string& reason);

    // List all submissions in a given status.
    [[nodiscard]] std::vector<std::string> GetSubmissionsByStatus(ReviewStatus status) const;

    void SetReviewerEndpoint(const std::string& url);
    void EnableCheck(ReviewCheck check, bool enabled) noexcept;

private:
    std::string m_reviewerEndpoint = "https://review.explorerlens.io/v4";
    uint32_t    m_enabledCheckMask = 0xFF;

    struct Impl {};
    std::unique_ptr<Impl> m_impl;

    [[nodiscard]] CheckResult RunStaticAnalysis(const SubmissionPackage& pkg) const;
    [[nodiscard]] CheckResult RunSignatureVerify(const SubmissionPackage& pkg) const;
    [[nodiscard]] CheckResult RunWAsmSandbox(const SubmissionPackage& pkg) const;
    [[nodiscard]] CheckResult RunCVEScan(const SubmissionPackage& pkg) const;
    [[nodiscard]] CheckResult RunLicenseCheck(const SubmissionPackage& pkg) const;
};

} // namespace ExplorerLens::Engine
