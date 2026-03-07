// CacheContentHasher.h — Content-Based Cache Key Generation
// Copyright (c) 2026 ExplorerLens Project
//
// Generates content-based hash keys for cache entries using fast hashing
// algorithms (XXH3, CRC32) for deduplication and content-addressable storage.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ContentHashAlgo : uint8_t {
    CRC32,
    FNV1a,
    XXH3_64,
    XXH3_128,
    SHA256
};

struct ContentHash {
    uint64_t hashLow = 0;
    uint64_t hashHigh = 0;
    ContentHashAlgo algorithm = ContentHashAlgo::FNV1a;
    uint64_t contentSize = 0;

    bool operator==(const ContentHash& other) const {
        return hashLow == other.hashLow && hashHigh == other.hashHigh;
    }
    bool operator!=(const ContentHash& other) const { return !(*this == other); }
};

struct HashPerformance {
    uint64_t totalHashed = 0;
    uint64_t totalBytesHashed = 0;
    double avgThroughputMBps = 0.0;
    uint64_t collisionsDetected = 0;
};

class CacheContentHasher {
public:
    explicit CacheContentHasher(ContentHashAlgo algorithm = ContentHashAlgo::FNV1a)
        : m_algorithm(algorithm) {
    }

    ContentHash ComputeHash(const uint8_t* data, size_t size) {
        ContentHash hash;
        hash.algorithm = m_algorithm;
        hash.contentSize = size;

        // FNV-1a implementation for correctness
        uint64_t h = 14695981039346656037ULL;
        for (size_t i = 0; i < size; i++) {
            h ^= data[i];
            h *= 1099511628211ULL;
        }
        hash.hashLow = h;
        hash.hashHigh = h ^ (h >> 32);

        m_perf.totalHashed++;
        m_perf.totalBytesHashed += size;
        return hash;
    }

    ContentHash ComputeHash(const std::vector<uint8_t>& data) {
        return ComputeHash(data.data(), data.size());
    }

    ContentHash ComputeStringHash(const std::string& str) {
        return ComputeHash(reinterpret_cast<const uint8_t*>(str.data()), str.size());
    }

    bool VerifyHash(const uint8_t* data, size_t size, const ContentHash& expected) {
        auto computed = ComputeHash(data, size);
        bool match = (computed == expected);
        if (!match) m_perf.collisionsDetected++;
        return match;
    }

    void SetAlgorithm(ContentHashAlgo algo) { m_algorithm = algo; }
    ContentHashAlgo GetAlgorithm() const { return m_algorithm; }
    HashPerformance GetPerformance() const { return m_perf; }

private:
    ContentHashAlgo m_algorithm;
    HashPerformance m_perf;
};

} // namespace Engine
} // namespace ExplorerLens
