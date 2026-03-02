//==============================================================================
// ExplorerLens Engine — Thumbnail Search Index
//
// Purpose:
//   Searchable index of thumbnail content using perceptual hashing (pHash)
//   and 32-dimensional feature vectors.  Enables finding visually similar
//   thumbnails by Hamming distance on the hash and cosine similarity on the
//   feature vectors.
//
// Classes:
//   ThumbnailSearchIndex — Thread-safe indexer that computes a 64-bit pHash
//   (via 8x8 DCT on a 32x32 grayscale tile) and a 32-float feature vector
//   (16-bin color histogram + 8 edge-orientation bins + 4 GLCM texture
//   features + 4 spatial moments) per image.
//
// Inputs:
//   - RGBA pixel buffer + path for AddToIndex
//   - RGBA query buffer or pHash for searches
//
// Outputs:
//   - ThumbnailSearchResult { filePath, similarity, hammingDistance }
//   - IndexStats   { count, avg times, memory estimate }
//   - Persistence  via SaveIndex / LoadIndex (binary format)
//
// Thread Safety:
//   All mutable state is protected by an SRWLOCK.
//
// Build:
//   Header-only, C++20, MSVC /W4 clean, no external dependencies.
//==============================================================================
#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <numeric>
#include <bitset>

namespace ExplorerLens {
namespace Engine {

/// An entry stored in the search index.
struct IndexEntry {
    uint64_t                fileHash = 0;
    std::wstring            filePath;
    uint64_t                perceptualHash = 0;
    std::array<float, 32>   featureVector{};
    uint64_t                indexTime = 0; // ms since epoch
};

/// A search result returned by similarity queries.
struct ThumbnailSearchResult {
    std::wstring filePath;
    float        similarity = 0.0f; // [0, 1]
    uint32_t     hammingDistance = 0;
};

/// Aggregate index statistics.
struct IndexStats {
    uint64_t indexedCount = 0;
    double   totalIndexTimeMs = 0.0;
    double   totalSearchTimeMs = 0.0;
    uint64_t searchCount = 0;
    size_t   estimatedMemBytes = 0;
    double AvgIndexTimeMs()  const { return indexedCount ? totalIndexTimeMs / static_cast<double>(indexedCount) : 0.0; }
    double AvgSearchTimeMs() const { return searchCount ? totalSearchTimeMs / static_cast<double>(searchCount) : 0.0; }
};

/// Searchable thumbnail index using perceptual hashing + feature vectors.
class ThumbnailSearchIndex {
public:
    ThumbnailSearchIndex() {
        InitializeSRWLock(&m_lock);
    }

    // ---- Indexing -----------------------------------------------------------

    /// Add an image to the index (computes pHash + feature vector).
    inline void AddToIndex(const std::wstring& path,
        const uint8_t* rgbaData,
        uint32_t width, uint32_t height) {
        using Clock = std::chrono::high_resolution_clock;
        auto t0 = Clock::now();

        IndexEntry entry;
        entry.filePath = path;
        entry.fileHash = SimplePathHash(path);
        entry.perceptualHash = ComputePHash(rgbaData, width, height);
        entry.featureVector = ComputeFeatureVector(rgbaData, width, height);
        auto now = std::chrono::system_clock::now();
        entry.indexTime = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());

        auto t1 = Clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        AcquireSRWLockExclusive(&m_lock);
        m_entries[path] = std::move(entry);
        m_stats.indexedCount = m_entries.size();
        m_stats.totalIndexTimeMs += ms;
        m_stats.estimatedMemBytes = m_entries.size() * (sizeof(IndexEntry) + 128);
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Remove an entry by path.
    inline void RemoveFromIndex(const std::wstring& path) {
        AcquireSRWLockExclusive(&m_lock);
        m_entries.erase(path);
        m_stats.indexedCount = m_entries.size();
        m_stats.estimatedMemBytes = m_entries.size() * (sizeof(IndexEntry) + 128);
        ReleaseSRWLockExclusive(&m_lock);
    }

    // ---- Searching ---------------------------------------------------------

    /// Find images most similar to the query image.
    inline std::vector<ThumbnailSearchResult> FindSimilar(const uint8_t* queryRGBA,
        uint32_t width, uint32_t height,
        uint32_t maxResults = 10) {
        using Clock = std::chrono::high_resolution_clock;
        auto t0 = Clock::now();

        uint64_t qHash = ComputePHash(queryRGBA, width, height);
        auto qFeatures = ComputeFeatureVector(queryRGBA, width, height);

        std::vector<ThumbnailSearchResult> results;

        AcquireSRWLockShared(&m_lock);
        results.reserve(m_entries.size());
        for (const auto& [path, entry] : m_entries) {
            uint32_t hamming = HammingDistance(qHash, entry.perceptualHash);
            // Quick filter: skip if Hamming distance is too high
            if (hamming > 24) continue;
            float cosine = CosineSimilarity(qFeatures, entry.featureVector);
            // Combined similarity: 40% hash-based + 60% feature-based
            float hashSim = 1.0f - static_cast<float>(hamming) / 64.0f;
            float similarity = 0.4f * hashSim + 0.6f * cosine;
            results.push_back({ path, similarity, hamming });
        }
        ReleaseSRWLockShared(&m_lock);

        // Sort descending by similarity
        std::sort(results.begin(), results.end(),
            [](const ThumbnailSearchResult& a, const ThumbnailSearchResult& b) { return a.similarity > b.similarity; });

        uint32_t n = (std::min)(maxResults, static_cast<uint32_t>(results.size()));
        results.resize(n);

        auto t1 = Clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        AcquireSRWLockExclusive(&m_lock);
        m_stats.searchCount++;
        m_stats.totalSearchTimeMs += ms;
        ReleaseSRWLockExclusive(&m_lock);

        return results;
    }

    /// Find entries within a given Hamming distance of the supplied hash.
    inline std::vector<ThumbnailSearchResult> FindByPerceptualHash(uint64_t hash,
        uint32_t maxHammingDistance = 8) {
        using Clock = std::chrono::high_resolution_clock;
        auto t0 = Clock::now();

        std::vector<ThumbnailSearchResult> results;

        AcquireSRWLockShared(&m_lock);
        for (const auto& [path, entry] : m_entries) {
            uint32_t d = HammingDistance(hash, entry.perceptualHash);
            if (d <= maxHammingDistance) {
                float sim = 1.0f - static_cast<float>(d) / 64.0f;
                results.push_back({ path, sim, d });
            }
        }
        ReleaseSRWLockShared(&m_lock);

        std::sort(results.begin(), results.end(),
            [](const ThumbnailSearchResult& a, const ThumbnailSearchResult& b) { return a.hammingDistance < b.hammingDistance; });

        auto t1 = Clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        AcquireSRWLockExclusive(&m_lock);
        m_stats.searchCount++;
        m_stats.totalSearchTimeMs += ms;
        ReleaseSRWLockExclusive(&m_lock);

        return results;
    }

    // ---- Persistence -------------------------------------------------------

    /// Save index to a binary file.  Format: [count:u64] [entries…]
    inline bool SaveIndex(const std::wstring& filePath) const {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        std::ofstream out(filePath, std::ios::binary);
        if (!out.is_open()) {
            ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
            return false;
        }
        uint64_t count = m_entries.size();
        out.write(reinterpret_cast<const char*>(&count), sizeof(count));
        for (const auto& [path, entry] : m_entries) {
            // Write path length (bytes) + path data (UTF-16)
            uint32_t pathLen = static_cast<uint32_t>(entry.filePath.size());
            out.write(reinterpret_cast<const char*>(&pathLen), sizeof(pathLen));
            out.write(reinterpret_cast<const char*>(entry.filePath.data()),
                pathLen * sizeof(wchar_t));
            out.write(reinterpret_cast<const char*>(&entry.fileHash), sizeof(entry.fileHash));
            out.write(reinterpret_cast<const char*>(&entry.perceptualHash), sizeof(entry.perceptualHash));
            out.write(reinterpret_cast<const char*>(entry.featureVector.data()),
                entry.featureVector.size() * sizeof(float));
            out.write(reinterpret_cast<const char*>(&entry.indexTime), sizeof(entry.indexTime));
        }
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        return out.good();
    }

    /// Load index from a binary file.
    inline bool LoadIndex(const std::wstring& filePath) {
        std::ifstream in(filePath, std::ios::binary);
        if (!in.is_open()) return false;

        uint64_t count = 0;
        in.read(reinterpret_cast<char*>(&count), sizeof(count));
        if (!in.good() || count > 10'000'000) return false;

        std::unordered_map<std::wstring, IndexEntry> loaded;
        loaded.reserve(static_cast<size_t>(count));

        for (uint64_t i = 0; i < count; ++i) {
            IndexEntry entry;
            uint32_t pathLen = 0;
            in.read(reinterpret_cast<char*>(&pathLen), sizeof(pathLen));
            if (!in.good() || pathLen > 32768) return false;
            entry.filePath.resize(pathLen);
            in.read(reinterpret_cast<char*>(entry.filePath.data()), pathLen * sizeof(wchar_t));
            in.read(reinterpret_cast<char*>(&entry.fileHash), sizeof(entry.fileHash));
            in.read(reinterpret_cast<char*>(&entry.perceptualHash), sizeof(entry.perceptualHash));
            in.read(reinterpret_cast<char*>(entry.featureVector.data()),
                entry.featureVector.size() * sizeof(float));
            in.read(reinterpret_cast<char*>(&entry.indexTime), sizeof(entry.indexTime));
            if (!in.good()) return false;
            loaded[entry.filePath] = std::move(entry);
        }

        AcquireSRWLockExclusive(&m_lock);
        m_entries = std::move(loaded);
        m_stats.indexedCount = m_entries.size();
        m_stats.estimatedMemBytes = m_entries.size() * (sizeof(IndexEntry) + 128);
        ReleaseSRWLockExclusive(&m_lock);
        return true;
    }

    /// Retrieve aggregate statistics.
    inline IndexStats GetStats() const {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        IndexStats copy = m_stats;
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_lock));
        return copy;
    }

private:
    // ---- Perceptual hash (pHash) -------------------------------------------

    /// Compute a 64-bit perceptual hash.  Resize to 32x32 gray, apply 8x8
    /// DCT on the top-left quadrant, compare to mean → 64-bit hash.
    static inline uint64_t ComputePHash(const uint8_t* rgba,
        uint32_t width, uint32_t height) {
        if (!rgba || width == 0 || height == 0) return 0;

        // Resize to 32x32 grayscale using nearest-neighbor
        std::array<float, 32 * 32> gray{};
        for (uint32_t y = 0; y < 32; ++y) {
            for (uint32_t x = 0; x < 32; ++x) {
                uint32_t sx = x * width / 32;
                uint32_t sy = y * height / 32;
                sx = (std::min)(sx, width - 1);
                sy = (std::min)(sy, height - 1);
                uint32_t off = (sy * width + sx) * 4;
                gray[y * 32 + x] = 0.299f * rgba[off] + 0.587f * rgba[off + 1] + 0.114f * rgba[off + 2];
            }
        }

        // 8x8 Type-II DCT on the top-left 8x8 block of the 32x32 image
        std::array<float, 64> dctCoeffs{};
        const float PI = 3.14159265358979323846f;
        for (uint32_t u = 0; u < 8; ++u) {
            for (uint32_t v = 0; v < 8; ++v) {
                float sum = 0.0f;
                for (uint32_t y = 0; y < 8; ++y) {
                    for (uint32_t x = 0; x < 8; ++x) {
                        sum += gray[y * 32 + x]
                            * std::cos(PI * (2.0f * x + 1.0f) * u / 16.0f)
                            * std::cos(PI * (2.0f * y + 1.0f) * v / 16.0f);
                    }
                }
                float cu = (u == 0) ? (1.0f / std::sqrt(2.0f)) : 1.0f;
                float cv = (v == 0) ? (1.0f / std::sqrt(2.0f)) : 1.0f;
                dctCoeffs[v * 8 + u] = 0.25f * cu * cv * sum;
            }
        }

        // Exclude DC component (index 0), compute mean of remaining 63 coefficients
        float dctSum = 0.0f;
        for (uint32_t i = 1; i < 64; ++i) dctSum += dctCoeffs[i];
        float dctMean = dctSum / 63.0f;

        // Build 64-bit hash: bit=1 if coeff > mean
        uint64_t hash = 0;
        for (uint32_t i = 0; i < 64; ++i) {
            if (dctCoeffs[i] > dctMean) hash |= (1ULL << i);
        }
        return hash;
    }

    // ---- Feature vector (32-dimensional) -----------------------------------

    /// Compute a 32-float feature vector:
    ///   [0..15]  16-bin normalized color histogram (R averaged with G, B)
    ///   [16..23] 8 edge-orientation histogram bins
    ///   [24..27] 4 GLCM texture features (energy, entropy, contrast, homogeneity)
    ///   [28..31] 4 spatial moments
    static inline std::array<float, 32> ComputeFeatureVector(const uint8_t* rgba,
        uint32_t width,
        uint32_t height) {
        std::array<float, 32> fv{};
        if (!rgba || width == 0 || height == 0) return fv;

        const uint32_t pixelCount = width * height;

        // --- 16-bin color histogram (average luminance-ish) ------------------
        std::array<uint32_t, 16> colorHist{};
        for (uint32_t i = 0; i < pixelCount; ++i) {
            uint32_t off = i * 4;
            uint32_t lum = (static_cast<uint32_t>(rgba[off]) + rgba[off + 1] + rgba[off + 2]) / 3;
            colorHist[(std::min)(lum >> 4, 15u)]++;
        }
        for (uint32_t i = 0; i < 16; ++i) {
            fv[i] = static_cast<float>(colorHist[i]) / static_cast<float>((std::max)(pixelCount, 1u));
        }

        // --- 8-bin edge orientation histogram --------------------------------
        std::array<float, 8> edgeHist{};
        if (width >= 3 && height >= 3) {
            uint32_t edgeTotal = 0;
            for (uint32_t y = 1; y + 1 < height; y += 2) {
                for (uint32_t x = 1; x + 1 < width; x += 2) {
                    auto grayAt = [&](uint32_t px, uint32_t py) -> float {
                        uint32_t off = (py * width + px) * 4;
                        return 0.299f * rgba[off] + 0.587f * rgba[off + 1] + 0.114f * rgba[off + 2];
                        };
                    float gx = -grayAt(x - 1, y - 1) - 2.0f * grayAt(x - 1, y) - grayAt(x - 1, y + 1)
                        + grayAt(x + 1, y - 1) + 2.0f * grayAt(x + 1, y) + grayAt(x + 1, y + 1);
                    float gy = -grayAt(x - 1, y - 1) - 2.0f * grayAt(x, y - 1) - grayAt(x + 1, y - 1)
                        + grayAt(x - 1, y + 1) + 2.0f * grayAt(x, y + 1) + grayAt(x + 1, y + 1);
                    float mag = std::sqrt(gx * gx + gy * gy);
                    if (mag > 10.0f) {
                        float angle = std::atan2(gy, gx); // [-π, π]
                        if (angle < 0.0f) angle += 2.0f * 3.14159265358979323846f;
                        uint32_t bin = static_cast<uint32_t>(angle / (2.0f * 3.14159265358979323846f) * 8.0f);
                        bin = (std::min)(bin, 7u);
                        edgeHist[bin] += mag;
                        ++edgeTotal;
                    }
                }
            }
            if (edgeTotal > 0) {
                float invTot = 1.0f / static_cast<float>(edgeTotal);
                for (uint32_t i = 0; i < 8; ++i) {
                    // Normalize: divide edges by avg magnitude to get ratio
                    fv[16 + i] = edgeHist[i] * invTot / 255.0f;
                    fv[16 + i] = (std::min)(fv[16 + i], 1.0f);
                }
            }
        }

        // --- 4 GLCM texture features (energy, entropy, contrast, homogeneity) ---
        // Use simplified 8-level GLCM at displacement (1,0)
        constexpr uint32_t GLCM_LEVELS = 8;
        std::array<std::array<float, GLCM_LEVELS>, GLCM_LEVELS> glcm{};
        uint32_t glcmPairs = 0;
        for (uint32_t y = 0; y < height; y += 2) {
            for (uint32_t x = 0; x + 1 < width; x += 2) {
                uint32_t off1 = (y * width + x) * 4;
                uint32_t off2 = (y * width + x + 1) * 4;
                uint32_t g1 = (static_cast<uint32_t>(rgba[off1]) + rgba[off1 + 1] + rgba[off1 + 2]) / 3;
                uint32_t g2 = (static_cast<uint32_t>(rgba[off2]) + rgba[off2 + 1] + rgba[off2 + 2]) / 3;
                uint32_t l1 = (std::min)(g1 >> 5, GLCM_LEVELS - 1);
                uint32_t l2 = (std::min)(g2 >> 5, GLCM_LEVELS - 1);
                glcm[l1][l2] += 1.0f;
                glcm[l2][l1] += 1.0f;
                glcmPairs += 2;
            }
        }
        if (glcmPairs > 0) {
            float invPairs = 1.0f / static_cast<float>(glcmPairs);
            float energy = 0.0f, entropy = 0.0f, contrast = 0.0f, homogeneity = 0.0f;
            for (uint32_t i = 0; i < GLCM_LEVELS; ++i) {
                for (uint32_t j = 0; j < GLCM_LEVELS; ++j) {
                    float p = glcm[i][j] * invPairs;
                    energy += p * p;
                    if (p > 1e-8f) entropy -= p * std::log2(p);
                    contrast += static_cast<float>((i > j ? i - j : j - i) * (i > j ? i - j : j - i)) * p;
                    homogeneity += p / (1.0f + static_cast<float>(i > j ? i - j : j - i));
                }
            }
            fv[24] = (std::min)(energy, 1.0f);
            fv[25] = (std::min)(entropy / 6.0f, 1.0f); // max ~log2(64)=6
            fv[26] = (std::min)(contrast / static_cast<float>((GLCM_LEVELS - 1) * (GLCM_LEVELS - 1)), 1.0f);
            fv[27] = (std::min)(homogeneity, 1.0f);
        }

        // --- 4 spatial moments -----------------------------------------------
        // centroid_x, centroid_y, spread_x, spread_y (normalized to [0,1])
        double sumI = 0.0, sumIX = 0.0, sumIY = 0.0;
        for (uint32_t y = 0; y < height; y += 2) {
            for (uint32_t x = 0; x < width; x += 2) {
                uint32_t off = (y * width + x) * 4;
                float intensity = (rgba[off] + rgba[off + 1] + rgba[off + 2]) / 765.0f; // [0,1]
                sumI += intensity;
                sumIX += intensity * x;
                sumIY += intensity * y;
            }
        }
        if (sumI > 1e-6) {
            float centX = static_cast<float>(sumIX / sumI) / static_cast<float>((std::max)(width, 1u));
            float centY = static_cast<float>(sumIY / sumI) / static_cast<float>((std::max)(height, 1u));
            fv[28] = (std::min)((std::max)(centX, 0.0f), 1.0f);
            fv[29] = (std::min)((std::max)(centY, 0.0f), 1.0f);
            // Second moments (spread)
            double sumIXX = 0.0, sumIYY = 0.0;
            for (uint32_t y = 0; y < height; y += 2) {
                for (uint32_t x = 0; x < width; x += 2) {
                    uint32_t off = (y * width + x) * 4;
                    float intensity = (rgba[off] + rgba[off + 1] + rgba[off + 2]) / 765.0f;
                    float dx = static_cast<float>(x) / static_cast<float>(width) - centX;
                    float dy = static_cast<float>(y) / static_cast<float>(height) - centY;
                    sumIXX += intensity * dx * dx;
                    sumIYY += intensity * dy * dy;
                }
            }
            fv[30] = (std::min)(static_cast<float>(std::sqrt(sumIXX / sumI)), 1.0f);
            fv[31] = (std::min)(static_cast<float>(std::sqrt(sumIYY / sumI)), 1.0f);
        }

        return fv;
    }

    // ---- Utility -----------------------------------------------------------

    /// Hamming distance between two 64-bit hashes.
    static inline uint32_t HammingDistance(uint64_t a, uint64_t b) {
        uint64_t x = a ^ b;
        uint32_t count = 0;
        while (x) { count += static_cast<uint32_t>(x & 1ULL); x >>= 1; }
        return count;
    }

    /// Cosine similarity between two 32-dimensional vectors.
    static inline float CosineSimilarity(const std::array<float, 32>& a,
        const std::array<float, 32>& b) {
        float dot = 0.0f, magA = 0.0f, magB = 0.0f;
        for (uint32_t i = 0; i < 32; ++i) {
            dot += a[i] * b[i];
            magA += a[i] * a[i];
            magB += b[i] * b[i];
        }
        float denom = std::sqrt(magA) * std::sqrt(magB);
        if (denom < 1e-8f) return 0.0f;
        return (std::max)(0.0f, dot / denom);
    }

    /// Simple hash of a file path (FNV-1a on wchar_t data).
    static inline uint64_t SimplePathHash(const std::wstring& path) {
        uint64_t hash = 14695981039346656037ULL;
        for (wchar_t ch : path) {
            hash ^= static_cast<uint64_t>(ch);
            hash *= 1099511628211ULL;
        }
        return hash;
    }

    mutable SRWLOCK                                    m_lock{};
    std::unordered_map<std::wstring, IndexEntry>       m_entries;
    IndexStats                                         m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
