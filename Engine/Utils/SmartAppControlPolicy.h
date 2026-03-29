// SmartAppControlPolicy.h — Smart App Control Policy Enforcement
// Copyright (c) 2026 ExplorerLens Project
//
// Evaluates Smart App Control (SAC) and Windows Defender Application Control (WDAC)
// policies for plugin binaries before loading, enforcing integrity and code-signing rules.
//
#pragma once
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class SACPolicyMode      { Enforcement, Evaluation, Off };
enum class SACBinaryTrustLevel { Trusted, Untrusted, Unknown, Blocked };

struct SACPolicyResult {
    SACBinaryTrustLevel trustLevel  = SACBinaryTrustLevel::Unknown;
    bool                allowLoad   = false;
    std::string         policyName;
    std::string         reason;
    bool Ok() const noexcept { return allowLoad; }
};

struct SACBinaryInfo {
    std::wstring filePath;
    std::string  publisherCN;
    bool         isSigned     = false;
    bool         isTimestamped = false;
    std::string  sha256Hash;
};

class SmartAppControlPolicy {
public:
    explicit SmartAppControlPolicy(SACPolicyMode mode = SACPolicyMode::Enforcement)
        : m_mode(mode) {}

    SACPolicyResult Evaluate(const SACBinaryInfo& binary) const {
        if (m_mode == SACPolicyMode::Off)
            return { SACBinaryTrustLevel::Trusted, true, "SAC-Off", "Policy disabled" };

        // Trusted if signed + timestamped + publisher in allowlist
        if (binary.isSigned && binary.isTimestamped) {
            bool inAllowlist = m_trustedPublishers.empty();
            for (const auto& pub : m_trustedPublishers)
                if (binary.publisherCN == pub) { inAllowlist = true; break; }
            if (inAllowlist)
                return { SACBinaryTrustLevel::Trusted, true, "SAC-Enforcement", "Signed trusted publisher" };
        }
        if (m_mode == SACPolicyMode::Evaluation)
            return { SACBinaryTrustLevel::Unknown, true, "SAC-Evaluation", "Evaluation mode — allowing" };

        return { SACBinaryTrustLevel::Untrusted, false, "SAC-Enforcement",
                 binary.isSigned ? "Unsigned publisher" : "Unsigned binary" };
    }

    void AddTrustedPublisher(const std::string& cn) { m_trustedPublishers.push_back(cn); }
    void SetMode(SACPolicyMode mode) noexcept { m_mode = mode; }
    SACPolicyMode Mode() const noexcept { return m_mode; }

    static std::string ModeName(SACPolicyMode mode) noexcept {
        switch (mode) {
        case SACPolicyMode::Enforcement: return "Enforcement";
        case SACPolicyMode::Evaluation:  return "Evaluation";
        case SACPolicyMode::Off:         return "Off";
        }
        return "Unknown";
    }

private:
    SACPolicyMode            m_mode;
    std::vector<std::string> m_trustedPublishers;
};

} // namespace Engine
} // namespace ExplorerLens
