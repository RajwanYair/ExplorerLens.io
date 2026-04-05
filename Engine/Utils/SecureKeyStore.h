// SecureKeyStore.h — Secure cryptographic key storage abstraction
// Copyright (c) 2026 ExplorerLens Project
//
// Provides a platform-neutral API for storing and retrieving cryptographic keys
// used by plugin signing validation, enterprise policy signing, and telemetry
// HMAC tokens. Backed by DPAPI on Windows, Keychain on macOS, and libsecret on
// Linux. All keys are AES-256-encrypted at rest; never stored in plain text.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class KeyStoreBackend : uint8_t
{
    DPAPI      = 0,
    Keychain   = 1,
    LibSecret  = 2,
    InMemory   = 3,
    None       = 255,
};

struct KeyEntry
{
    std::string          keyId;
    std::vector<uint8_t> keyMaterial;
    bool                 persisted = false;
};

class SecureKeyStore
{
public:
    SecureKeyStore();
    ~SecureKeyStore();

    SecureKeyStore(const SecureKeyStore&)            = delete;
    SecureKeyStore& operator=(const SecureKeyStore&) = delete;

    bool            Initialize(KeyStoreBackend backend = KeyStoreBackend::None);
    void            Shutdown();
    bool            StoreKey(const std::string& keyId, const std::vector<uint8_t>& material);
    bool            RetrieveKey(const std::string& keyId, std::vector<uint8_t>& outMaterial) const;
    bool            DeleteKey(const std::string& keyId);
    bool            HasKey(const std::string& keyId)  const;
    uint32_t        KeyCount()                         const noexcept { return static_cast<uint32_t>(m_keys.size()); }
    KeyStoreBackend Backend()                          const noexcept { return m_backend; }

    static SecureKeyStore& Instance() noexcept;

private:
    std::vector<KeyEntry>  m_keys;
    KeyStoreBackend        m_backend     = KeyStoreBackend::None;
    bool                   m_initialized = false;
    static SecureKeyStore  s_instance;
};

}} // namespace ExplorerLens::Engine
