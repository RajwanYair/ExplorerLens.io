// CacheBloomFilter.h — Space-Efficient Negative Cache Lookup
// Copyright (c) 2026 ExplorerLens Project
//
// Provides an O(1) bloom filter for determining whether a cache key is
// definitely NOT present.  Eliminates unnecessary disk I/O for items
// that have never been cached.  False positives are bounded by the
// configured bits-per-element ratio.
//
#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Bloom filter configuration
struct BloomFilterConfig {
    uint32_t expectedElements = 100000;   // Expected number of cache keys
    uint8_t  hashFunctions = 7;        // Number of hash functions (k)
    double   targetFPRate = 0.01;     // Target false-positive rate
    uint32_t bitsPerElement = 10;       // Bits per element (m/n)
};

/// Bloom filter statistics
struct BloomFilterStats {
    uint64_t insertions = 0;
    uint64_t positiveQueries = 0;   // Returned "maybe present"
    uint64_t negativeQueries = 0;   // Returned "definitely absent"
    uint64_t totalQueries = 0;
    uint32_t bitArraySizeBytes = 0;
    double   estimatedFPRate = 0.0;
    double   fillRatio = 0.0;
};

/// Thread-safe bloom filter for cache negative lookups
class CacheBloomFilter {
public:
    explicit CacheBloomFilter(const BloomFilterConfig& config = {})
        : m_config(config) {
        uint64_t totalBits = static_cast<uint64_t>(config.expectedElements) *
            config.bitsPerElement;
        m_bitArraySize = static_cast<uint32_t>((totalBits + 7) / 8);
        m_bits.resize(m_bitArraySize, 0);
        m_totalBits = totalBits;
    }

    /// Insert a key into the bloom filter
    void Insert(const std::string& key) {
        std::lock_guard<std::mutex> lock(m_mutex);
        uint64_t h1 = FNV1a(key);
        uint64_t h2 = MurmurMix(h1);
        for (uint8_t i = 0; i < m_config.hashFunctions; ++i) {
            uint64_t bitIdx = (h1 + i * h2) % m_totalBits;
            m_bits[static_cast<size_t>(bitIdx / 8)] |=
                static_cast<uint8_t>(1u << (bitIdx % 8));
        }
        m_stats.insertions++;
    }

    /// Check if key might be in the cache.
    /// Returns false = definitely NOT cached (safe to skip disk I/O)
    /// Returns true  = maybe cached (must check actual cache)
    bool MayContain(const std::string& key) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        uint64_t h1 = FNV1a(key);
        uint64_t h2 = MurmurMix(h1);
        for (uint8_t i = 0; i < m_config.hashFunctions; ++i) {
            uint64_t bitIdx = (h1 + i * h2) % m_totalBits;
            if (!(m_bits[static_cast<size_t>(bitIdx / 8)] &
                static_cast<uint8_t>(1u << (bitIdx % 8)))) {
                m_stats.negativeQueries++;
                m_stats.totalQueries++;
                return false;
            }
        }
        m_stats.positiveQueries++;
        m_stats.totalQueries++;
        return true;
    }

    /// Reset the filter (e.g., after cache purge)
    void Clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::memset(m_bits.data(), 0, m_bits.size());
        m_stats = {};
    }

    /// Get statistics
    BloomFilterStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto stats = m_stats;
        stats.bitArraySizeBytes = m_bitArraySize;
        // Estimate fill ratio
        uint64_t setBits = 0;
        for (auto byte : m_bits) {
            uint8_t b = byte;
            while (b) { setBits += b & 1; b >>= 1; }
        }
        stats.fillRatio = (m_totalBits > 0) ?
            static_cast<double>(setBits) / static_cast<double>(m_totalBits) : 0.0;
        stats.estimatedFPRate = std::pow(stats.fillRatio, m_config.hashFunctions);
        return stats;
    }

    uint32_t SizeBytes() const { return m_bitArraySize; }

private:
    /// FNV-1a 64-bit hash
    static inline uint64_t FNV1a(const std::string& key) {
        uint64_t hash = 14695981039346656037ULL;
        for (auto c : key) {
            hash ^= static_cast<uint64_t>(static_cast<uint8_t>(c));
            hash *= 1099511628211ULL;
        }
        return hash;
    }

    /// Murmur-style mix for double hashing
    static inline uint64_t MurmurMix(uint64_t h) {
        h ^= h >> 33;
        h *= 0xFF51AFD7ED558CCDULL;
        h ^= h >> 33;
        h *= 0xC4CEB9FE1A85EC53ULL;
        h ^= h >> 33;
        return h;
    }

    BloomFilterConfig        m_config;
    std::vector<uint8_t>     m_bits;
    uint32_t                 m_bitArraySize = 0;
    uint64_t                 m_totalBits = 0;
    mutable std::mutex       m_mutex;
    mutable BloomFilterStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
