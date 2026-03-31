// DocumentationExcellenceV2.h — Documentation Excellence Engine v2
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks documentation coverage, format output options, scope classification,
// and documentation drift detection across the codebase.
//
#pragma once
#include <cstddef>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

enum class DocOutputFormat : uint8_t {
    Doxygen,
    Sphinx,
    Markdown,
    HTML,
    PDF,
    COUNT = 5
};

enum class DocScope : uint8_t {
    Public,
    Protected,
    Private,
    Internal,
    COUNT = 4
};

enum class DocDriftLevel : uint8_t {
    Clean,
    Minor,
    Moderate,
    Severe,
    Critical,
    COUNT = 5
};

class DocumentationExcellenceV2 {
public:
    static const wchar_t *DocFormatName(DocOutputFormat f) noexcept {
        switch (f) {
        case DocOutputFormat::Doxygen:  return L"Doxygen";
        case DocOutputFormat::Sphinx:   return L"Sphinx";
        case DocOutputFormat::Markdown: return L"Markdown";
        case DocOutputFormat::HTML:     return L"HTML";
        case DocOutputFormat::PDF:      return L"PDF";
        default: return L"Unknown";
        }
    }
    static const wchar_t *DocScopeName(DocScope s) noexcept {
        switch (s) {
        case DocScope::Public:    return L"Public";
        case DocScope::Protected: return L"Protected";
        case DocScope::Private:   return L"Private";
        case DocScope::Internal:  return L"Internal";
        default: return L"Unknown";
        }
    }
    static const wchar_t *DriftLevelName(DocDriftLevel d) noexcept {
        switch (d) {
        case DocDriftLevel::Clean:    return L"Clean";
        case DocDriftLevel::Minor:    return L"Minor";
        case DocDriftLevel::Moderate: return L"Moderate";
        case DocDriftLevel::Severe:   return L"Severe";
        case DocDriftLevel::Critical: return L"Critical";
        default: return L"Unknown";
        }
    }
    static size_t DocFormatCount() noexcept {
        return static_cast<size_t>(DocOutputFormat::COUNT);
    }
    static size_t DocScopeCount() noexcept {
        return static_cast<size_t>(DocScope::COUNT);
    }
};

} // namespace Engine
} // namespace ExplorerLens
