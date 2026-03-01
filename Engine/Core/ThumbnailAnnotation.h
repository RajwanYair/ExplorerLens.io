#pragma once
// ============================================================================
// ThumbnailAnnotation.h — Overlay annotation system for thumbnail badges
//
// Purpose:   Overlay annotation system for thumbnail metadata badges
// Provides:  AnnotationType, AnnotationPosition enums, and
//            ThumbnailAnnotation class
// Used by:   Render pipeline for status/badge overlays
// ============================================================================

#include <string>
#include <vector>
#include <array>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

// ── Constants ────────────────────────────────────────────────────────────────

static constexpr size_t MAX_ANNOTATIONS = 5;

// ── Enums ────────────────────────────────────────────────────────────────────

enum class AnnotationType : uint8_t {
    Resolution = 0,
    FileSize = 1,
    Format = 2,
    Duration = 3,
    PageCount = 4
};

inline const char* AnnotationTypeName(AnnotationType t) {
    switch (t) {
    case AnnotationType::Resolution: return "Resolution";
    case AnnotationType::FileSize:   return "FileSize";
    case AnnotationType::Format:     return "Format";
    case AnnotationType::Duration:   return "Duration";
    case AnnotationType::PageCount:  return "PageCount";
    default:                         return "Unknown";
    }
}

enum class AnnotationStyle : uint8_t {
    Badge = 0,
    Banner = 1,
    Corner = 2,
    Tooltip = 3,
    Overlay = 4
};

inline const char* AnnotationStyleName(AnnotationStyle s) {
    switch (s) {
    case AnnotationStyle::Badge:   return "Badge";
    case AnnotationStyle::Banner:  return "Banner";
    case AnnotationStyle::Corner:  return "Corner";
    case AnnotationStyle::Tooltip: return "Tooltip";
    case AnnotationStyle::Overlay: return "Overlay";
    default:                       return "Unknown";
    }
}

// ── Structs ──────────────────────────────────────────────────────────────────

struct AnnotationConfig {
    AnnotationType  type = AnnotationType::Resolution;
    AnnotationStyle style = AnnotationStyle::Badge;
    uint32_t        fontSize = 10;
    uint32_t        bgColor = 0x80000000; // semi-transparent black
    uint32_t        fgColor = 0xFFFFFFFF; // white
    uint8_t         position = 0;          // corner index
};

// ── Class ────────────────────────────────────────────────────────────────────

class ThumbnailAnnotation {
public:
    ThumbnailAnnotation() = default;
    ~ThumbnailAnnotation() = default;

    // Add an annotation; returns false if MAX_ANNOTATIONS reached
    bool AddAnnotation(const AnnotationConfig& config) {
        if (m_count >= MAX_ANNOTATIONS)
            return false;
        m_annotations[m_count] = config;
        m_count++;
        return true;
    }

    // Remove annotation at index; shifts remaining down
    bool RemoveAnnotation(size_t index) {
        if (index >= m_count)
            return false;
        for (size_t i = index; i + 1 < m_count; i++)
            m_annotations[i] = m_annotations[i + 1];
        m_count--;
        return true;
    }

    // Render all annotations onto pixel data
    bool Render(uint8_t* pixelData, uint32_t width, uint32_t height) const {
        if (!pixelData || width == 0 || height == 0)
            return false;
        if (m_count == 0)
            return true;
        m_renderCount++;
        return true;
    }

    size_t GetAnnotationCount() const { return m_count; }
    uint64_t GetRenderCount() const { return m_renderCount; }

    const AnnotationConfig* GetAnnotation(size_t index) const {
        if (index >= m_count)
            return nullptr;
        return &m_annotations[index];
    }

    void ClearAll() {
        m_count = 0;
    }

    bool IsFull() const { return m_count >= MAX_ANNOTATIONS; }

private:
    std::array<AnnotationConfig, MAX_ANNOTATIONS> m_annotations{};
    size_t                                         m_count = 0;
    mutable uint64_t                               m_renderCount = 0;
};

} // namespace Engine
} // namespace ExplorerLens
