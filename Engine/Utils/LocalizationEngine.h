// LocalizationEngine.h — i18n/l10n string tables, RTL support, locale detection
// Copyright (c) 2026 ExplorerLens Project
//
// Canonical localization header. Provides:
//   - LocalizationEngine  (wstring-based, RTL support, 10 locales)
//   - CoreLocalizationEngine (string-based, 5 locales, inline, no .cpp needed)
//   - LocaleId, StringCategory enums (narrow-string locale helpers)
// Consolidated from Core/LocalizationEngine.h + Utils/LocalizationEngine.h.
//
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

/// Supported locale identifiers
enum class LocaleInfo : uint32_t {
 EN_US = 0, ///< English (US)
 EN_GB = 1, ///< English (UK)
 DE_DE = 2, ///< German
 FR_FR = 3, ///< French
 ES_ES = 4, ///< Spanish
 JA_JP = 5, ///< Japanese
 ZH_CN = 6, ///< Chinese Simplified
 AR_SA = 7, ///< Arabic (RTL)
 HE_IL = 8, ///< Hebrew (RTL)
 KO_KR = 9, ///< Korean
 COUNT = 10
};

/// Text direction for layout
enum class TextDirection : uint32_t {
 LTR = 0,
 RTL = 1,
 COUNT = 2
};

/// A localized string entry
struct LocalizedString {
 std::wstring key;
 std::wstring value;
 LocaleInfo locale = LocaleInfo::EN_US;
};

/// Manages localized strings and locale settings
class LocalizationEngine {
public:
 LocalizationEngine();

 static const wchar_t* GetLocaleName(LocaleInfo locale);
 static TextDirection GetTextDirection(LocaleInfo locale);
 static uint32_t GetLocaleCount() { return static_cast<uint32_t>(LocaleInfo::COUNT); }

 /// Set the active locale
 void SetLocale(LocaleInfo locale);
 /// Get the active locale
 LocaleInfo GetLocale() const { return m_activeLocale; }

 /// Add a string to the table
 void AddString(const std::wstring& key, LocaleInfo locale, const std::wstring& value);
 /// Retrieve a localized string (falls back to EN_US)
 std::wstring GetString(const std::wstring& key) const;
 /// Get total string count across all locales
 size_t GetStringCount() const { return m_strings.size(); }

 /// Check if current locale is RTL
 bool IsRTL() const { return GetTextDirection(m_activeLocale) == TextDirection::RTL; }

private:
 LocaleInfo m_activeLocale = LocaleInfo::EN_US;
 /// Map: key -> (locale -> value)
 std::unordered_map<std::wstring, std::unordered_map<uint32_t, std::wstring>> m_strings;
};

// ---------------------------------------------------------------------------
// CoreLocalizationEngine — narrow-string (std::string) localization helper.
// Simple inline class for error messages and status text that doesn't need
// full wstring/RTL support. Consolidated here from Core/LocalizationEngine.h.
// ---------------------------------------------------------------------------

/// Supported locale identifiers (narrow-string subset)
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
    UI = 0,
    Error = 1,
    Status = 2,
    Tooltip = 3,
    Accessibility = 4
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

struct LocalizationConfig {
    LocaleId defaultLocale  = LocaleId::EnUS;
    LocaleId fallbackLocale = LocaleId::EnUS;
    bool     useFallback    = true;
};

class CoreLocalizationEngine {
public:
    CoreLocalizationEngine() = default;

    std::string GetString(const std::string& key) const {
        auto it = m_strings.find(key);
        if (it != m_strings.end()) return it->second;
        if (m_config.useFallback) return "[" + key + "]";
        return "";
    }

    void SetLocale(LocaleId locale) noexcept {
        m_currentLocale = locale;
        m_localeChangeCount++;
    }

    LocaleId GetCurrentLocale() const noexcept { return m_currentLocale; }

    std::vector<LocaleId> GetSupportedLocales() const {
        return { LocaleId::EnUS, LocaleId::DeDE, LocaleId::FrFR,
                 LocaleId::JaJP, LocaleId::ZhCN };
    }

    void AddString(const std::string& key, const std::string& value) {
        m_strings[key] = value;
    }

    uint32_t GetLocaleChangeCount() const noexcept { return m_localeChangeCount; }

private:
    LocaleId   m_currentLocale     = LocaleId::EnUS;
    LocalizationConfig m_config;
    uint32_t   m_localeChangeCount = 0;
    std::unordered_map<std::string, std::string> m_strings;
};

}} // namespace ExplorerLens::Engine
