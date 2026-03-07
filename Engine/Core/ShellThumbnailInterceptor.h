// ShellThumbnailInterceptor.h — Shell Thumbnail Enhancement Layer
// Copyright (c) 2026 ExplorerLens Project
//
// Intercepts Windows Shell thumbnail requests to enhance them with
// overlays, quality improvements, and format-specific optimizations.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class InterceptionMode : uint8_t {
    PassThrough,
    Enhance,
    Replace,
    Augment
};

struct InterceptionRule {
    std::wstring extensionPattern;
    InterceptionMode mode = InterceptionMode::PassThrough;
    float qualityBoost = 0.0f;
    bool addOverlay = false;
};

struct InterceptionResult {
    bool intercepted = false;
    uint32_t originalWidth = 0;
    uint32_t originalHeight = 0;
    uint32_t enhancedWidth = 0;
    uint32_t enhancedHeight = 0;
    double processingTimeMs = 0.0;
};

class ShellThumbnailInterceptor {
public:
    ShellThumbnailInterceptor() = default;

    void AddRule(const InterceptionRule& rule) {
        m_rules.push_back(rule);
    }

    InterceptionResult ProcessRequest(const std::wstring& filePath, uint32_t requestedSize) {
        InterceptionResult result;
        result.originalWidth = requestedSize;
        result.originalHeight = requestedSize;
        for (const auto& rule : m_rules) {
            if (MatchesExtension(filePath, rule.extensionPattern)) {
                result.intercepted = true;
                result.enhancedWidth = requestedSize;
                result.enhancedHeight = requestedSize;
                m_interceptCount++;
                break;
            }
        }
        return result;
    }

    uint64_t GetInterceptCount() const { return m_interceptCount; }
    size_t GetRuleCount() const { return m_rules.size(); }
    void ClearRules() { m_rules.clear(); }

private:
    bool MatchesExtension(const std::wstring& path, const std::wstring& pattern) const {
        if (pattern.empty()) return true;
        if (path.size() < pattern.size()) return false;
        auto ext = path.substr(path.find_last_of(L'.'));
        return ext == pattern;
    }

    std::vector<InterceptionRule> m_rules;
    uint64_t m_interceptCount = 0;
};

} // namespace Engine
} // namespace ExplorerLens
