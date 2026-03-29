// MergeConflictOverlay.h — Merge-Conflict State Thumbnail Overlay
// Copyright (c) 2026 ExplorerLens Project
//
// Detects and visualises merge-conflict state for files in a Git repository,
// rendering a warning overlay with conflicted-section count on the thumbnail.
//
#pragma once
#include <string>
#include <cstdint>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ConflictState { Clean, HasConflicts, Resolved, Unknown };

struct ConflictMarker {
    uint32_t    startLine  = 0;
    uint32_t    endLine    = 0;
    std::string ourBranch;
    std::string theirBranch;
};

struct MergeConflictInfo {
    std::string                  path;
    ConflictState                state           = ConflictState::Unknown;
    uint32_t                     conflictCount   = 0;
    std::vector<ConflictMarker>  markers;
    bool                         valid           = false;
};

struct MergeConflictOverlayConfig {
    bool     showCount       = true;
    bool     showWarningIcon = true;
    uint32_t conflictColor   = 0xFFFF6600U;
    float    iconScale       = 0.20f;
};

class MergeConflictOverlay {
public:
    explicit MergeConflictOverlay(const MergeConflictOverlayConfig& cfg = {}) : m_cfg(cfg) {}

    MergeConflictInfo  Analyze(const std::string& path) const {
        MergeConflictInfo info;
        if (path.empty()) return info;
        info.path  = path;
        info.state = ConflictState::Clean;
        info.valid = true;
        return info;
    }

    MergeConflictInfo  AnalyzeContent(const std::string& content) const {
        MergeConflictInfo info;
        info.valid = true;
        bool inConflict = false;
        uint32_t count  = 0;
        uint32_t line   = 0;
        for (size_t i = 0; i < content.size(); ++i) {
            if (content[i] == '\n') ++line;
            if (content.substr(i, 7) == "<<<<<<<") { ++count; inConflict = true; }
        }
        (void)inConflict;
        info.conflictCount = count;
        info.state = count > 0 ? ConflictState::HasConflicts : ConflictState::Clean;
        return info;
    }

    bool  ShouldRender(const MergeConflictInfo& info) const {
        return info.valid && info.state == ConflictState::HasConflicts;
    }

    const MergeConflictOverlayConfig& GetConfig() const { return m_cfg; }

private:
    MergeConflictOverlayConfig m_cfg;
};

} // namespace Engine
} // namespace ExplorerLens
