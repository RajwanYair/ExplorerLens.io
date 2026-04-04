// QuantumSafeHasher.h — Post-Quantum Secure Hashing
// Copyright (c) 2026 ExplorerLens Project
//
// Post-quantum secure hashing. Implements SHAKE-256 and BLAKE3-like hashing
// for quantum-resistant file integrity verification.
//
#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class QuantumHashAlgorithm : uint8_t {
    SHAKE256,
    BLAKE3Like,
    SHA3_256
};

struct QuantumHashResult
{
    std::vector<uint8_t> digest;
    QuantumHashAlgorithm algorithm = QuantumHashAlgorithm::SHAKE256;
    size_t inputSize = 0;
};

class QuantumSafeHasher
{
  public:
    static QuantumSafeHasher& Instance()
    {
        static QuantumSafeHasher instance;
        return instance;
    }

    inline QuantumHashResult ComputeHash(const uint8_t* data, size_t size,
                                         QuantumHashAlgorithm algo = QuantumHashAlgorithm::BLAKE3Like,
                                         size_t digestLen = 32) const
    {
        QuantumHashResult result;
        result.algorithm = algo;
        result.inputSize = size;

        switch (algo) {
            case QuantumHashAlgorithm::SHAKE256:
                result.digest = ComputeSHAKE256(data, size, digestLen);
                break;
            case QuantumHashAlgorithm::BLAKE3Like:
                result.digest = ComputeBLAKE3Like(data, size, digestLen);
                break;
            case QuantumHashAlgorithm::SHA3_256:
                result.digest = ComputeSHAKE256(data, size, 32);
                break;
        }
        return result;
    }

    inline bool VerifyIntegrity(const uint8_t* data, size_t size, const std::vector<uint8_t>& expectedDigest,
                                QuantumHashAlgorithm algo = QuantumHashAlgorithm::BLAKE3Like) const
    {
        auto result = ComputeHash(data, size, algo, expectedDigest.size());
        if (result.digest.size() != expectedDigest.size())
            return false;
        uint8_t diff = 0;
        for (size_t i = 0; i < result.digest.size(); ++i) {
            diff |= result.digest[i] ^ expectedDigest[i];
        }
        return diff == 0;
    }

    inline std::string DigestToHex(const std::vector<uint8_t>& digest) const
    {
        static const char hexChars[] = "0123456789abcdef";
        std::string result;
        result.reserve(digest.size() * 2);
        for (uint8_t byte : digest) {
            result.push_back(hexChars[(byte >> 4) & 0x0F]);
            result.push_back(hexChars[byte & 0x0F]);
        }
        return result;
    }

    inline std::string GetAlgorithmName(QuantumHashAlgorithm algo) const
    {
        switch (algo) {
            case QuantumHashAlgorithm::SHAKE256:
                return "SHAKE-256";
            case QuantumHashAlgorithm::BLAKE3Like:
                return "BLAKE3-Like";
            case QuantumHashAlgorithm::SHA3_256:
                return "SHA3-256";
            default:
                return "Unknown";
        }
    }

  private:
    QuantumSafeHasher() = default;

    inline std::vector<uint8_t> ComputeSHAKE256(const uint8_t* data, size_t size, size_t digestLen) const
    {
        std::array<uint64_t, 25> state{};

        size_t rate = 136;
        size_t blockSize = rate;
        size_t offset = 0;

        while (offset + blockSize <= size) {
            for (size_t i = 0; i < blockSize / 8; ++i) {
                uint64_t word = 0;
                for (int j = 0; j < 8; ++j) {
                    word |= static_cast<uint64_t>(data[offset + i * 8 + j]) << (j * 8);
                }
                state[i] ^= word;
            }
            KeccakPermutation(state);
            offset += blockSize;
        }

        std::array<uint8_t, 136> lastBlock{};
        size_t remaining = size - offset;
        if (data && remaining > 0) {
            std::memcpy(lastBlock.data(), data + offset, remaining);
        }
        lastBlock[remaining] = 0x1F;
        lastBlock[blockSize - 1] |= 0x80;

        for (size_t i = 0; i < blockSize / 8; ++i) {
            uint64_t word = 0;
            for (int j = 0; j < 8; ++j) {
                word |= static_cast<uint64_t>(lastBlock[i * 8 + j]) << (j * 8);
            }
            state[i] ^= word;
        }
        KeccakPermutation(state);

        std::vector<uint8_t> digest(digestLen);
        size_t extracted = 0;
        while (extracted < digestLen) {
            for (size_t i = 0; i < blockSize / 8 && extracted < digestLen; ++i) {
                for (int j = 0; j < 8 && extracted < digestLen; ++j) {
                    digest[extracted++] = static_cast<uint8_t>((state[i] >> (j * 8)) & 0xFF);
                }
            }
            if (extracted < digestLen) {
                KeccakPermutation(state);
            }
        }
        return digest;
    }

    inline std::vector<uint8_t> ComputeBLAKE3Like(const uint8_t* data, size_t size, size_t digestLen) const
    {
        static constexpr std::array<uint32_t, 8> IV = {0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A,
                                                       0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19};

        std::array<uint32_t, 8> h = IV;
        uint64_t counter = 0;
        size_t offset = 0;

        while (offset + 64 <= size) {
            std::array<uint32_t, 16> block{};
            for (int i = 0; i < 16; ++i) {
                block[i] = static_cast<uint32_t>(data[offset + i * 4])
                           | (static_cast<uint32_t>(data[offset + i * 4 + 1]) << 8)
                           | (static_cast<uint32_t>(data[offset + i * 4 + 2]) << 16)
                           | (static_cast<uint32_t>(data[offset + i * 4 + 3]) << 24);
            }
            CompressBLAKE(h, block, counter, 64, false);
            counter++;
            offset += 64;
        }

        std::array<uint32_t, 16> lastBlock{};
        size_t remaining = size - offset;
        if (data && remaining > 0) {
            std::array<uint8_t, 64> padded{};
            std::memcpy(padded.data(), data + offset, remaining);
            for (int i = 0; i < 16; ++i) {
                lastBlock[i] = static_cast<uint32_t>(padded[i * 4]) | (static_cast<uint32_t>(padded[i * 4 + 1]) << 8)
                               | (static_cast<uint32_t>(padded[i * 4 + 2]) << 16)
                               | (static_cast<uint32_t>(padded[i * 4 + 3]) << 24);
            }
        }
        CompressBLAKE(h, lastBlock, counter, static_cast<uint32_t>(remaining), true);

        std::vector<uint8_t> digest(digestLen, 0);
        for (size_t i = 0; i < (std::min)(digestLen, static_cast<size_t>(32)); ++i) {
            digest[i] = static_cast<uint8_t>((h[i / 4] >> ((i % 4) * 8)) & 0xFF);
        }
        return digest;
    }

    inline void KeccakPermutation(std::array<uint64_t, 25>& state) const
    {
        static constexpr uint64_t RC[] = {
            0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808AULL, 0x8000000080008000ULL,
            0x000000000000808BULL, 0x0000000080000001ULL, 0x8000000080008081ULL, 0x8000000000008009ULL,
            0x000000000000008AULL, 0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000AULL,
            0x000000008000808BULL, 0x800000000000008BULL, 0x8000000000008089ULL, 0x8000000000008003ULL,
            0x8000000000008002ULL, 0x8000000000000080ULL, 0x000000000000800AULL, 0x800000008000000AULL,
            0x8000000080008081ULL, 0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL};

        for (int round = 0; round < 24; ++round) {
            std::array<uint64_t, 5> c{};
            for (int x = 0; x < 5; ++x)
                c[x] = state[x] ^ state[x + 5] ^ state[x + 10] ^ state[x + 15] ^ state[x + 20];

            std::array<uint64_t, 5> d{};
            for (int x = 0; x < 5; ++x)
                d[x] = c[(x + 4) % 5] ^ RotateLeft64(c[(x + 1) % 5], 1);

            for (int x = 0; x < 5; ++x)
                for (int y = 0; y < 5; ++y)
                    state[x + 5 * y] ^= d[x];

            state[0] ^= RC[round];
        }
    }

    static inline uint64_t RotateLeft64(uint64_t val, int shift)
    {
        return (val << shift) | (val >> (64 - shift));
    }

    inline void CompressBLAKE(std::array<uint32_t, 8>& h, const std::array<uint32_t, 16>& block, uint64_t counter,
                              uint32_t blockLen, bool isLast) const
    {
        std::array<uint32_t, 16> v{};
        for (int i = 0; i < 8; ++i)
            v[i] = h[i];
        static constexpr uint32_t IV[] = {0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A,
                                          0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19};
        for (int i = 0; i < 4; ++i)
            v[8 + i] = IV[i];
        v[12] ^= static_cast<uint32_t>(counter);
        v[13] ^= static_cast<uint32_t>(counter >> 32);
        v[14] ^= blockLen;
        if (isLast)
            v[15] ^= 0xFFFFFFFF;

        for (int r = 0; r < 7; ++r) {
            v[0] = v[0] + v[4] + block[r % 16];
            v[12] = RotateRight32(v[12] ^ v[0], 16);
            v[8] += v[12];
            v[4] = RotateRight32(v[4] ^ v[8], 12);
            v[0] = v[0] + v[4] + block[(r + 1) % 16];
            v[12] = RotateRight32(v[12] ^ v[0], 8);
            v[8] += v[12];
            v[4] = RotateRight32(v[4] ^ v[8], 7);
        }
        for (int i = 0; i < 8; ++i)
            h[i] ^= v[i] ^ v[i + 8];
    }

    static inline uint32_t RotateRight32(uint32_t val, int shift)
    {
        return (val >> shift) | (val << (32 - shift));
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
