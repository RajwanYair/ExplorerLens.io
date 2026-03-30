// PolicyVersionControl.h — Git-Backed GPO Policy Version Control
// Copyright (c) 2026 ExplorerLens Project
//
// Provides PolicyVersionControl for tracking, diffing, and rolling back enterprise
// policy changes with Git-backed immutable history and atomic commit semantics.
//
#pragma once

#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <cstdint>
#include <optional>

namespace ExplorerLens::Engine {

enum class PolicyChangeType : uint8_t {
    Add    = 0,
    Modify = 1,
    Delete = 2,
    Revert = 3
};

struct PolicyDiff {
    std::string      controlId;
    PolicyChangeType changeType{PolicyChangeType::Modify};
    std::string      oldValue;
    std::string      newValue;
};

struct PolicyVersion {
    std::string             hash;
    std::string             author;
    std::chrono::system_clock::time_point timestamp;
    std::string             message;
    std::vector<PolicyDiff> changes;
    uint32_t                parentIndex{0};
};

struct CommitOptions {
    std::string author;
    std::string message;
    bool        allowEmptyCommit{false};
    bool        signCommit{false};
};

class PolicyVersionControl {
public:
    explicit PolicyVersionControl(const std::string& repoPath);
    ~PolicyVersionControl() = default;

    PolicyVersionControl(const PolicyVersionControl&)            = delete;
    PolicyVersionControl& operator=(const PolicyVersionControl&) = delete;
    PolicyVersionControl(PolicyVersionControl&&)                 = default;
    PolicyVersionControl& operator=(PolicyVersionControl&&)      = default;

    // Commit operations
    std::string CommitPolicy(const std::string& policyJson,
                             const CommitOptions& options);
    bool        Rollback(const std::string& targetHash);
    bool        Rollback(uint32_t stepsBack);

    // History and diff
    std::vector<PolicyVersion> GetHistory(uint32_t maxEntries = 50) const;
    std::vector<PolicyDiff>    DiffVersions(const std::string& hashA,
                                             const std::string& hashB) const;

    // Current state
    std::optional<PolicyVersion> GetCurrentVersion() const;
    std::string                  GetCurrentPolicyJson() const;
    bool                         HasUncommittedChanges() const;

    // Tag support
    bool                         TagVersion(const std::string& hash,
                                            const std::string& tag);
    std::optional<PolicyVersion> GetVersionByTag(const std::string& tag) const;
    std::vector<std::string>     ListTags() const;

    // Change notification
    using ChangeCallback = std::function<void(const PolicyVersion&)>;
    void SetChangeCallback(ChangeCallback cb);

private:
    std::string    m_repoPath;
    ChangeCallback m_changeCallback;

    std::string ComputeHash(const std::string& content) const;
    bool        WriteVersion(const PolicyVersion& version) const;
    bool        ValidatePolicyJson(const std::string& json) const;
    std::string BuildCommitPath(const std::string& hash) const;
};

} // namespace ExplorerLens::Engine
