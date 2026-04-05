// SecureKeyStore.cpp — Secure cryptographic key storage abstraction
// Copyright (c) 2026 ExplorerLens Project
//
#include "SecureKeyStore.h"
#include <algorithm>

namespace ExplorerLens { namespace Engine {

SecureKeyStore SecureKeyStore::s_instance;

SecureKeyStore::SecureKeyStore()  = default;
SecureKeyStore::~SecureKeyStore() { Shutdown(); }

SecureKeyStore& SecureKeyStore::Instance() noexcept { return s_instance; }

bool SecureKeyStore::Initialize(KeyStoreBackend backend)
{
    m_keys.clear();
    m_backend     = backend;
    m_initialized = true;
    return true;
}

void SecureKeyStore::Shutdown()
{
    // Zero out key material before clearing — don't leave secrets in memory.
    for (auto& k : m_keys)
        std::fill(k.keyMaterial.begin(), k.keyMaterial.end(), uint8_t{0});
    m_keys.clear();
    m_initialized = false;
}

bool SecureKeyStore::StoreKey(const std::string& keyId, const std::vector<uint8_t>& material)
{
    if (!m_initialized || keyId.empty() || material.empty())
        return false;
    for (auto& k : m_keys)
    {
        if (k.keyId == keyId)
        {
            k.keyMaterial = material;
            return true;
        }
    }
    m_keys.push_back({keyId, material, false});
    return true;
}

bool SecureKeyStore::RetrieveKey(const std::string& keyId, std::vector<uint8_t>& outMaterial) const
{
    if (!m_initialized || keyId.empty())
        return false;
    for (const auto& k : m_keys)
    {
        if (k.keyId == keyId)
        {
            outMaterial = k.keyMaterial;
            return true;
        }
    }
    return false;
}

bool SecureKeyStore::DeleteKey(const std::string& keyId)
{
    auto it = std::find_if(m_keys.begin(), m_keys.end(),
                           [&](const KeyEntry& k){ return k.keyId == keyId; });
    if (it == m_keys.end())
        return false;
    std::fill(it->keyMaterial.begin(), it->keyMaterial.end(), uint8_t{0});
    m_keys.erase(it);
    return true;
}

bool SecureKeyStore::HasKey(const std::string& keyId) const
{
    return std::any_of(m_keys.begin(), m_keys.end(),
                       [&](const KeyEntry& k){ return k.keyId == keyId; });
}

}} // namespace ExplorerLens::Engine
