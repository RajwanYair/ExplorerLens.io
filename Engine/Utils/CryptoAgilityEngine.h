// CryptoAgilityEngine.h — Cryptographic Agility Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Provides runtime cryptographic algorithm negotiation and hot-swap capabilities,
// enabling seamless migration between classical and post-quantum algorithms
// without service interruption or binary redeployment.
//
#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class CryptoRole {
    KeyExchange,
    Signature,
    SymmetricEncryption,
    Hash,
    MAC
};
enum class CryptoPreference {
    Classical,
    PostQuantum,
    Hybrid,
    Auto
};

struct CryptoAlgorithmDescriptor
{
    std::string id;
    CryptoRole role = CryptoRole::KeyExchange;
    bool fipsApproved = false;
    bool postQuantum = false;
    int securityBits = 128;
    int priority = 50;  // lower = higher preference
};

struct CryptoNegotiationResult
{
    bool success = false;
    std::string selected;
    CryptoRole role = CryptoRole::KeyExchange;
    bool isFIPS = false;
    std::string errorMsg;
    bool Ok() const noexcept
    {
        return success;
    }
};

using AlgorithmFilter = std::function<bool(const CryptoAlgorithmDescriptor&)>;

class CryptoAgilityEngine
{
  public:
    explicit CryptoAgilityEngine(CryptoPreference pref = CryptoPreference::Hybrid) : m_preference(pref)
    {
        // Built-in algorithm registry
        Register({"ML-KEM-768", CryptoRole::KeyExchange, true, true, 192, 10});
        Register({"ECDH-P384", CryptoRole::KeyExchange, true, false, 192, 20});
        Register({"SLH-DSA-128s", CryptoRole::Signature, true, true, 128, 10});
        Register({"ECDSA-P384", CryptoRole::Signature, true, false, 192, 20});
        Register({"AES-256-GCM", CryptoRole::SymmetricEncryption, true, false, 256, 10});
        Register({"SHA-256", CryptoRole::Hash, true, false, 128, 10});
        Register({"HMAC-SHA-256", CryptoRole::MAC, true, false, 128, 10});
    }

    void Register(CryptoAlgorithmDescriptor desc)
    {
        m_algorithms[desc.id] = std::move(desc);
    }

    CryptoNegotiationResult Negotiate(CryptoRole role, const std::vector<std::string>& clientSupported) const
    {
        const CryptoAlgorithmDescriptor* best = nullptr;
        for (const auto& id : clientSupported) {
            auto it = m_algorithms.find(id);
            if (it == m_algorithms.end())
                continue;
            const auto& algo = it->second;
            if (algo.role != role)
                continue;
            if (!algo.fipsApproved && m_requireFIPS)
                continue;
            if (m_preference == CryptoPreference::PostQuantum && !algo.postQuantum)
                continue;
            if (m_preference == CryptoPreference::Classical && algo.postQuantum)
                continue;
            if (!best || algo.priority < best->priority)
                best = &algo;
        }
        if (!best)
            return {false, {}, role, false, "No mutually supported algorithm"};
        return {true, best->id, role, best->fipsApproved, {}};
    }

    void SetRequireFIPS(bool require) noexcept
    {
        m_requireFIPS = require;
    }
    void SetPreference(CryptoPreference pref) noexcept
    {
        m_preference = pref;
    }
    size_t AlgorithmCount() const noexcept
    {
        return m_algorithms.size();
    }

  private:
    std::unordered_map<std::string, CryptoAlgorithmDescriptor> m_algorithms;
    CryptoPreference m_preference = CryptoPreference::Hybrid;
    bool m_requireFIPS = true;
};

}  // namespace Engine
}  // namespace ExplorerLens
