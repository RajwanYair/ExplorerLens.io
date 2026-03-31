// SmartThumbnailCompositor.h — Multi-Layer Thumbnail Compositor
// Copyright (c) 2026 ExplorerLens Project
//
// Composites thumbnails from multi-page documents, layered images, and archive
// previews. Selects representative pages/layers and arranges them in a grid or
// overlay layout. Singleton with Initialize/Shutdown lifecycle.
//
#pragma once

#include <algorithm>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class CompositeLayout : uint8_t {
    Single,
    Grid2x2,
    Grid3x3,
    StackedOverlay,
    FanSpread,
    Filmstrip
};

enum class LayerSelectionStrategy : uint8_t {
    FirstPage,
    LargestLayer,
    MostColorful,
    EvenlySpaced,
    CoverAndSample
};

struct CompositeLayer {
    uint32_t pageIndex = 0;
    uint32_t layerIndex = 0;
    float weight = 1.0f;
    bool isSelected = false;
};

struct CompositeRequest {
    std::wstring filePath;
    uint32_t totalPages = 1;
    uint32_t totalLayers = 1;
    CompositeLayout layout = CompositeLayout::Single;
    LayerSelectionStrategy strategy = LayerSelectionStrategy::FirstPage;
    uint32_t maxLayersToComposite = 4;
};

struct CompositeResult {
    bool success = false;
    CompositeLayout appliedLayout = CompositeLayout::Single;
    uint32_t layersUsed = 0;
    std::vector<CompositeLayer> selectedLayers;
    float compositeTimeMs = 0.0f;
};

struct CompositorStats {
    uint64_t totalComposites = 0;
    uint64_t singlePageResults = 0;
    uint64_t multiLayerResults = 0;
    uint64_t failedComposites = 0;
    bool initialized = false;
};

class SmartThumbnailCompositor {
public:
    static SmartThumbnailCompositor& Instance() {
        static SmartThumbnailCompositor instance;
        return instance;
    }

    void Initialize(uint32_t maxLayers = 4) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_maxLayers = maxLayers;
        m_stats = {};
        m_stats.initialized = true;
    }

    CompositeResult Composite(const CompositeRequest& request) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.totalComposites++;

        CompositeResult result;
        if (request.totalPages <= 1 && request.totalLayers <= 1) {
            result.success = true;
            result.appliedLayout = CompositeLayout::Single;
            result.layersUsed = 1;
            result.selectedLayers.push_back({0, 0, 1.0f, true});
            m_stats.singlePageResults++;
            return result;
        }

        uint32_t count = (std::min)(request.maxLayersToComposite, m_maxLayers);
        uint32_t totalAvailable = (std::max)(request.totalPages, request.totalLayers);
        count = (std::min)(count, totalAvailable);

        result.selectedLayers.reserve(count);
        for (uint32_t i = 0; i < count; ++i) {
            uint32_t idx = 0;
            if (request.strategy == LayerSelectionStrategy::EvenlySpaced && totalAvailable > 1) {
                idx = i * (totalAvailable - 1) / (count - 1);
            } else {
                idx = i;
            }
            result.selectedLayers.push_back({idx, 0, 1.0f / count, true});
        }

        if (count <= 1) result.appliedLayout = CompositeLayout::Single;
        else if (count <= 4) result.appliedLayout = CompositeLayout::Grid2x2;
        else result.appliedLayout = CompositeLayout::Grid3x3;

        result.success = true;
        result.layersUsed = count;
        m_stats.multiLayerResults++;
        return result;
    }

    bool IsInitialized() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats.initialized;
    }

    CompositorStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    void Shutdown() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.initialized = false;
    }

private:
    SmartThumbnailCompositor() = default;
    ~SmartThumbnailCompositor() = default;
    SmartThumbnailCompositor(const SmartThumbnailCompositor&) = delete;
    SmartThumbnailCompositor& operator=(const SmartThumbnailCompositor&) = delete;

    mutable std::mutex m_mutex;
    uint32_t m_maxLayers = 4;
    CompositorStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
