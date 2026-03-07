// ThumbnailComplexityRouter.h — Complexity-Based Decode Routing
// Copyright (c) 2026 ExplorerLens Project
//
// Estimates image complexity from file metadata and routes to the
// appropriate decoder tier (fast/standard/quality) based on complexity.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class ComplexityTier : uint8_t {
    Trivial,
    Simple,
    Moderate,
    Complex,
    Extreme
};

enum class DecodeTier : uint8_t {
    FastPath,
    StandardPath,
    QualityPath,
    PremiumPath
};

struct ThumbnailComplexityEstimate {
    ComplexityTier tier = ComplexityTier::Moderate;
    float score = 0.5f;
    uint64_t fileSize = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t colorDepth = 8;
    bool hasAlpha = false;
    bool hasAnimation = false;
    bool isMultiPage = false;
};

struct ComplexityRoutingDecision {
    DecodeTier selectedTier = DecodeTier::StandardPath;
    ThumbnailComplexityEstimate estimate;
    uint32_t maxConcurrentAllowed = 4;
    uint32_t estimatedMemoryMB = 0;
    double estimatedDecodeTimeMs = 0.0;
};

class ThumbnailComplexityRouter {
public:
    ThumbnailComplexityRouter() = default;

    ThumbnailComplexityEstimate EstimateComplexity(uint64_t fileSize, uint32_t width,
        uint32_t height, uint32_t colorDepth = 8) const {
        ThumbnailComplexityEstimate estimate;
        estimate.fileSize = fileSize;
        estimate.width = width;
        estimate.height = height;
        estimate.colorDepth = colorDepth;

        uint64_t pixels = static_cast<uint64_t>(width) * height;
        float fileSizeScore = fileSize > 10 * 1024 * 1024 ? 1.0f :
            static_cast<float>(fileSize) / (10.0f * 1024 * 1024);
        float pixelScore = pixels > 100000000ULL ? 1.0f :
            static_cast<float>(pixels) / 100000000.0f;
        estimate.score = (fileSizeScore + pixelScore) / 2.0f;

        if (estimate.score < 0.1f) estimate.tier = ComplexityTier::Trivial;
        else if (estimate.score < 0.3f) estimate.tier = ComplexityTier::Simple;
        else if (estimate.score < 0.6f) estimate.tier = ComplexityTier::Moderate;
        else if (estimate.score < 0.85f) estimate.tier = ComplexityTier::Complex;
        else estimate.tier = ComplexityTier::Extreme;

        return estimate;
    }

    ComplexityRoutingDecision Route(const ThumbnailComplexityEstimate& estimate) const {
        ComplexityRoutingDecision decision;
        decision.estimate = estimate;
        switch (estimate.tier) {
        case ComplexityTier::Trivial:
        case ComplexityTier::Simple:
            decision.selectedTier = DecodeTier::FastPath;
            decision.maxConcurrentAllowed = 8;
            break;
        case ComplexityTier::Moderate:
            decision.selectedTier = DecodeTier::StandardPath;
            decision.maxConcurrentAllowed = 4;
            break;
        case ComplexityTier::Complex:
            decision.selectedTier = DecodeTier::QualityPath;
            decision.maxConcurrentAllowed = 2;
            break;
        case ComplexityTier::Extreme:
            decision.selectedTier = DecodeTier::PremiumPath;
            decision.maxConcurrentAllowed = 1;
            break;
        }
        decision.estimatedMemoryMB = static_cast<uint32_t>(estimate.fileSize / (1024 * 1024)) + 1;
        return decision;
    }

    uint64_t GetTotalRouted() const { return m_totalRouted; }

private:
    uint64_t m_totalRouted = 0;
};

} // namespace Engine
} // namespace ExplorerLens
