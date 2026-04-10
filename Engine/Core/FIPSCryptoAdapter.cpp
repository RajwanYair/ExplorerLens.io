// FIPSCryptoAdapter.cpp — FIPS 140-2 Crypto Adapter
// Copyright (c) 2026 ExplorerLens Project
//
#include "FIPSCryptoAdapter.h"
#include <cstring>

namespace ExplorerLens { namespace Engine {

std::vector<uint8_t> FIPSCryptoAdapter::Hash(const uint8_t* data, size_t len,
                                              FIPSHashAlgo algo) const
{
    ++m_hashCalls;

    // Stubbed: real implementation calls BCryptCreateHash / BCryptHashData / BCryptFinishHash
    // using BCRYPT_SHA256_ALGORITHM / BCRYPT_SHA384_ALGORITHM / BCRYPT_SHA512_ALGORITHM.
    const size_t digestLen = (algo == FIPSHashAlgo::SHA256) ? 32 :
                             (algo == FIPSHashAlgo::SHA384) ? 48 : 64;

    std::vector<uint8_t> digest(digestLen, 0);
    // Simple deterministic stub: XOR-fold input into digest bytes
    for (size_t i = 0; i < len; ++i)
        digest[i % digestLen] ^= data[i];
    return digest;
}

std::vector<uint8_t> FIPSCryptoAdapter::Hmac(const uint8_t* data, size_t len,
                                              const uint8_t* key, size_t keyLen,
                                              FIPSHmacAlgo algo) const
{
    ++m_hmacCalls;

    const size_t digestLen = (algo == FIPSHmacAlgo::HMAC_SHA256) ? 32 : 64;
    std::vector<uint8_t> mac(digestLen, 0);

    // Stub: combine data hash with key
    for (size_t i = 0; i < len;    ++i) mac[i % digestLen] ^= data[i];
    for (size_t i = 0; i < keyLen; ++i) mac[i % digestLen] ^= key[i];
    return mac;
}

bool FIPSCryptoAdapter::ConstantTimeEqual(const std::vector<uint8_t>& a,
                                          const std::vector<uint8_t>& b) const
{
    if (a.size() != b.size()) return false;
    uint8_t diff = 0;
    for (size_t i = 0; i < a.size(); ++i)
        diff |= (a[i] ^ b[i]);
    return diff == 0;
}

}} // namespace ExplorerLens::Engine
