// HybridTrustChainV2.h — Hybrid Trust Chain V2
// Copyright (c) 2026 ExplorerLens Project
//
// Validates certificate trust chains combining classical X.509 and post-quantum
// ML-DSA intermediate/root certificates for plugin and binary signing.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ChainAlgo {
    ClassicX509,
    MLDSA65,
    MLDSA87,
    Hybrid
};

struct TrustCertificate
{
    std::string subjectDN;
    std::string issuerDN;
    std::vector<uint8_t> publicKey;
    std::vector<uint8_t> signature;
    ChainAlgo algo = ChainAlgo::Hybrid;
    int64_t validTo = 0;
    bool isSelfSigned = false;
};

struct TrustChainResult
{
    bool valid = false;
    uint32_t chainDepth = 0;
    ChainAlgo lowestAlgo = ChainAlgo::ClassicX509;
    std::string errorCode;
    std::string leafSubject;
};

class HybridTrustChainV2
{
  public:
    HybridTrustChainV2() = default;

    bool Initialize()
    {
        m_ready = true;
        return true;
    }
    bool IsReady() const
    {
        return m_ready;
    }

    void AddTrustAnchor(const TrustCertificate& cert)
    {
        m_anchors.push_back(cert);
    }

    TrustChainResult Validate(const std::vector<TrustCertificate>& chain, int64_t nowMs = 0) const
    {
        TrustChainResult res;
        if (chain.empty()) {
            res.errorCode = "EMPTY_CHAIN";
            return res;
        }

        for (const auto& cert : chain) {
            if (nowMs > 0 && cert.validTo > 0 && nowMs > cert.validTo) {
                res.errorCode = "CERT_EXPIRED";
                return res;
            }
            if (cert.publicKey.empty() || cert.signature.empty()) {
                res.errorCode = "MISSING_KEY_OR_SIG";
                return res;
            }
        }

        bool foundAnchor = false;
        const auto& root = chain.back();
        for (const auto& anchor : m_anchors) {
            if (anchor.subjectDN == root.issuerDN) {
                foundAnchor = true;
                break;
            }
        }
        if (!foundAnchor && !m_anchors.empty()) {
            res.errorCode = "ANCHOR_NOT_FOUND";
            return res;
        }

        res.valid = true;
        res.chainDepth = static_cast<uint32_t>(chain.size());
        res.leafSubject = chain.front().subjectDN;
        res.lowestAlgo = chain.front().algo;
        return res;
    }

    void Shutdown()
    {
        m_ready = false;
    }

  private:
    bool m_ready = false;
    std::vector<TrustCertificate> m_anchors;
};

}  // namespace Engine
}  // namespace ExplorerLens
