#pragma once
//==============================================================================
// AccessibilityEngine.h — Merged Accessibility Module
//
// Contains:
//   - AccessibilityFramework: Screen reader, keyboard nav, localization, UIA
//   - AccessibilityI18n: Locale, string tables, contrast, a11y config
//   - AccessibilityEngine: Core accessibility engine class
//==============================================================================

#include <algorithm>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// ─── AccessibilityFramework ─────────────────────────────────────────────────
namespace ExplorerLens {
namespace Engine {

// ============================================================================
// Localization Types
// ============================================================================

/// Supported UI languages
enum class Language {
    English, ///< en-US (default, fallback)
    German, ///< de-DE
    Japanese, ///< ja-JP
    ChineseSimp, ///< zh-CN
    Arabic ///< ar-SA (RTL)
};

/// Text direction (framework-local — see LocalizationEngine.h for canonical TextDirection)
enum class TextDir {
    LTR, ///< Left-to-Right (English, German, Japanese, Chinese)
    RTL ///< Right-to-Left (Arabic, Hebrew)
};

/// Plural category (CLDR standard)
enum class PluralCategory {
    Zero,
    One,
    Two,
    Few,
    Many,
    Other
};

/// A localized string entry (framework-local — see LocalizationEngine.h for canonical LocalizedString)
struct LocalizedStringEntry {
    std::string key; ///< Unique string ID (e.g., "menu.file.open")
    std::string defaultValue; ///< English fallback value
    std::map<Language, std::string> translations;

    /// Get translation for language, falling back to English
    const std::string& Get(Language lang) const {
        auto it = translations.find(lang);
        if (it != translations.end()) return it->second;
        return defaultValue;
    }
};

/// Language metadata
struct LanguageInfo {
    Language id;
    std::string code; ///< BCP 47 tag: "en-US", "de-DE", etc.
    std::string nativeName; ///< Name in native script
    std::string englishName; ///< Name in English
    TextDir direction = TextDir::LTR;
    bool isComplete = true; ///< Whether all strings are translated

    static LanguageInfo Get(Language lang) {
        switch (lang) {
        case Language::English:
            return { lang, "en-US", "English", "English", TextDir::LTR, true };
        case Language::German:
            return { lang, "de-DE", "Deutsch", "German", TextDir::LTR, true };
        case Language::Japanese:
            return { lang, "ja-JP", "\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e",
            "Japanese", TextDir::LTR, true };
        case Language::ChineseSimp:
            return { lang, "zh-CN", "\xe7\xae\x80\xe4\xbd\x93\xe4\xb8\xad\xe6\x96\x87",
            "Chinese (Simplified)", TextDir::LTR, true };
        case Language::Arabic:
            return { lang, "ar-SA", "\xd8\xa7\xd9\x84\xd8\xb9\xd8\xb1\xd8\xa8\xd9\x8a\xd8\xa9",
            "Arabic", TextDir::RTL, true };
        default:
            return { Language::English, "en-US", "English", "English" };
        }
    }
};

/// Manages string tables and localization
class LocalizationManager {
public:
    LocalizationManager() : m_currentLang(Language::English) {
        InitializeDefaultStrings();
    }

    /// Set the active UI language
    void SetLanguage(Language lang) {
        m_currentLang = lang;
        m_langInfo = LanguageInfo::Get(lang);
    }

    /// Get the active language
    Language GetLanguage() const { return m_currentLang; }

    /// Get language info for active language
    const LanguageInfo& GetLanguageInfo() const { return m_langInfo; }

    /// Get current text direction
    TextDir GetTextDirection() const { return m_langInfo.direction; }

    /// Look up a localized string by key
    std::string GetString(const std::string& key) const {
        auto it = m_strings.find(key);
        if (it != m_strings.end()) {
            return it->second.Get(m_currentLang);
        }
        return "[MISSING:" + key + "]";
    }

    /// Format a number according to locale
    std::string FormatNumber(int64_t value) const {
        std::string str = std::to_string(std::abs(value));
        std::string result;

        // Insert thousand separators
        char sep = (m_currentLang == Language::German) ? '.' : ',';
        int count = 0;
        for (auto it = str.rbegin(); it != str.rend(); ++it) {
            if (count > 0 && count % 3 == 0) result = sep + result;
            result = *it + result;
            count++;
        }

        if (value < 0) result = "-" + result;
        return result;
    }

    /// Get plural category for a count (simplified CLDR rules)
    PluralCategory GetPluralCategory(int32_t count) const {
        switch (m_currentLang) {
        case Language::Arabic:
            // Arabic has complex plural rules
            if (count == 0) return PluralCategory::Zero;
            if (count == 1) return PluralCategory::One;
            if (count == 2) return PluralCategory::Two;
            if (count % 100 >= 3 && count % 100 <= 10) return PluralCategory::Few;
            if (count % 100 >= 11) return PluralCategory::Many;
            return PluralCategory::Other;

        case Language::Japanese:
        case Language::ChineseSimp:
            return PluralCategory::Other; // No grammatical plurals

        default:
            return (count == 1) ? PluralCategory::One : PluralCategory::Other;
        }
    }

    /// Register a localized string
    void RegisterString(const std::string& key, const std::string& defaultValue,
        const std::map<Language, std::string>& translations = {}) {
        LocalizedStringEntry ls;
        ls.key = key;
        ls.defaultValue = defaultValue;
        ls.translations = translations;
        ls.translations[Language::English] = defaultValue;
        m_strings[key] = std::move(ls);
    }

    /// Get all available languages
    std::vector<LanguageInfo> GetAvailableLanguages() const {
        return {
        LanguageInfo::Get(Language::English),
        LanguageInfo::Get(Language::German),
        LanguageInfo::Get(Language::Japanese),
        LanguageInfo::Get(Language::ChineseSimp),
        LanguageInfo::Get(Language::Arabic)
        };
    }

    /// Calculate localization completeness for a language
    double GetCompleteness(Language lang) const {
        if (m_strings.empty()) return 0.0;
        uint32_t translated = 0;
        for (const auto& [key, ls] : m_strings) {
            if (ls.translations.count(lang) > 0) translated++;
        }
        return static_cast<double>(translated) / m_strings.size();
    }

private:
    void InitializeDefaultStrings() {
        RegisterString("app.name", "ExplorerLens", {
        {Language::German, "ExplorerLens"},
        {Language::Japanese, "ExplorerLens"},
        {Language::ChineseSimp, "ExplorerLens"},
        {Language::Arabic, "ExplorerLens"}
            });
        RegisterString("menu.settings", "Settings", {
        {Language::German, "Einstellungen"},
        {Language::Japanese, "\xe8\xa8\xad\xe5\xae\x9a"},
        {Language::ChineseSimp, "\xe8\xae\xbe\xe7\xbd\xae"},
        {Language::Arabic, "\xd8\xa7\xd9\x84\xd8\xa5\xd8\xb9\xd8\xaf\xd8\xa7\xd8\xaf\xd8\xa7\xd8\xaa"}
            });
        RegisterString("status.loading", "Loading thumbnails...", {
        {Language::German, "Lade Vorschaubilder..."},
        {Language::Japanese, "\xe3\x82\xb5\xe3\x83\xa0\xe3\x83\x8d\xe3\x82\xa4\xe3\x83\xab\xe3\x82\x92\xe8\xaa\xad\xe3\x81\xbf\xe8\xbe\xbc\xe3\x82\x93\xe3\x81\xa7\xe3\x81\x84\xe3\x81\xbe\xe3\x81\x99..."},
        {Language::ChineseSimp, "\xe6\xad\xa3\xe5\x9c\xa8\xe5\x8a\xa0\xe8\xbd\xbd\xe7\xbc\xa9\xe7\x95\xa5\xe5\x9b\xbe..."},
        {Language::Arabic, "\xd8\xac\xd8\xa7\xd8\xb1\xd9\x8a \xd8\xaa\xd8\xad\xd9\x85\xd9\x8a\xd9\x84 \xd8\xa7\xd9\x84\xd8\xb5\xd9\x88\xd8\xb1 \xd8\xa7\xd9\x84\xd9\x85\xd8\xb5\xd8\xba\xd8\xb1\xd8\xa9..."}
            });
        RegisterString("format.count", "{0} files", {
        {Language::German, "{0} Dateien"},
        {Language::Japanese, "{0} \xe3\x83\x95\xe3\x82\xa1\xe3\x82\xa4\xe3\x83\xab"},
        {Language::ChineseSimp, "{0} \xe4\xb8\xaa\xe6\x96\x87\xe4\xbb\xb6"},
        {Language::Arabic, "{0} \xd9\x85\xd9\x84\xd9\x81\xd8\xa7\xd8\xaa"}
            });
    }

    Language m_currentLang;
    LanguageInfo m_langInfo = LanguageInfo::Get(Language::English);
    std::unordered_map<std::string, LocalizedStringEntry> m_strings;
};


// ============================================================================
// Keyboard Navigation
// ============================================================================

/// Keyboard navigation actions
enum class NavAction {
    NextControl, ///< Tab
    PrevControl, ///< Shift+Tab
    Activate, ///< Enter / Space
    Cancel, ///< Escape
    MoveUp, ///< Arrow Up
    MoveDown, ///< Arrow Down
    MoveLeft, ///< Arrow Left
    MoveRight, ///< Arrow Right
    SelectAll, ///< Ctrl+A
    Search, ///< Ctrl+F
    Help, ///< F1
    ContextMenu ///< Shift+F10 / Application key
};

/// Virtual key to NavAction mapping
struct KeyMapping {
    uint32_t virtualKey;
    bool ctrl = false;
    bool shift = false;
    bool alt = false;
    NavAction action;
};

/// Manages keyboard focus and navigation
class KeyboardNavigator {
public:
    KeyboardNavigator() {
        InitializeDefaultMappings();
    }

    /// Map a virtual key combination to a navigation action
    void AddMapping(uint32_t vk, bool ctrl, bool shift, bool alt, NavAction action) {
        m_mappings.push_back({ vk, ctrl, shift, alt, action });
    }

    /// Process a key event
    bool ProcessKey(uint32_t vk, bool ctrl, bool shift, bool alt, NavAction& outAction) const {
        for (const auto& m : m_mappings) {
            if (m.virtualKey == vk && m.ctrl == ctrl && m.shift == shift && m.alt == alt) {
                outAction = m.action;
                return true;
            }
        }
        return false;
    }

    /// Set focus to next/previous control
    int32_t MoveFocus(int32_t currentIndex, int32_t controlCount, bool forward) const {
        if (controlCount <= 0) return -1;
        if (forward) {
            return (currentIndex + 1) % controlCount;
        }
        else {
            return (currentIndex - 1 + controlCount) % controlCount;
        }
    }

    /// Check if a control index is valid
    bool IsFocusValid(int32_t index, int32_t controlCount) const {
        return index >= 0 && index < controlCount;
    }

    /// Get tab order for all registered controls
    const std::vector<KeyMapping>& GetMappings() const { return m_mappings; }

private:
    void InitializeDefaultMappings() {
        AddMapping(0x09, false, false, false, NavAction::NextControl); // Tab
        AddMapping(0x09, false, true, false, NavAction::PrevControl); // Shift+Tab
        AddMapping(0x0D, false, false, false, NavAction::Activate); // Enter
        AddMapping(0x20, false, false, false, NavAction::Activate); // Space
        AddMapping(0x1B, false, false, false, NavAction::Cancel); // Escape
        AddMapping(0x26, false, false, false, NavAction::MoveUp); // Arrow Up
        AddMapping(0x28, false, false, false, NavAction::MoveDown); // Arrow Down
        AddMapping(0x25, false, false, false, NavAction::MoveLeft); // Arrow Left
        AddMapping(0x27, false, false, false, NavAction::MoveRight); // Arrow Right
        AddMapping('A', true, false, false, NavAction::SelectAll); // Ctrl+A
        AddMapping('F', true, false, false, NavAction::Search); // Ctrl+F
        AddMapping(0x70, false, false, false, NavAction::Help); // F1
        AddMapping(0x79, false, true, false, NavAction::ContextMenu); // Shift+F10
    }

    std::vector<KeyMapping> m_mappings;
};


// ============================================================================
// UI Automation (Screen Reader Support)
// ============================================================================

/// UIA control types (framework-local — see AccessibilitySuiteV2.h for canonical UIAControlType)
enum class UIAControlKind {
    Window,
    Button,
    CheckBox,
    ComboBox,
    ListItem,
    Image,
    Text,
    Group,
    ToolBar,
    StatusBar,
    Tab,
    TabItem,
    Menu,
    MenuItem
};

/// Accessible element information
struct AccessibleElement {
    uint32_t id = 0;
    std::string name; ///< Accessible name (screen reader text)
    std::string description; ///< Accessible description / help text
    UIAControlKind controlType = UIAControlKind::Button;
    std::string automationId; ///< Unique automation identifier
    bool isEnabled = true;
    bool isVisible = true;
    bool isKeyboardFocusable = true;
    std::string value; ///< Current value for editable controls
    int32_t tabOrder = -1; ///< Tab navigation order (-1 = not in tab order)
};

/// UIA provider that exposes accessible element tree
class AccessibilityProvider {
public:
    /// Register an accessible UI element
    void RegisterElement(AccessibleElement element) {
        m_elements[element.id] = std::move(element);
    }

    /// Get element by ID
    const AccessibleElement* GetElement(uint32_t id) const {
        auto it = m_elements.find(id);
        return it != m_elements.end() ? &it->second : nullptr;
    }

    /// Get all focusable elements in tab order
    std::vector<const AccessibleElement*> GetTabOrder() const {
        std::vector<const AccessibleElement*> ordered;
        for (const auto& [id, elem] : m_elements) {
            if (elem.isKeyboardFocusable && elem.tabOrder >= 0) {
                ordered.push_back(&elem);
            }
        }
        std::sort(ordered.begin(), ordered.end(),
            [](const AccessibleElement* a, const AccessibleElement* b) {
                return a->tabOrder < b->tabOrder;
            });
        return ordered;
    }

    /// Raise an automation event (focus changed, value changed, etc.)
    void RaiseEvent(uint32_t elementId, const std::string& eventType) {
        m_lastEventElementId = elementId;
        m_lastEventType = eventType;
        m_eventCount++;
    }

    /// Accessibility audit — find elements missing accessible names
    std::vector<uint32_t> AuditMissingNames() const {
        std::vector<uint32_t> missing;
        for (const auto& [id, elem] : m_elements) {
            if (elem.name.empty() && elem.controlType != UIAControlKind::Group) {
                missing.push_back(id);
            }
        }
        return missing;
    }

    uint32_t GetElementCount() const { return static_cast<uint32_t>(m_elements.size()); }
    uint32_t GetEventCount() const { return m_eventCount; }
    uint32_t GetLastEventElementId() const { return m_lastEventElementId; }

private:
    std::map<uint32_t, AccessibleElement> m_elements;
    uint32_t m_lastEventElementId = 0;
    std::string m_lastEventType;
    uint32_t m_eventCount = 0;
};


// ============================================================================
// High Contrast & Theme Adaptation
// ============================================================================

/// Theme mode
enum class ThemeMode {
    Light,
    Dark,
    HighContrastBlack,
    HighContrastWhite,
    System ///< Follow OS setting
};

/// Color for accessible UI rendering
struct AccessibleColor {
    uint8_t r, g, b, a;

    /// Calculate relative luminance (WCAG 2.1 formula)
    double GetRelativeLuminance() const {
        auto srgbToLinear = [](uint8_t val) -> double {
            double v = val / 255.0;
            return v <= 0.03928 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4);
            };
        return 0.2126 * srgbToLinear(r) + 0.7152 * srgbToLinear(g) + 0.0722 * srgbToLinear(b);
    }

    /// WCAG contrast ratio with another color
    double ContrastRatio(const AccessibleColor& other) const {
        double l1 = GetRelativeLuminance();
        double l2 = other.GetRelativeLuminance();
        double lighter = (l1 > l2) ? l1 : l2;
        double darker = (l1 > l2) ? l2 : l1;
        return (lighter + 0.05) / (darker + 0.05);
    }
};

/// WCAG compliance level (framework-local — see AccessibilitySuiteV2.h for canonical WCAGLevel)
enum class WCAGContrastLevel {
    None, ///< Does not meet any level
    A, ///< Basic accessibility
    AA, ///< Standard (target for ExplorerLens)
    AAA ///< Enhanced
};

/// Check if contrast ratio meets WCAG level for normal text
inline WCAGContrastLevel CheckContrastCompliance(double contrastRatio, bool largeText = false) {
    if (largeText) {
        if (contrastRatio >= 4.5) return WCAGContrastLevel::AAA;
        if (contrastRatio >= 3.0) return WCAGContrastLevel::AA;
    }
    else {
        if (contrastRatio >= 7.0) return WCAGContrastLevel::AAA;
        if (contrastRatio >= 4.5) return WCAGContrastLevel::AA;
        if (contrastRatio >= 3.0) return WCAGContrastLevel::A;
    }
    return WCAGContrastLevel::None;
}

/// Accessibility statistics
struct AccessibilityStats {
    uint32_t elementsRegistered = 0;
    uint32_t elementsMissingNames = 0;
    uint32_t keyboardEventsProcessed = 0;
    uint32_t automationEventsRaised = 0;
    uint32_t stringsLocalized = 0;
    uint32_t languagesAvailable = 5; ///< en, de, ja, zh, ar
    double localizationCompleteness = 1.0;
    WCAGContrastLevel complianceLevel = WCAGContrastLevel::AA;

    void Reset() { *this = AccessibilityStats{}; }
};

} // namespace Engine
} // namespace ExplorerLens

// ─── AccessibilityI18n ──────────────────────────────────────────────────────
namespace ExplorerLens::Engine::Utils {

//==============================================================================
// Locale — Language + region identifier
//==============================================================================

struct Locale {
    std::string language; // ISO 639-1: "en", "ja", "de", "he"
    std::string region; // ISO 3166-1: "US", "JP", "DE", "IL"

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
            loc.region = tag.substr(sep + 1);
        }
        else {
            loc.language = tag;
        }
        return loc;
    }

    static Locale English() { return { "en", "US" }; }
    static Locale Japanese() { return { "ja", "JP" }; }
    static Locale German() { return { "de", "DE" }; }
    static Locale Hebrew() { return { "he", "IL" }; }
    static Locale Arabic() { return { "ar", "SA" }; }
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
        t.Set("app.name", "ExplorerLens");
        t.Set("app.version", "v7.0.0");
        // Configuration
        t.Set("config.title", "ExplorerLens Configuration");
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
    const Locale& GetLocale() const { return m_currentLocale; }

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
    std::string name; // Short: "sunset.heic"
    std::string role; // "image", "button"
    std::string description; // Full: "HEIF image, 4000x3000, 2.5 MB"
    std::string value; // Optional: badge text
    std::string keyboardHint; // "Press Enter to preview"
    bool isImportant = false;

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
        const std::string& fileSize) {
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
        const std::string& context) {
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
    Standard, // Normal color scheme
    HighContrast, // Windows High Contrast theme
    DarkMode, // Dark theme with sufficient contrast
    Custom // User-defined contrast settings
};

inline const char* ContrastModeName(ContrastMode m) {
    switch (m) {
    case ContrastMode::Standard: return "Standard";
    case ContrastMode::HighContrast: return "High Contrast";
    case ContrastMode::DarkMode: return "Dark Mode";
    case ContrastMode::Custom: return "Custom";
    default: return "Unknown";
    }
}

struct ContrastConfig {
    ContrastMode mode = ContrastMode::Standard;
    uint32_t badgeForeground = 0xFFFFFF; // White
    uint32_t badgeBackground = 0x000000; // Black
    uint32_t borderColor = 0x333333;
    float badgeOpacity = 0.85f;
    float minContrastRatio = 4.5f; // WCAG AA

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
        c.borderColor = 0xFFFFFF;
        c.badgeOpacity = 1.0f;
        c.minContrastRatio = 7.0f; // WCAG AAA
        return c;
    }

    static ContrastConfig DarkMode() {
        ContrastConfig c;
        c.mode = ContrastMode::DarkMode;
        c.badgeForeground = 0xE0E0E0;
        c.badgeBackground = 0x1E1E1E;
        c.borderColor = 0x404040;
        c.badgeOpacity = 0.90f;
        return c;
    }
};

//==============================================================================
// Keyboard Navigation — Focus and keyboard interaction support
//==============================================================================

struct KeyboardNavigation {
    bool tabStopEnabled = true;
    int tabOrder = 0;
    bool arrowKeyNavigation = true;
    std::string shortcutKey; // e.g., "Ctrl+Shift+T"

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
    bool screenReaderEnabled = false;
    bool reduceMotion = false;
    bool largeFonts = false;
    ContrastConfig contrast;
    float fontScale = 1.0f;
    bool keyboardOnly = false;
    bool announceStatusChanges = true;

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

} // namespace ExplorerLens::Engine::Utils

// ─── AccessibilityEngine ────────────────────────────────────────────────────
namespace ExplorerLens {
namespace Engine {

enum class A11yFeature : uint8_t {
    ScreenReader = 0,
    HighContrast = 1,
    ReducedMotion = 2,
    LargeText = 3,
    KeyboardNav = 4,
    ColorBlindMode = 5,
    NarratorSupport = 6,
    FeatureCount = 7
};

enum class ContrastMode : uint8_t {
    Normal = 0,
    HighWhite = 1,
    HighBlack = 2,
    Custom = 3
};

struct A11yStatus {
    bool screenReaderActive = false;
    bool highContrastEnabled = false;
    ContrastMode contrastMode = ContrastMode::Normal;
    bool reducedMotion = false;
    uint32_t textScalePercent = 100;
    uint32_t featuresEnabled = 0;
};

struct A11yAuditResult {
    bool compliant = false;
    uint32_t checksRun = 0;
    uint32_t checksPassed = 0;
    double auditTimeMs = 0.0;
    std::vector<std::wstring> issues;
};

class AccessibilityEngine {
public:
    AccessibilityEngine();

    A11yStatus DetectSettings() const;
    A11yAuditResult RunComplianceAudit() const;

    bool IsFeatureEnabled(A11yFeature feature) const;
    void EnableFeature(A11yFeature feature);
    void DisableFeature(A11yFeature feature);

    uint32_t GetEnabledFeatureCount() const;

    static const wchar_t* GetFeatureName(A11yFeature feature);
    static const wchar_t* GetContrastModeName(ContrastMode mode);
    static uint32_t GetFeatureCount() { return static_cast<uint32_t>(A11yFeature::FeatureCount); }

private:
    uint32_t m_enabledFeatures = 0;
};

}
} // namespace ExplorerLens::Engine
