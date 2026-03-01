#pragma once
// ============================================================================
// BatchRenamePreview.h — Dry-run rename preview without modifying files
//
// Purpose:   Dry-run rename preview without modifying files
// Provides:  RenamePattern, RenamePreviewResult, and BatchRenamePreview class
// Used by:   Shell context menu extensions
// ============================================================================

#include <string>
#include <vector>
#include <cstdint>
#include <regex>

namespace ExplorerLens {
namespace Engine {

// ── Enums ────────────────────────────────────────────────────────────────────

enum class RenamePattern : uint8_t {
    Sequential = 0,
    DateTime = 1,
    RegexReplace = 2,
    PrefixSuffix = 3,
    Custom = 4
};

inline const char* RenamePatternName(RenamePattern p) {
    switch (p) {
    case RenamePattern::Sequential:   return "Sequential";
    case RenamePattern::DateTime:     return "DateTime";
    case RenamePattern::RegexReplace: return "RegexReplace";
    case RenamePattern::PrefixSuffix: return "PrefixSuffix";
    case RenamePattern::Custom:       return "Custom";
    default:                          return "Unknown";
    }
}

enum class RenamePreviewState : uint8_t {
    Idle = 0,
    Generating = 1,
    Ready = 2,
    Stale = 3,
    Error = 4
};

inline const char* RenamePreviewStateName(RenamePreviewState s) {
    switch (s) {
    case RenamePreviewState::Idle:       return "Idle";
    case RenamePreviewState::Generating: return "Generating";
    case RenamePreviewState::Ready:      return "Ready";
    case RenamePreviewState::Stale:      return "Stale";
    case RenamePreviewState::Error:      return "Error";
    default:                             return "Unknown";
    }
}

// ── Structs ──────────────────────────────────────────────────────────────────

struct RenamePreviewItem {
    std::string        originalName;
    std::string        newName;
    std::vector<uint8_t> previewThumbnail;
    RenamePreviewState state = RenamePreviewState::Idle;
};

// ── Class ────────────────────────────────────────────────────────────────────

class BatchRenamePreview {
public:
    BatchRenamePreview() = default;
    ~BatchRenamePreview() = default;

    // Generate previews for a list of files using a given pattern
    bool GeneratePreviews(const std::vector<std::string>& filePaths,
        RenamePattern pattern,
        const std::string& patternArg = "") {
        if (filePaths.empty())
            return false;

        m_state = RenamePreviewState::Generating;
        m_items.clear();
        m_items.reserve(filePaths.size());

        uint32_t index = 1;
        for (const auto& path : filePaths) {
            RenamePreviewItem item;
            item.originalName = path;
            item.newName = ApplyPattern(path, pattern, patternArg, index);
            item.state = RenamePreviewState::Ready;
            m_items.push_back(std::move(item));
            index++;
        }
        m_pattern = pattern;
        m_state = RenamePreviewState::Ready;
        return true;
    }

    const RenamePreviewItem* GetPreview(size_t index) const {
        if (index >= m_items.size())
            return nullptr;
        return &m_items[index];
    }

    size_t GetItemCount() const { return m_items.size(); }
    RenamePreviewState GetState() const { return m_state; }
    RenamePattern GetPattern() const { return m_pattern; }

    void Clear() {
        m_items.clear();
        m_state = RenamePreviewState::Idle;
    }

private:
    std::string ApplyPattern(const std::string& path,
        RenamePattern pattern,
        const std::string& arg,
        uint32_t index) const {
        switch (pattern) {
        case RenamePattern::Sequential:
            return "file_" + std::to_string(index);
        case RenamePattern::PrefixSuffix:
            return arg + path;
        case RenamePattern::DateTime:
            return "20260301_" + std::to_string(index);
        case RenamePattern::RegexReplace: {
            // arg format: "pattern|replacement" — applies std::regex_replace on the filename
            auto sep = arg.find('|');
            if (sep == std::string::npos || sep == 0)
                return path; // No valid pattern|replacement pair provided
            try {
                std::regex re(arg.substr(0, sep));
                return std::regex_replace(path, re, arg.substr(sep + 1));
            }
            catch (const std::regex_error&) {
                return path; // Invalid regex — return original path unchanged
            }
        }
        case RenamePattern::Custom:
            return arg.empty() ? path : arg + "_" + std::to_string(index);
        default:
            return path;
        }
    }

    std::vector<RenamePreviewItem> m_items;
    RenamePreviewState             m_state = RenamePreviewState::Idle;
    RenamePattern                  m_pattern = RenamePattern::Sequential;
};

} // namespace Engine
} // namespace ExplorerLens
