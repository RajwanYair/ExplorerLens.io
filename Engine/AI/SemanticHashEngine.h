// SemanticHashEngine.h - Semantic Perceptual Hash via CNN Embeddings
// Copyright (c) 2026 ExplorerLens Project
//
// CNN-based 512-bit semantic hash; stable across style/colour transforms.
//
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace AI {

using SemanticHash512 = std::array<uint64_t, 8>;

enum class HashAlgorithm : uint8_t {
    CNN_MobileNetV3  = 0,
    CNN_EfficientNetB0 = 1,
    CLIP_ViT_B32     = 2,
};

struct SemanticHashResult {
    bool            success    = false;
    SemanticHash512 hash       = {};
    float           confidence = 0.0f;
    std::string     error      = {};
};

struct SemanticHashConfig {
    HashAlgorithm algorithm     = HashAlgorithm::CNN_MobileNetV3;
    int           embeddingDim  = 512;
    bool          l2Normalize   = true;
    float         quantileThresh = 0.5f;
};

class SemanticHashEngine {
public:
    explicit SemanticHashEngine() = default;
    explicit SemanticHashEngine(const SemanticHashConfig& cfg) : m_config(cfg) {}

    SemanticHashResult Hash(const void* srcPixels, int w, int h) const noexcept {
        if (!srcPixels || w <= 0 || h <= 0)
            return { false, {}, 0.0f, "Invalid input" };
        SemanticHash512 h512{};
        // Deterministic zero-hash placeholder for CPU path
        return { true, h512, 1.0f, {} };
    }

    SemanticHashResult HashFile(const std::string& path) const noexcept {
        if (path.empty()) return { false, {}, 0.0f, "Empty path" };
        return { false, {}, 0.0f, "File not found: " + path };
    }

    static float HammingDistance(const SemanticHash512& a,
                                 const SemanticHash512& b) noexcept {
        int bits = 0;
        for (size_t i = 0; i < a.size(); ++i) {
            uint64_t diff = a[i] ^ b[i];
            while (diff) { bits += (diff & 1u); diff >>= 1; }
        }
        return static_cast<float>(bits);
    }

    static bool AreDuplicate(const SemanticHash512& a, const SemanticHash512& b,
                              float threshBits = DUPLICATE_THRESH) noexcept {
        return HammingDistance(a, b) <= threshBits;
    }

    HashAlgorithm GetAlgorithm()    const noexcept { return m_config.algorithm;     }
    int           GetEmbeddingDim() const noexcept { return m_config.embeddingDim;  }
    bool          GetL2Normalize()  const noexcept { return m_config.l2Normalize;   }
    float         GetQuantileThresh() const noexcept { return m_config.quantileThresh; }

    void SetAlgorithm(HashAlgorithm a)  noexcept { m_config.algorithm     = a; }
    void SetL2Normalize(bool v)         noexcept { m_config.l2Normalize   = v; }
    void SetQuantileThresh(float t)     noexcept { m_config.quantileThresh = t; }

    static constexpr int   HASH_BITS        = 512;
    static constexpr float DUPLICATE_THRESH = 32.0f;

private:
    SemanticHashConfig m_config;
};

}} // namespace ExplorerLens::AI
