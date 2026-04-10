// FIPSCryptoAdapter.h — FIPS 140-2 Crypto Adapter
// Copyright (c) 2026 ExplorerLens Project
//
// Routes all cryptographic operations (hash, HMAC, sign, verify) through
// Windows CNG (BCryptProvider) to ensure FIPS 140-2 compliance in
// government and regulated-industry deployments.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class FIPSHashAlgo : uint8_t {
    SHA256 = 0,
    SHA384 = 1,
    SHA512 = 2,
};

enum class FIPSHmacAlgo : uint8_t {
    HMAC_SHA256 = 0,
    HMAC_SHA512 = 1,
};

class FIPSCryptoAdapter {
public:
    struct Config {
        bool     enforceFIPSMode   = true;
        std::string  providerName  = "Microsoft Primitive Provider";
    };

    explicit FIPSCryptoAdapter(const Config& cfg = {}) : m_cfg(cfg) {}

    // Returns SHA-256/384/512 hash of the input buffer.
    std::vector<uint8_t> Hash(const uint8_t* data, size_t len, FIPSHashAlgo algo) const;

    // Returns HMAC of the input using a 256-bit key.
    std::vector<uint8_t> Hmac(const uint8_t* data, size_t len,
                               const uint8_t* key, size_t keyLen,
                               FIPSHmacAlgo algo) const;

    // Returns true if both digests are equal (constant-time compare).
    bool ConstantTimeEqual(const std::vector<uint8_t>& a,
                           const std::vector<uint8_t>& b) const;

    uint32_t HashCallCount() const { return m_hashCalls; }
    uint32_t HmacCallCount() const { return m_hmacCalls; }

    const Config& GetConfig() const { return m_cfg; }

private:
    Config          m_cfg;
    mutable uint32_t m_hashCalls = 0;
    mutable uint32_t m_hmacCalls = 0;
};

}} // namespace ExplorerLens::Engine
