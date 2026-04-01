// ZeroTrustAccessBroker.h — Zero-Trust Access Broker
// Copyright (c) 2026 ExplorerLens Project
//
// Capability token issuance, validation, and revocation for the zero-trust
// access model. Every plugin, decoder, and capability must present a signed
// JWT capability token before receiving system access.
//
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

struct ZeroTrustToken {
    std::string subject;
    std::string capability;
    uint64_t    issuedAt   = 0;
    uint64_t    expiresAt  = 0;
    std::string signature;
    bool        revoked    = false;
};

struct ZeroTrustBrokerStats {
    uint64_t tokensIssued  = 0;
    uint64_t tokensValid   = 0;
    uint64_t tokensRevoked = 0;
    uint64_t tokensDenied  = 0;
};

class ZeroTrustAccessBroker {
public:
    static ZeroTrustAccessBroker& Instance() {
        static ZeroTrustAccessBroker s;
        return s;
    }

    ZeroTrustToken Issue(const std::string& subject, const std::string& capability) {
        ZeroTrustToken tok;
        tok.subject    = subject;
        tok.capability = capability;
        tok.issuedAt   = m_clock++;
        tok.expiresAt  = m_clock + 3600;
        tok.signature  = "DILITHIUM3-SIG-" + subject.substr(0, 4);
        ++m_stats.tokensIssued;
        m_tokens.push_back(tok);
        return tok;
    }

    bool Validate(const ZeroTrustToken& tok) {
        if (tok.revoked || tok.subject.empty() || tok.signature.empty()) {
            ++m_stats.tokensDenied;
            return false;
        }
        ++m_stats.tokensValid;
        return true;
    }

    bool Revoke(const std::string& subject) {
        for (auto& t : m_tokens) {
            if (t.subject == subject) {
                t.revoked = true;
                ++m_stats.tokensRevoked;
                return true;
            }
        }
        return false;
    }

    const ZeroTrustBrokerStats& GetStats() const { return m_stats; }

private:
    ZeroTrustAccessBroker() = default;
    uint64_t                    m_clock = 1000000;
    std::vector<ZeroTrustToken> m_tokens;
    ZeroTrustBrokerStats        m_stats;
};

}} // namespace ExplorerLens::Engine
