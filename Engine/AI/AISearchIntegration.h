// AISearchIntegration.h — AI-Powered Thumbnail Search Integration
// Copyright (c) 2026 ExplorerLens Project
//
// Semantic similarity search, embedding model selection, and search index
// status tracking for AI-accelerated thumbnail retrieval.
//
#pragma once
#include <cstddef>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

enum class AISearchMode : uint8_t {
    SemanticSimilarity,
    Keywords,
    Visual,
    Hybrid,
    COUNT = 4
};

enum class EmbeddingModel : uint8_t {
    CLIP,
    ResNet,
    BLIP,
    Sentence,
    COUNT = 4
};

enum class SearchIndexStatus : uint8_t {
    Building,
    Ready,
    Stale,
    Error,
    COUNT = 4
};

class AISearchIntegration {
public:
    static const wchar_t *ModeName(AISearchMode m) noexcept {
        switch (m) {
        case AISearchMode::SemanticSimilarity: return L"Semantic Similarity";
        case AISearchMode::Keywords:           return L"Keywords";
        case AISearchMode::Visual:             return L"Visual";
        case AISearchMode::Hybrid:             return L"Hybrid";
        default: return L"Unknown";
        }
    }
    static const wchar_t *EmbeddingModelName(EmbeddingModel m) noexcept {
        switch (m) {
        case EmbeddingModel::CLIP:     return L"CLIP";
        case EmbeddingModel::ResNet:   return L"ResNet";
        case EmbeddingModel::BLIP:     return L"BLIP";
        case EmbeddingModel::Sentence: return L"Sentence";
        default: return L"Unknown";
        }
    }
    static const wchar_t *IndexStatusName(SearchIndexStatus s) noexcept {
        switch (s) {
        case SearchIndexStatus::Building: return L"Building";
        case SearchIndexStatus::Ready:    return L"Ready";
        case SearchIndexStatus::Stale:    return L"Stale";
        case SearchIndexStatus::Error:    return L"Error";
        default: return L"Unknown";
        }
    }
    static size_t ModeCount() noexcept {
        return static_cast<size_t>(AISearchMode::COUNT);
    }
    static size_t EmbeddingModelCount() noexcept {
        return static_cast<size_t>(EmbeddingModel::COUNT);
    }

    // Average hash: compare each pixel to the mean; returns 64-bit hash
    static uint64_t ComputeAverageHash(
        const uint8_t* data, uint32_t W, uint32_t H, uint32_t stride) noexcept {
        if (!data || W == 0 || H == 0) return 0;
        uint64_t sum = 0;
        for (uint32_t y = 0; y < H; ++y)
            for (uint32_t x = 0; x < W; ++x)
                sum += data[y * stride + x];
        uint64_t mean = sum / (static_cast<uint64_t>(W) * H);
        uint64_t hash = 0;
        uint32_t bits = 0;
        for (uint32_t y = 0; y < H && bits < 64; ++y)
            for (uint32_t x = 0; x < W && bits < 64; ++x)
                hash |= (static_cast<uint64_t>(data[y * stride + x] > mean) << bits++);
        return hash;
    }

    // Difference hash: compare adjacent horizontal pixels; returns 64-bit hash
    static uint64_t ComputeDifferenceHash(
        const uint8_t* data, uint32_t W, uint32_t H, uint32_t stride) noexcept {
        if (!data || W < 2 || H == 0) return 0;
        uint64_t hash = 0;
        uint32_t bits = 0;
        for (uint32_t y = 0; y < H && bits < 64; ++y)
            for (uint32_t x = 0; x + 1 < W && bits < 64; ++x)
                hash |= (static_cast<uint64_t>(
                    data[y * stride + x] > data[y * stride + x + 1]) << bits++);
        return hash;
    }

    // Hamming distance: count differing bits between two 64-bit hashes
    static uint32_t HammingDistance(uint64_t a, uint64_t b) noexcept {
        uint64_t diff = a ^ b;
        uint32_t count = 0;
        while (diff) { count += static_cast<uint32_t>(diff & 1u); diff >>= 1; }
        return count;
    }

    // Similarity check: hashes are similar if Hamming distance <= threshold
    static bool AreSimilar(
        uint64_t a, uint64_t b, uint32_t threshold = 10) noexcept {
        return HammingDistance(a, b) <= threshold;
    }
};

} // namespace Engine
} // namespace ExplorerLens
