//==============================================================================
// ExplorerLens Engine — AI Search Integration
// Visual similarity search via embedding vectors, Windows Search integration,
// semantic image retrieval, and cross-format duplicate detection via CLIP.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class AISearchMode : uint8_t {
    VisualSimilarity = 0,
    SemanticText,
    HybridVT,
    Duplicate,
    SemanticSimilarity = SemanticText, // compat alias
    COUNT = Duplicate + 1
};
enum class EmbeddingModel : uint8_t {
    CLIP_ViT_B32 = 0,
    CLIP_ViT_L14,
    DINOv2,
    CLIP = CLIP_ViT_B32, // compat alias
    COUNT = DINOv2 + 1
};
enum class SearchIndexStatus : uint8_t {
    NotIndexed = 0,
    Indexing,
    Indexed,
    Stale,
    Ready = Indexed, // compat alias
    COUNT = Stale + 1
};

struct EmbeddingVector {
    std::vector<float> values;
    EmbeddingModel model = EmbeddingModel::CLIP_ViT_B32;
    uint32_t dims = 512;
};

struct AISearchResult {
    std::wstring filePath;
    float similarity = 0.0f; // 0-1
    AISearchMode matchMode = AISearchMode::VisualSimilarity;
    bool isDuplicate = false;
};

class AISearchIntegration {
public:
    static const wchar_t* SearchModeName(AISearchMode m) {
        switch (m) {
        case AISearchMode::VisualSimilarity:
            return L"Visual Similarity";
        case AISearchMode::SemanticText:
            return L"Semantic Text";
        case AISearchMode::HybridVT:
            return L"Hybrid V+T";
        case AISearchMode::Duplicate:
            return L"Duplicate Detection";
        default:
            return L"Unknown";
        }
    }
    static const wchar_t* ModelName(EmbeddingModel m) {
        switch (m) {
        case EmbeddingModel::CLIP_ViT_B32:
            return L"CLIP ViT-B/32";
        case EmbeddingModel::CLIP_ViT_L14:
            return L"CLIP ViT-L/14";
        case EmbeddingModel::DINOv2:
            return L"DINOv2";
        default:
            return L"Unknown";
        }
    }
    static const wchar_t* IndexStatusName(SearchIndexStatus s) {
        switch (s) {
        case SearchIndexStatus::NotIndexed:
            return L"Not Indexed";
        case SearchIndexStatus::Indexing:
            return L"Indexing";
        case SearchIndexStatus::Indexed:
            return L"Indexed";
        case SearchIndexStatus::Stale:
            return L"Stale";
        default:
            return L"Unknown";
        }
    }
    static constexpr size_t SearchModeCount() {
        return static_cast<size_t>(AISearchMode::COUNT);
    }
    static constexpr size_t ModelCount() {
        return static_cast<size_t>(EmbeddingModel::COUNT);
    }
    static constexpr size_t IndexStatusCount() {
        return static_cast<size_t>(SearchIndexStatus::COUNT);
    }

    // Compatibility aliases (tests)
    static const wchar_t* ModeName(AISearchMode m) { return SearchModeName(m); }
    static const wchar_t* EmbeddingModelName(EmbeddingModel m) {
        return ModelName(m);
    }
    static constexpr size_t ModeCount() { return SearchModeCount(); }
    static constexpr size_t EmbeddingModelCount() { return ModelCount(); }

    //==========================================================================
    // Perceptual Hashing — CPU-based, no external dependencies
    //==========================================================================

    /// Compute average hash (aHash) for an 8-bit grayscale image.
    /// Returns 64-bit hash. Input: grayscale pixel buffer, width, height, stride.
    static uint64_t ComputeAverageHash(const uint8_t* gray, uint32_t width,
        uint32_t height, uint32_t stride) {
        if (!gray || width < 8 || height < 8) return 0;
        // Downscale to 8x8 using nearest-neighbor
        uint8_t block[64] = {};
        for (uint32_t y = 0; y < 8; ++y) {
            uint32_t srcY = y * height / 8;
            for (uint32_t x = 0; x < 8; ++x) {
                uint32_t srcX = x * width / 8;
                block[y * 8 + x] = gray[srcY * stride + srcX];
            }
        }
        // Compute mean
        uint32_t sum = 0;
        for (int i = 0; i < 64; ++i) sum += block[i];
        uint8_t mean = static_cast<uint8_t>(sum / 64);
        // Generate hash: set bit if pixel > mean
        uint64_t hash = 0;
        for (int i = 0; i < 64; ++i) {
            if (block[i] > mean) hash |= (1ULL << i);
        }
        return hash;
    }

    /// Compute difference hash (dHash) for an 8-bit grayscale image.
    /// Returns 64-bit hash. Compares adjacent pixels in 9x8 downscaled grid.
    static uint64_t ComputeDifferenceHash(const uint8_t* gray, uint32_t width,
        uint32_t height, uint32_t stride) {
        if (!gray || width < 9 || height < 8) return 0;
        // Downscale to 9x8 using nearest-neighbor
        uint8_t block[72] = {};
        for (uint32_t y = 0; y < 8; ++y) {
            uint32_t srcY = y * height / 8;
            for (uint32_t x = 0; x < 9; ++x) {
                uint32_t srcX = x * width / 9;
                block[y * 9 + x] = gray[srcY * stride + srcX];
            }
        }
        // Hash: bit set if left pixel > right pixel
        uint64_t hash = 0;
        int bit = 0;
        for (uint32_t y = 0; y < 8; ++y) {
            for (uint32_t x = 0; x < 8; ++x) {
                if (block[y * 9 + x] > block[y * 9 + x + 1])
                    hash |= (1ULL << bit);
                ++bit;
            }
        }
        return hash;
    }

    /// Compute Hamming distance between two perceptual hashes.
    /// 0 = identical, 64 = maximally different.
    static uint32_t HammingDistance(uint64_t hash1, uint64_t hash2) {
        uint64_t diff = hash1 ^ hash2;
        uint32_t count = 0;
        while (diff) { count += diff & 1; diff >>= 1; }
        return count;
    }

    /// Check if two images are perceptually similar (Hamming distance <= threshold).
    /// Default threshold of 10 catches near-duplicates with different compression.
    static bool AreSimilar(uint64_t hash1, uint64_t hash2, uint32_t threshold = 10) {
        return HammingDistance(hash1, hash2) <= threshold;
    }
};

} // namespace Engine
} // namespace ExplorerLens
