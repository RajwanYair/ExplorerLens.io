// CopilotPlatformBridge.h — Windows Copilot Platform Bridge
// Copyright (c) 2026 ExplorerLens Project
//
// Integrates ExplorerLens with the Windows Copilot Platform (Phi Silica / AI Runtime),
// enabling on-device AI features such as semantic file search and smart thumbnail ranking.
//
#pragma once
#include <string>
#include <vector>
#include <functional>

namespace ExplorerLens {
namespace Engine {

enum class CopilotFeature {
    SemanticSearch,
    SmartThumbnailRank,
    FileDescriptionGeneration,
    ContentSummary
};

enum class CopilotRuntimeStatus { Available, Unavailable, Loading, Error };

struct CopilotInvokeRequest {
    CopilotFeature   feature      = CopilotFeature::SemanticSearch;
    std::wstring     filePath;
    std::string      prompt;
    int              maxTokens    = 256;
};

struct CopilotInvokeResult {
    bool        success    = false;
    std::string output;
    double      confidence = 0.0;
    std::string errorMsg;
    bool Ok() const noexcept { return success; }
};

using CopilotInvokeFn = std::function<CopilotInvokeResult(const CopilotInvokeRequest&)>;

class CopilotPlatformBridge {
public:
    explicit CopilotPlatformBridge() = default;
    void SetInvokeFunction(CopilotInvokeFn fn) { m_invokeFn = std::move(fn); }
    void SetStatus(CopilotRuntimeStatus status) noexcept { m_status = status; }

    CopilotRuntimeStatus Status() const noexcept { return m_status; }
    bool IsAvailable() const noexcept { return m_status == CopilotRuntimeStatus::Available; }

    CopilotInvokeResult Invoke(const CopilotInvokeRequest& req) const {
        if (!IsAvailable()) return { false, {}, 0.0, "Copilot runtime not available" };
        if (!m_invokeFn)    return { false, {}, 0.0, "No invoke function configured" };
        return m_invokeFn(req);
    }

    static std::string FeatureName(CopilotFeature f) noexcept {
        switch (f) {
        case CopilotFeature::SemanticSearch:             return "SemanticSearch";
        case CopilotFeature::SmartThumbnailRank:         return "SmartThumbnailRank";
        case CopilotFeature::FileDescriptionGeneration:  return "FileDescriptionGeneration";
        case CopilotFeature::ContentSummary:             return "ContentSummary";
        }
        return "Unknown";
    }

    static std::string StatusName(CopilotRuntimeStatus s) noexcept {
        switch (s) {
        case CopilotRuntimeStatus::Available:   return "Available";
        case CopilotRuntimeStatus::Unavailable: return "Unavailable";
        case CopilotRuntimeStatus::Loading:     return "Loading";
        case CopilotRuntimeStatus::Error:       return "Error";
        }
        return "Unknown";
    }

private:
    CopilotRuntimeStatus m_status = CopilotRuntimeStatus::Unavailable;
    CopilotInvokeFn      m_invokeFn;
};

} // namespace Engine
} // namespace ExplorerLens
