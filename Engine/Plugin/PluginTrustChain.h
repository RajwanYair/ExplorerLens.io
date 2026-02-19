#pragma once
// Sprint 154 — Plugin Marketplace Trust Workflow
// Certificate chain validation, trust levels, revocation, publisher policy enforcement.

#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <functional>

namespace DarkThumbs::Plugin {

// ─── Trust level ──────────────────────────────────────────────────────────────

enum class PluginTrustLevel : uint32_t {
    Trusted     = 0,  // verified by DarkThumbs signing authority
    Verified    = 1,  // signed by known 3rd-party certificate
    Community   = 2,  // self-signed, user-accepted
    Untrusted   = 3,  // no valid signature
    Blocked     = 4,  // explicitly revoked or blocked
};

inline std::string ToString(PluginTrustLevel t) {
    switch (t) {
        case PluginTrustLevel::Trusted:    return "Trusted";
        case PluginTrustLevel::Verified:   return "Verified";
        case PluginTrustLevel::Community:  return "Community";
        case PluginTrustLevel::Untrusted:  return "Untrusted";
        case PluginTrustLevel::Blocked:    return "Blocked";
        default: return "Unknown";
    }
}

// ─── Certificate chain ────────────────────────────────────────────────────────

struct CertificateEntry {
    std::string     subject;
    std::string     issuer;
    std::string     thumbprint;       // SHA-256 hex
    uint64_t        notBeforeEpoch  { 0 };
    uint64_t        notAfterEpoch   { 0 };
    bool            isRootCA        { false };

    bool IsExpired(uint64_t nowEpoch) const {
        return nowEpoch > notAfterEpoch;
    }
};

struct CertificateChain {
    std::vector<CertificateEntry>   entries;   // leaf → root order
    bool                            isComplete { false };

    const CertificateEntry* Leaf() const {
        return entries.empty() ? nullptr : &entries.front();
    }
    const CertificateEntry* Root() const {
        return entries.empty() ? nullptr : &entries.back();
    }
};

// ─── Signature block ──────────────────────────────────────────────────────────

struct PluginSignatureBlock {
    std::string         algorithmId;    // "sha256WithRSAEncryption" etc.
    std::string         signatureHex;   // hex-encoded signature bytes
    CertificateChain    chain;
    uint64_t            timestampEpoch  { 0 };  // RFC 3161 trusted timestamp
    std::string         publisherId;    // from Subject CN of leaf cert

    bool HasTimestamp() const { return timestampEpoch != 0; }
};

// ─── Revocation entry ────────────────────────────────────────────────────────

struct RevocationEntry {
    std::string thumbprint;     // certificate thumbprint to revoke
    std::string reason;
    uint64_t    revokedAtEpoch  { 0 };
};

// ─── Publisher allow/block lists ─────────────────────────────────────────────

struct PublisherPolicy {
    std::vector<std::string>    allowedPublishers;  // CN values
    std::vector<std::string>    blockedPublishers;
    std::vector<RevocationEntry> revokedCerts;
    bool                        enforceStrict   { false };  // block Community when true

    bool IsPublisherAllowed(const std::string& publisherId) const {
        for (const auto& b : blockedPublishers)
            if (b == publisherId) return false;
        if (allowedPublishers.empty()) return true;  // open list
        for (const auto& a : allowedPublishers)
            if (a == publisherId) return true;
        return false;
    }

    bool IsCertRevoked(const std::string& thumbprint) const {
        for (const auto& r : revokedCerts)
            if (r.thumbprint == thumbprint) return true;
        return false;
    }

    static PublisherPolicy Open() { return {}; }
    static PublisherPolicy DarkThumbsOfficial() {
        PublisherPolicy p;
        p.allowedPublishers = { "DarkThumbs", "Intel Corporation" };
        p.enforceStrict = false;
        return p;
    }
    static PublisherPolicy Enterprise() {
        PublisherPolicy p;
        p.enforceStrict = true;
        return p;
    }
};

// ─── Trust chain validator ────────────────────────────────────────────────────

struct TrustValidationResult {
    PluginTrustLevel    level           { PluginTrustLevel::Untrusted };
    bool                allowed         { false };
    std::string         reason;

    static TrustValidationResult Allow(PluginTrustLevel l, std::string msg = "") {
        return { l, true, std::move(msg) };
    }
    static TrustValidationResult Deny(std::string msg) {
        return { PluginTrustLevel::Blocked, false, std::move(msg) };
    }
};

class PluginTrustChainValidator {
public:
    explicit PluginTrustChainValidator(PublisherPolicy policy, uint64_t nowEpoch = 0)
        : m_policy(std::move(policy)), m_nowEpoch(nowEpoch) {}

    TrustValidationResult Validate(const PluginSignatureBlock& sig) const {
        // Check basic signature presence
        if (sig.signatureHex.empty() || sig.chain.entries.empty())
            return TrustValidationResult::Deny("no valid signature found");

        // Check revocation
        if (sig.chain.Leaf()) {
            if (m_policy.IsCertRevoked(sig.chain.Leaf()->thumbprint))
                return TrustValidationResult::Deny("certificate revoked");
        }

        // Check publisher
        if (!m_policy.IsPublisherAllowed(sig.publisherId))
            return TrustValidationResult::Deny("publisher '" + sig.publisherId + "' not allowed");

        // Check expiry
        if (m_nowEpoch > 0 && sig.chain.Leaf()) {
            if (sig.chain.Leaf()->IsExpired(m_nowEpoch))
                return TrustValidationResult::Deny("signing certificate expired");
        }

        // Classify trust level
        bool isTrusted = false;
        for (const auto& pub : m_policy.allowedPublishers)
            if (pub == sig.publisherId) { isTrusted = true; break; }

        if (isTrusted)
            return TrustValidationResult::Allow(PluginTrustLevel::Trusted, "official publisher");

        if (sig.chain.isComplete)
            return TrustValidationResult::Allow(PluginTrustLevel::Verified, "verified chain");

        if (m_policy.enforceStrict)
            return TrustValidationResult::Deny("strict policy requires Trusted or Verified");

        return TrustValidationResult::Allow(PluginTrustLevel::Community, "self-signed accepted");
    }

private:
    PublisherPolicy m_policy;
    uint64_t        m_nowEpoch;
};

// ─── Trust badge UI mapping ───────────────────────────────────────────────────

struct TrustBadge {
    PluginTrustLevel    level;
    std::string         iconId;     // resource ID
    std::string         tooltip;

    static TrustBadge For(PluginTrustLevel l) {
        switch (l) {
            case PluginTrustLevel::Trusted:    return { l, "icon_trust_verified",  "Officially verified by DarkThumbs" };
            case PluginTrustLevel::Verified:   return { l, "icon_trust_checked",   "Signed by verified publisher" };
            case PluginTrustLevel::Community:  return { l, "icon_trust_community", "Community plugin — use with care" };
            case PluginTrustLevel::Untrusted:  return { l, "icon_trust_none",      "Unsigned plugin — not recommended" };
            case PluginTrustLevel::Blocked:    return { l, "icon_trust_blocked",   "Plugin is blocked or revoked" };
            default: return { l, "icon_trust_unknown", "Unknown trust level" };
        }
    }
};

} // namespace DarkThumbs::Plugin
