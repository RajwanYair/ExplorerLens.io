// CryptoAgilityBroker.h — Cryptographic Agility Broker
// Copyright (c) 2026 ExplorerLens Project
//
// Centralizes algorithm selection, negotiation, and migration. Allows runtime
// switching from classical to PQC algorithms without code changes in consumers.
//
#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class CryptoCategory {
    KEM,
    Signature,
    Hash,
    MAC
};
enum class CryptoAlgoId {
    ECDH_X25519,
    ECDH_P256,
    ECDSA_P256,
    RSA_PSS_2048,
    ML_KEM_768,
    ML_KEM_1024,
    ML_DSA_65,
    ML_DSA_87,
    SLH_DSA_128,
    SLH_DSA_256,
    SHA2_256,
    SHA3_256,
    HMAC_SHA256
};

struct AlgoCapability
{
    CryptoAlgoId id;
    CryptoCategory category;
    std::string name;
    bool pqcSafe = false;
    uint32_t securityBits = 0;
    bool fipsApproved = false;
};

class CryptoAgilityBroker
{
  public:
    CryptoAgilityBroker() = default;

    bool Initialize()
    {
        RegisterDefaults();
        m_ready = true;
        return true;
    }
    bool IsReady() const
    {
        return m_ready;
    }

    void SetPreferred(CryptoCategory cat, CryptoAlgoId id)
    {
        m_preferred[static_cast<int>(cat)] = id;
    }

    CryptoAlgoId GetPreferred(CryptoCategory cat) const
    {
        auto it = m_preferred.find(static_cast<int>(cat));
        if (it != m_preferred.end())
            return it->second;
        return CryptoAlgoId::ML_KEM_768;
    }

    bool IsAlgoAvailable(CryptoAlgoId id) const
    {
        return m_capabilities.count(static_cast<int>(id)) > 0;
    }

    AlgoCapability GetCapability(CryptoAlgoId id) const
    {
        auto it = m_capabilities.find(static_cast<int>(id));
        if (it != m_capabilities.end())
            return it->second;
        return {};
    }

    std::vector<AlgoCapability> ListByCategory(CryptoCategory cat) const
    {
        std::vector<AlgoCapability> out;
        for (const auto& kv : m_capabilities) {
            if (kv.second.category == cat)
                out.push_back(kv.second);
        }
        return out;
    }

    void Shutdown()
    {
        m_ready = false;
    }

  private:
    bool m_ready = false;
    std::unordered_map<int, CryptoAlgoId> m_preferred;
    std::unordered_map<int, AlgoCapability> m_capabilities;

    void RegisterDefaults()
    {
        auto reg = [&](CryptoAlgoId id, CryptoCategory cat, const std::string& name, bool pqc, uint32_t bits,
                       bool fips) {
            AlgoCapability c;
            c.id = id;
            c.category = cat;
            c.name = name;
            c.pqcSafe = pqc;
            c.securityBits = bits;
            c.fipsApproved = fips;
            m_capabilities[static_cast<int>(id)] = c;
        };
        reg(CryptoAlgoId::ML_KEM_768, CryptoCategory::KEM, "ML-KEM-768", true, 192, true);
        reg(CryptoAlgoId::ML_DSA_65, CryptoCategory::Signature, "ML-DSA-65", true, 128, true);
        reg(CryptoAlgoId::ECDH_X25519, CryptoCategory::KEM, "X25519", false, 128, false);
        reg(CryptoAlgoId::SHA3_256, CryptoCategory::Hash, "SHA3-256", true, 256, true);
        reg(CryptoAlgoId::HMAC_SHA256, CryptoCategory::MAC, "HMAC-SHA256", false, 256, true);

        m_preferred[static_cast<int>(CryptoCategory::KEM)] = CryptoAlgoId::ML_KEM_768;
        m_preferred[static_cast<int>(CryptoCategory::Signature)] = CryptoAlgoId::ML_DSA_65;
        m_preferred[static_cast<int>(CryptoCategory::Hash)] = CryptoAlgoId::SHA3_256;
        m_preferred[static_cast<int>(CryptoCategory::MAC)] = CryptoAlgoId::HMAC_SHA256;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
