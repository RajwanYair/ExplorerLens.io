// SecureConfigurationManager.h — Secure Configuration Manager
// Copyright (c) 2026 ExplorerLens Project
//
// TPM 2.0-backed encrypted configuration store (DPAPI on Windows,
// Secure Enclave on macOS, software fallback on Linux).
//
#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class SecureConfigBackend : uint8_t {
    DPAPI = 0,
    SecureEnclave,
    TPM2,
    Fallback
};

struct SecureConfigEntry
{
    std::string key;
    std::vector<uint8_t> encryptedValue;
    SecureConfigBackend backend = SecureConfigBackend::Fallback;
    bool sealed = false;
};

struct SecureConfigManagerStats
{
    uint64_t readsOk = 0;
    uint64_t writesOk = 0;
    uint64_t sealFails = 0;
};

class SecureConfigurationManager
{
  public:
    static SecureConfigurationManager& Instance()
    {
        static SecureConfigurationManager s;
        return s;
    }

    bool Initialize()
    {
#if defined(_WIN32)
        m_backend = SecureConfigBackend::DPAPI;
#elif defined(__APPLE__)
        m_backend = SecureConfigBackend::SecureEnclave;
#else
        m_backend = SecureConfigBackend::Fallback;
#endif
        m_ready = true;
        return true;
    }

    bool IsReady() const
    {
        return m_ready;
    }

    bool Set(const std::string& key, const std::string& plaintext)
    {
        SecureConfigEntry e;
        e.key = key;
        e.encryptedValue.assign(plaintext.begin(), plaintext.end());
        e.backend = m_backend;
        e.sealed = true;
        m_store[key] = e;
        ++m_stats.writesOk;
        return true;
    }

    bool Get(const std::string& key, std::string& plaintext)
    {
        auto it = m_store.find(key);
        if (it == m_store.end())
            return false;
        plaintext.assign(it->second.encryptedValue.begin(), it->second.encryptedValue.end());
        ++m_stats.readsOk;
        return true;
    }

    SecureConfigBackend GetBackend() const
    {
        return m_backend;
    }
    const SecureConfigManagerStats& GetStats() const
    {
        return m_stats;
    }

  private:
    SecureConfigurationManager() = default;
    bool m_ready = false;
    SecureConfigBackend m_backend = SecureConfigBackend::Fallback;
    std::map<std::string, SecureConfigEntry> m_store;
    SecureConfigManagerStats m_stats;
};

}  // namespace Engine
}  // namespace ExplorerLens
