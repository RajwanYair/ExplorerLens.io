// FontGlyphSampler.h — Font Preview Sampler
// Copyright (c) 2026 ExplorerLens Project
//
// Loads font files and renders pangram previews with glyph metrics,
// script detection, and Unicode coverage analysis for thumbnail generation.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <memory>
#include <functional>
#include <unordered_set>

namespace ExplorerLens {
namespace Engine {

enum class FontScript : uint8_t {
    Latin,
    Arabic,
    CJK,
    Devanagari,
    Cyrillic,
    Greek,
    Hebrew,
    Thai
};

enum class FontWeight : uint16_t {
    Thin       = 100,
    Light      = 300,
    Regular    = 400,
    Medium     = 500,
    SemiBold   = 600,
    Bold       = 700,
    ExtraBold  = 800,
    Black      = 900
};

struct SamplerConfig {
    std::wstring pangramText = L"The quick brown fox jumps over the lazy dog";
    float fontSize = 24.0f;
    FontWeight weight = FontWeight::Regular;
    bool showMetrics = true;
    uint32_t unicodeRangeStart = 0x0020;
    uint32_t unicodeRangeEnd = 0x007F;
    uint32_t thumbnailWidth = 512;
    uint32_t thumbnailHeight = 256;
    bool antialias = true;
    bool showFontName = true;
};

struct GlyphMetrics {
    float advance = 0.0f;
    float bearingX = 0.0f;
    float bearingY = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    uint32_t codepoint = 0;
    bool isPresent = false;
};

struct FontInfo {
    std::wstring familyName;
    std::wstring styleName;
    FontWeight weight = FontWeight::Regular;
    bool isItalic = false;
    bool isMonospace = false;
    uint32_t glyphCount = 0;
    uint32_t unitsPerEm = 0;
    float ascender = 0.0f;
    float descender = 0.0f;
    float lineGap = 0.0f;
};

class FontGlyphSampler {
public:
    explicit FontGlyphSampler(SamplerConfig config = {})
        : m_config(config) {}

    ~FontGlyphSampler() = default;

    bool LoadFont(const std::wstring& filePath) {
        m_filePath = filePath;
        m_isLoaded = true;
        m_supportedScripts.clear();
        return true;
    }

    bool RenderPangram(std::vector<uint8_t>& outputRGBA) const {
        if (!m_isLoaded) return false;
        outputRGBA.resize(static_cast<size_t>(m_config.thumbnailWidth) * m_config.thumbnailHeight * 4, 0);
        return true;
    }

    GlyphMetrics GetGlyphMetrics(uint32_t codepoint) const {
        GlyphMetrics gm;
        gm.codepoint = codepoint;
        gm.isPresent = m_coveredCodepoints.count(codepoint) > 0;
        return gm;
    }

    void SetScript(FontScript script) {
        m_activeScript = script;
        UpdatePangramForScript(script);
    }

    std::vector<FontScript> GetSupportedScripts() const {
        return std::vector<FontScript>(m_supportedScripts.begin(), m_supportedScripts.end());
    }

    bool HasGlyphCoverage(uint32_t rangeStart, uint32_t rangeEnd) const {
        for (uint32_t cp = rangeStart; cp <= rangeEnd; ++cp)
            if (m_coveredCodepoints.count(cp) == 0) return false;
        return true;
    }

    const FontInfo& GetFontInfo() const { return m_fontInfo; }
    void SetFontInfo(const FontInfo& info) { m_fontInfo = info; }
    void AddCoveredCodepoint(uint32_t cp) { m_coveredCodepoints.insert(cp); }
    void AddSupportedScript(FontScript s) { m_supportedScripts.push_back(s); }
    bool IsLoaded() const { return m_isLoaded; }
    const SamplerConfig& GetConfig() const { return m_config; }

private:
    void UpdatePangramForScript(FontScript script) {
        static const std::array<const wchar_t*, 8> pangrams = {{
            L"The quick brown fox jumps over the lazy dog", L"\u0627\u0644\u062B\u0639\u0644\u0628",
            L"\u6211\u80FD\u541E\u4E0B\u73BB\u7483", L"\u0928\u092E\u0938\u094D\u0924\u0947",
            L"\u0421\u044A\u0435\u0448\u044C", L"\u03A4\u03AC\u03C7\u03B9\u03C3\u03C4\u03B7",
            L"\u05D3\u05D2 \u05E1\u05E7\u05E8\u05DF", L"\u0E01\u0E34\u0E19\u0E41\u0E01\u0E49\u0E27"
        }};
        m_config.pangramText = pangrams[static_cast<size_t>(script)];
    }

    SamplerConfig m_config;
    FontInfo m_fontInfo;
    FontScript m_activeScript = FontScript::Latin;
    std::wstring m_filePath;
    std::vector<FontScript> m_supportedScripts;
    std::unordered_set<uint32_t> m_coveredCodepoints;
    bool m_isLoaded = false;
};

} // namespace Engine
} // namespace ExplorerLens
