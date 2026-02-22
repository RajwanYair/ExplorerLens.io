//==============================================================================
// DarkThumbs Engine — Sprint 346: Documentation Excellence V2
// Automated API doc generation (Doxygen/MSRC format), documentation coverage
// tracking per namespace, doc drift detection vs code changes, and
// readability scoring with Flesch-Kincaid grade level target.
//==============================================================================
#pragma once
#include <string>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

enum class DocFormat    : uint8_t { Doxygen=0, MSRC, Sphinx, DocFX, Markdown, COUNT };
enum class DocScope     : uint8_t { Public=0, Internal, All, COUNT };
enum class DocDriftLevel: uint8_t { Clean=0, Minor, Moderate, Critical, COUNT };

struct DocCoverageReport {
    std::wstring  namespaceName;
    uint32_t      symbolsTotal      = 0;
    uint32_t      symbolsDocumented = 0;
    float         coveragePct       = 0.0f;
    float         fleschKincaidGrade= 0.0f; // target ≤ 12 (college undergrad)
    DocDriftLevel driftLevel        = DocDriftLevel::Clean;
};

struct AutoDocConfig {
    DocFormat  outputFormat     = DocFormat::Doxygen;
    DocScope   scope            = DocScope::Public;
    bool       failOnDrift      = true;
    float      minCoveragePct   = 90.0f;
    float      maxGradeLevel    = 12.0f;
};

class DocumentationExcellenceV2 {
public:
    static const wchar_t* DocFormatName(DocFormat f) {
        switch(f) {
            case DocFormat::Doxygen:  return L"Doxygen";
            case DocFormat::MSRC:     return L"MSRC";
            case DocFormat::Sphinx:   return L"Sphinx";
            case DocFormat::DocFX:    return L"DocFX";
            case DocFormat::Markdown: return L"Markdown";
            default: return L"Unknown";
        }
    }
    static const wchar_t* DocScopeName(DocScope s) {
        switch(s) {
            case DocScope::Public:   return L"Public";
            case DocScope::Internal: return L"Internal";
            case DocScope::All:      return L"All";
            default: return L"Unknown";
        }
    }
    static const wchar_t* DriftLevelName(DocDriftLevel d) {
        switch(d) {
            case DocDriftLevel::Clean:    return L"Clean";
            case DocDriftLevel::Minor:    return L"Minor";
            case DocDriftLevel::Moderate: return L"Moderate";
            case DocDriftLevel::Critical: return L"Critical";
            default: return L"Unknown";
        }
    }
    static constexpr size_t DocFormatCount()    { return static_cast<size_t>(DocFormat::COUNT); }
    static constexpr size_t DocScopeCount()     { return static_cast<size_t>(DocScope::COUNT); }
    static constexpr size_t DriftLevelCount()   { return static_cast<size_t>(DocDriftLevel::COUNT); }
    static bool PassesCoverage(const DocCoverageReport& r, float minPct) {
        return r.coveragePct >= minPct;
    }
};

}} // namespace DarkThumbs::Engine
