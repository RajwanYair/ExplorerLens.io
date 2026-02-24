//==============================================================================
// ExplorerLens Engine — Plugin Marketplace V3
// Enhanced discovery, trust chain, sandboxed execution, auto-update, ratings.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

/// Plugin marketplace category V3
enum class PluginCategoryV3 : uint8_t {
    ImageDecoder,       // Image format decoders
    ArchiveHandler,     // Archive extraction
    DocumentViewer,     // Document preview
    ModelRenderer,      // 3D model rendering
    ScientificData,     // Scientific format support
    AudioVisualizer,    // Audio waveform/spectrum
    ThemeProvider,      // UI themes
    Utility,            // General utilities
    COUNT
};

/// Plugin trust level V3
enum class PluginTrustLevelV3 : uint8_t {
    Untrusted,
    CommunityReviewed,
    Verified,
    Official,
    COUNT
};

/// Plugin sandbox policy
enum class SandboxPolicy : uint8_t {
    None,               // No sandbox (legacy)
    FileSystem,         // Restrict file system access
    Network,            // Restrict network access
    Full,               // Full sandbox (FS + Net + Memory)
    COUNT
};

/// Marketplace entry V3
struct MarketplaceEntryV3 {
    std::wstring        pluginId;
    std::wstring        displayName;
    std::wstring        publisher;
    std::wstring        version;
    PluginCategoryV3    category    = PluginCategoryV3::Utility;
    PluginTrustLevelV3  trustLevel  = PluginTrustLevelV3::Untrusted;
    SandboxPolicy       sandbox     = SandboxPolicy::Full;
    uint32_t            downloads   = 0;
    double              rating      = 0.0;
    uint32_t            ratingCount = 0;
    bool                autoUpdate  = true;
};

/// Plugin Marketplace V3
class PluginMarketplaceV3 {
public:
    static const wchar_t* CategoryName(PluginCategoryV3 c) {
        switch (c) {
            case PluginCategoryV3::ImageDecoder:    return L"Image Decoder";
            case PluginCategoryV3::ArchiveHandler:  return L"Archive Handler";
            case PluginCategoryV3::DocumentViewer:  return L"Document Viewer";
            case PluginCategoryV3::ModelRenderer:   return L"Model Renderer";
            case PluginCategoryV3::ScientificData:  return L"Scientific Data";
            case PluginCategoryV3::AudioVisualizer: return L"Audio Visualizer";
            case PluginCategoryV3::ThemeProvider:    return L"Theme Provider";
            case PluginCategoryV3::Utility:         return L"Utility";
            default: return L"Unknown";
        }
    }

    static const wchar_t* TrustName(PluginTrustLevelV3 t) {
        switch (t) {
            case PluginTrustLevelV3::Untrusted:         return L"Untrusted";
            case PluginTrustLevelV3::CommunityReviewed: return L"Community Reviewed";
            case PluginTrustLevelV3::Verified:          return L"Verified";
            case PluginTrustLevelV3::Official:          return L"Official";
            default: return L"Unknown";
        }
    }

    static const wchar_t* SandboxName(SandboxPolicy s) {
        switch (s) {
            case SandboxPolicy::None:       return L"None";
            case SandboxPolicy::FileSystem: return L"File System";
            case SandboxPolicy::Network:    return L"Network";
            case SandboxPolicy::Full:       return L"Full";
            default: return L"Unknown";
        }
    }

    static constexpr size_t CategoryCount() { return static_cast<size_t>(PluginCategoryV3::COUNT); }
    static constexpr size_t TrustLevelCount() { return static_cast<size_t>(PluginTrustLevelV3::COUNT); }
    static constexpr size_t SandboxPolicyCount() { return static_cast<size_t>(SandboxPolicy::COUNT); }
};

}} // namespace ExplorerLens::Engine

