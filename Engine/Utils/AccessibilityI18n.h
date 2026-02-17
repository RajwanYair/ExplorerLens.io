//==============================================================================
// DarkThumbs Engine — Sprint 47: Accessibility & Internationalization
//
// Provides screen reader support for thumbnails and badges, high-contrast
// mode rendering, localization framework for UI strings, RTL layout
// support, keyboard navigation helpers, and WCAG 2.1 compliance
// infrastructure for the shell extension and manager GUI.
//==============================================================================
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <functional>

namespace DarkThumbs::Engine::Utils {

//==============================================================================
// Locale — Language + region identifier
//==============================================================================

struct Locale {
    std::string language;  // ISO 639-1: "en", "ja", "de", "he"
    std::string region;    // ISO 3166-1: "US", "JP", "DE", "IL"

    std::string Tag() const {
        if (region.empty()) return language;
        return language + "-" + region;
    }

    bool IsRTL() const {
        return language == "ar" || language == "he" ||
               language == "fa" || language == "ur";
    }

    bool IsEmpty() const { return language.empty(); }

    static Locale Parse(const std::string& tag) {
        Locale loc;
        auto sep = tag.find('-');
        if (sep == std::string::npos) sep = tag.find('_');
        if (sep != std::string::npos) {
            loc.language = tag.substr(0, sep);
            loc.region   = tag.substr(sep + 1);
        } else {
            loc.language = tag;
        }
        return loc;
    }

    static Locale English()  { return {"en", "US"}; }
    static Locale Japanese() { return {"ja", "JP"}; }
    static Locale German()   { return {"de", "DE"}; }
    static Locale Hebrew()   { return {"he", "IL"}; }
    static Locale Arabic()   { return {"ar", "SA"}; }
};

//==============================================================================
// String Table — Localized string storage
//==============================================================================

class StringTable {
public:
    void Set(const std::string& key, const std::string& value) {
        m_strings[key] = value;
    }

    std::string Get(const std::string& key, const std::string& fallback = "") const {
        auto it = m_strings.find(key);
        return (it != m_strings.end()) ? it->second : fallback;
    }

    bool Has(const std::string& key) const {
        return m_strings.count(key) > 0;
    }

    size_t Count() const { return m_strings.size(); }

    std::vector<std::string> MissingKeys(const StringTable& reference) const {
        std::vector<std::string> missing;
        for (const auto& [key, _] : reference.m_strings) {
            if (!Has(key)) missing.push_back(key);
        }
        return missing;
    }

    double CoveragePercent(const StringTable& reference) const {
        if (reference.Count() == 0) return 100.0;
        auto missing = MissingKeys(reference);
        return (1.0 - static_cast<double>(missing.size())
                / static_cast<double>(reference.Count())) * 100.0;
    }

    //--- Default English strings ---
    static StringTable DefaultEnglish() {
        StringTable t;
        // General
        t.Set("app.name", "DarkThumbs");
        t.Set("app.version", "v7.0.0");
        // Configuration
        t.Set("config.title", "DarkThumbs Configuration");
        t.Set("config.general", "General");
        t.Set("config.formats", "Formats");
        t.Set("config.cache", "Cache");
        t.Set("config.gpu", "GPU Settings");
        t.Set("config.plugins", "Plugins");
        t.Set("config.about", "About");
        // Actions
        t.Set("action.regenerate", "Regenerate Thumbnail");
        t.Set("action.copy", "Copy Thumbnail to Clipboard");
        t.Set("action.export", "Export Thumbnail as PNG");
        t.Set("action.findDuplicates", "Find Duplicate Images");
        t.Set("action.convert", "Convert Format");
        // Badges
        t.Set("badge.format", "Format: {0}");
        t.Set("badge.size", "File size: {0}");
        // Status messages
        t.Set("status.loading", "Loading thumbnail...");
        t.Set("status.error", "Failed to generate thumbnail");
        t.Set("status.cached", "Loaded from cache");
        t.Set("status.unsupported", "Unsupported format");
        return t;
    }

private:
    std::unordered_map<std::string, std::string> m_strings;
};

//==============================================================================
// Localization Manager — Multi-language string resolution
//==============================================================================

class LocalizationManager {
public:
    void RegisterLocale(const std::string& tag, const StringTable& table) {
        m_tables[tag] = table;
    }

    std::string Resolve(const std::string& key) const {
        // Try current locale
        auto it = m_tables.find(m_currentLocale.Tag());
        if (it != m_tables.end() && it->second.Has(key)) {
            return it->second.Get(key);
        }
        // Fall back to language only (e.g., "en" if "en-US" not found)
        it = m_tables.find(m_currentLocale.language);
        if (it != m_tables.end() && it->second.Has(key)) {
            return it->second.Get(key);
        }
        // Fall back to English
        it = m_tables.find("en-US");
        if (it != m_tables.end()) {
            return it->second.Get(key, key);
        }
        return key; // Return the key itself as last resort
    }

    void SetLocale(const Locale& loc) { m_currentLocale = loc; }
    const Locale& GetLocale() const   { return m_currentLocale; }

    bool IsRTL() const { return m_currentLocale.IsRTL(); }

    size_t LocaleCount() const { return m_tables.size(); }

    std::vector<std::string> AvailableLocales() const {
        std::vector<std::string> result;
        for (const auto& [tag, _] : m_tables) {
            result.push_back(tag);
        }
        return result;
    }

private:
    Locale m_currentLocale = Locale::English();
    std::unordered_map<std::string, StringTable> m_tables;
};

//==============================================================================
// Accessibility Description — Screen reader metadata for thumbnails
//==============================================================================

struct AccessibilityDescription {
    std::string name;           // Short: "sunset.heic"
    std::string role;           // "image", "button"
    std::string description;    // Full: "HEIF image, 4000x3000, 2.5 MB"
    std::string value;          // Optional: badge text
    std::string keyboardHint;   // "Press Enter to preview"
    bool        isImportant = false;

    bool IsEmpty() const { return name.empty() && description.empty(); }

    std::string NarratorText() const {
        std::string text = name;
        if (!description.empty()) {
            text += ". " + description;
        }
        if (!value.empty()) {
            text += ". " + value;
        }
        return text;
    }

    static AccessibilityDescription ForThumbnail(
        const std::string& fileName,
        const std::string& format,
        uint32_t width, uint32_t height,
        const std::string& fileSize)
    {
        AccessibilityDescription d;
        d.name = fileName;
        d.role = "image";
        d.description = format + " image, "
                       + std::to_string(width) + "x" + std::to_string(height)
                       + ", " + fileSize;
        d.keyboardHint = "Press Enter to preview, Space to select";
        return d;
    }

    static AccessibilityDescription ForBadge(
        const std::string& badgeText,
        const std::string& context)
    {
        AccessibilityDescription d;
        d.name = badgeText;
        d.role = "status";
        d.description = context;
        return d;
    }
};

//==============================================================================
// Contrast Mode — High-contrast rendering settings
//==============================================================================

enum class ContrastMode : uint8_t {
    Standard,       // Normal color scheme
    HighContrast,   // Windows High Contrast theme
    DarkMode,       // Dark theme with sufficient contrast
    Custom          // User-defined contrast settings
};

inline const char* ContrastModeName(ContrastMode m) {
    switch (m) {
        case ContrastMode::Standard:     return "Standard";
        case ContrastMode::HighContrast: return "High Contrast";
        case ContrastMode::DarkMode:     return "Dark Mode";
        case ContrastMode::Custom:       return "Custom";
        default:                         return "Unknown";
    }
}

struct ContrastConfig {
    ContrastMode mode = ContrastMode::Standard;
    uint32_t     badgeForeground = 0xFFFFFF; // White
    uint32_t     badgeBackground = 0x000000; // Black
    uint32_t     borderColor     = 0x333333;
    float        badgeOpacity    = 0.85f;
    float        minContrastRatio = 4.5f; // WCAG AA

    bool MeetsWCAGAA() const { return minContrastRatio >= 4.5f; }
    bool MeetsWCAGAAA() const { return minContrastRatio >= 7.0f; }

    static ContrastConfig Standard() {
        return {};
    }

    static ContrastConfig HighContrast() {
        ContrastConfig c;
        c.mode = ContrastMode::HighContrast;
        c.badgeForeground = 0xFFFF00; // Yellow on black
        c.badgeBackground = 0x000000;
        c.borderColor     = 0xFFFFFF;
        c.badgeOpacity    = 1.0f;
        c.minContrastRatio = 7.0f; // WCAG AAA
        return c;
    }

    static ContrastConfig DarkMode() {
        ContrastConfig c;
        c.mode = ContrastMode::DarkMode;
        c.badgeForeground = 0xE0E0E0;
        c.badgeBackground = 0x1E1E1E;
        c.borderColor     = 0x404040;
        c.badgeOpacity    = 0.90f;
        return c;
    }
};

//==============================================================================
// Keyboard Navigation — Focus and keyboard interaction support
//==============================================================================

struct KeyboardNavigation {
    bool     tabStopEnabled = true;
    int      tabOrder       = 0;
    bool     arrowKeyNavigation = true;
    std::string shortcutKey;      // e.g., "Ctrl+Shift+T"

    static KeyboardNavigation ForThumbnailGrid() {
        KeyboardNavigation n;
        n.tabStopEnabled = true;
        n.arrowKeyNavigation = true;
        return n;
    }

    static KeyboardNavigation ForContextMenu() {
        KeyboardNavigation n;
        n.tabStopEnabled = false;
        n.arrowKeyNavigation = true;
        return n;
    }
};

//==============================================================================
// Accessibility Config — Overall A11y settings
//==============================================================================

struct AccessibilityConfig {
    bool             screenReaderEnabled = false;
    bool             reduceMotion        = false;
    bool             largeFonts          = false;
    ContrastConfig   contrast;
    float            fontScale           = 1.0f;
    bool             keyboardOnly        = false;
    bool             announceStatusChanges = true;

    static AccessibilityConfig Default() {
        return {};
    }

    static AccessibilityConfig ScreenReaderOptimized() {
        AccessibilityConfig c;
        c.screenReaderEnabled = true;
        c.announceStatusChanges = true;
        c.reduceMotion = true;
        c.contrast = ContrastConfig::HighContrast();
        return c;
    }

    static AccessibilityConfig LowVision() {
        AccessibilityConfig c;
        c.largeFonts = true;
        c.fontScale = 1.5f;
        c.contrast = ContrastConfig::HighContrast();
        return c;
    }
};

} // namespace DarkThumbs::Engine::Utils
