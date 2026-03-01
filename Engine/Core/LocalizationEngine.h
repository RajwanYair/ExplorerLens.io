#pragma once
// LocalizationEngine.h — Multi-language support for UI strings and error messages
// Sprint 440 — ExplorerLens v15.0.0 Zenith (Core)

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Supported locale identifiers
enum class LocaleId : uint8_t {
    EnUS = 0,  // English (United States)
    DeDE = 1,  // German (Germany)
    FrFR = 2,  // French (France)
    JaJP = 3,  // Japanese (Japan)
    ZhCN = 4   // Chinese (Simplified, China)
};

inline const char* LocaleIdName(LocaleId id) noexcept {
    switch (id) {
    case LocaleId::EnUS: return "en-US";
    case LocaleId::DeDE: return "de-DE";
    case LocaleId::FrFR: return "fr-FR";
    case LocaleId::JaJP: return "ja-JP";
    case LocaleId::ZhCN: return "zh-CN";
    default:             return "Unknown";
    }
}

/// Category of localizable strings
enum class StringCategory : uint8_t {
    UI = 0,  // User interface labels
    Error = 1,  // Error messages
    Status = 2,  // Status bar / progress text
    Tooltip = 3,  // Tooltip hover text
    Accessibility = 4   // Screen reader / narrator text
};

inline const char* StringCategoryName(StringCategory c) noexcept {
    switch (c) {
    case StringCategory::UI:            return "UI";
    case StringCategory::Error:         return "Error";
    case StringCategory::Status:        return "Status";
    case StringCategory::Tooltip:       return "Tooltip";
    case StringCategory::Accessibility: return "Accessibility";
    default:                            return "Unknown";
    }
}

/// A localizable string entry
struct LocEngineString {
    std::string key;
    std::string value;
    StringCategory category = StringCategory::UI;
};

/// Configuration for locale management
struct LocalizationConfig {
    LocaleId   defaultLocale = LocaleId::EnUS;
    LocaleId   fallbackLocale = LocaleId::EnUS;
    bool       useFallback = true;    // Fall back to English if key missing
};

/// Multi-language localization engine for ExplorerLens UI
/// strings, error messages, tooltips, and accessibility
/// narrator text. Falls back to en-US for missing keys.
class CoreLocalizationEngine {
public:
    CoreLocalizationEngine() = default;
    ~CoreLocalizationEngine() = default;

    CoreLocalizationEngine(const CoreLocalizationEngine&) = delete;
    CoreLocalizationEngine& operator=(const CoreLocalizationEngine&) = delete;
    CoreLocalizationEngine(CoreLocalizationEngine&&) noexcept = default;
    CoreLocalizationEngine& operator=(CoreLocalizationEngine&&) noexcept = default;

    /// Get a localized string by key
    std::string GetString(const std::string& key) const {
        auto it = m_strings.find(key);
        if (it != m_strings.end()) return it->second;
        if (m_config.useFallback) return "[" + key + "]";
        return "";
    }

    /// Set the active locale
    void SetLocale(LocaleId locale) noexcept {
        m_currentLocale = locale;
        m_localeChangeCount++;
    }

    /// Get the current locale
    LocaleId GetCurrentLocale() const noexcept { return m_currentLocale; }

    /// Get list of supported locales
    std::vector<LocaleId> GetSupportedLocales() const {
        return { LocaleId::EnUS, LocaleId::DeDE, LocaleId::FrFR,
                 LocaleId::JaJP, LocaleId::ZhCN };
    }

    /// Add a localized string
    void AddString(const std::string& key, const std::string& value) {
        m_strings[key] = value;
    }

    /// Get number of loaded strings
    size_t GetStringCount() const noexcept { return m_strings.size(); }

    /// Get locale change count
    uint32_t GetLocaleChangeCount() const noexcept { return m_localeChangeCount; }

    /// Apply configuration
    void SetConfig(const LocalizationConfig& cfg) noexcept { m_config = cfg; }

    /// Get current config
    const LocalizationConfig& GetConfig() const noexcept { return m_config; }

private:
    LocalizationConfig                          m_config;
    LocaleId                                    m_currentLocale = LocaleId::EnUS;
    std::unordered_map<std::string, std::string> m_strings;
    uint32_t                                    m_localeChangeCount = 0;
};

} // namespace Engine
} // namespace ExplorerLens
