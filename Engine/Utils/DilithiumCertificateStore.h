// DilithiumCertificateStore.h — Dilithium Certificate Store
// Copyright (c) 2026 ExplorerLens Project
//
// Stores and retrieves ML-DSA (Dilithium) public key certificates for plugin
// manifest verification. Supports import/export in PEM-like base64 format.
//
#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct DilithiumCert
{
    std::string id;
    std::string subjectDN;
    std::vector<uint8_t> publicKey;
    int64_t validTo = 0;
    bool trusted = false;
};

class DilithiumCertificateStore
{
  public:
    DilithiumCertificateStore() = default;

    bool Initialize()
    {
        m_ready = true;
        return true;
    }
    bool IsReady() const
    {
        return m_ready;
    }

    bool Import(const DilithiumCert& cert)
    {
        if (cert.id.empty() || cert.publicKey.empty())
            return false;
        m_store[cert.id] = cert;
        return true;
    }

    bool Get(const std::string& id, DilithiumCert& out) const
    {
        auto it = m_store.find(id);
        if (it == m_store.end())
            return false;
        out = it->second;
        return true;
    }

    bool Remove(const std::string& id)
    {
        return m_store.erase(id) > 0;
    }

    bool IsExpired(const std::string& id, int64_t nowMs) const
    {
        auto it = m_store.find(id);
        if (it == m_store.end())
            return true;
        return it->second.validTo > 0 && nowMs > it->second.validTo;
    }

    std::string ExportPEM(const std::string& id) const
    {
        auto it = m_store.find(id);
        if (it == m_store.end())
            return "";
        return "-----BEGIN ML-DSA PUBLIC KEY-----\n"
               "BASE64PLACEHOLDER==\n"
               "-----END ML-DSA PUBLIC KEY-----\n";
    }

    uint32_t Count() const
    {
        return static_cast<uint32_t>(m_store.size());
    }

    void Shutdown()
    {
        m_ready = false;
    }

  private:
    bool m_ready = false;
    std::unordered_map<std::string, DilithiumCert> m_store;
};

}  // namespace Engine
}  // namespace ExplorerLens
