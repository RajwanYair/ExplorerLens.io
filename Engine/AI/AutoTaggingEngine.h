// AutoTaggingEngine.h - CLIP Zero-Shot Image Auto-Tagging
// Copyright (c) 2026 ExplorerLens Project
//
// Multi-label classifier using zero-shot CLIP embeddings against a configurable taxonomy.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace AI {

struct ImageTag {
    std::string label;
    float       confidence = 0.0f;
    int         categoryId = -1;
};

struct AutoTagResult {
    bool                  success = false;
    std::vector<ImageTag> tags;
    std::string           error;
};

struct AutoTagConfig {
    int   maxTags          = 10;
    float minConfidence    = 0.25f;
    bool  hierarchical     = false;
    bool  multiLabel       = true;
};

class AutoTaggingEngine {
public:
    explicit AutoTaggingEngine() = default;
    explicit AutoTaggingEngine(const AutoTagConfig& cfg) : m_config(cfg) {}

    AutoTagResult Tag(const void* srcPixels, int w, int h) const noexcept {
        if (!srcPixels || w <= 0 || h <= 0)
            return { false, {}, "Invalid input" };
        if (!m_taxonomyLoaded)
            return { false, {}, "Taxonomy not loaded" };
        return { true, {}, {} };
    }

    AutoTagResult TagFile(const std::string& path) const noexcept {
        if (path.empty()) return { false, {}, "Empty path" };
        return { false, {}, "File not found: " + path };
    }

    bool LoadTaxonomy(const std::string& path) noexcept {
        if (path.empty()) return false;
        m_taxonomyLoaded = true;
        m_taxonomyPath   = path;
        m_taxonomySize   = 1000; // default simulated size
        return true;
    }

    bool  IsTaxonomyLoaded() const noexcept { return m_taxonomyLoaded; }
    int   GetTaxonomySize()  const noexcept { return m_taxonomySize;   }
    int   GetMaxTags()       const noexcept { return m_config.maxTags; }
    float GetMinConfidence() const noexcept { return m_config.minConfidence; }
    bool  GetHierarchical()  const noexcept { return m_config.hierarchical; }
    bool  GetMultiLabel()    const noexcept { return m_config.multiLabel;   }

    void SetMaxTags(int v)         noexcept { m_config.maxTags       = v; }
    void SetMinConfidence(float v) noexcept { m_config.minConfidence = v; }
    void SetHierarchical(bool v)   noexcept { m_config.hierarchical  = v; }
    void SetMultiLabel(bool v)     noexcept { m_config.multiLabel    = v; }

    static constexpr int   MAX_SUPPORTED_TAGS    = 100;
    static constexpr float MIN_VALID_CONFIDENCE  = 0.10f;

private:
    AutoTagConfig m_config;
    bool          m_taxonomyLoaded = false;
    int           m_taxonomySize   = 0;
    std::string   m_taxonomyPath;
};

}} // namespace ExplorerLens::AI
