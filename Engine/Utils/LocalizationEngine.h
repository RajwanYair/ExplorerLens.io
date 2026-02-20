#pragma once
// Sprint 228: Localization Engine — i18n/l10n string tables, RTL support, locale detection
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

/// Supported locale identifiers
enum class Locale : uint32_t {
    EN_US = 0,    ///< English (US)
    EN_GB = 1,    ///< English (UK)
    DE_DE = 2,    ///< German
    FR_FR = 3,    ///< French
    ES_ES = 4,    ///< Spanish
    JA_JP = 5,    ///< Japanese
    ZH_CN = 6,    ///< Chinese Simplified
    AR_SA = 7,    ///< Arabic (RTL)
    HE_IL = 8,    ///< Hebrew (RTL)
    KO_KR = 9,    ///< Korean
    COUNT = 10
};

/// Text direction for layout
enum class TextDirection : uint32_t {
    LTR   = 0,
    RTL   = 1,
    COUNT = 2
};

/// A localized string entry
struct LocalizedString {
    std::wstring key;
    std::wstring value;
    Locale       locale = Locale::EN_US;
};

/// Manages localized strings and locale settings
class LocalizationEngine {
public:
    LocalizationEngine();

    static const wchar_t* GetLocaleName(Locale locale);
    static TextDirection GetTextDirection(Locale locale);
    static uint32_t GetLocaleCount() { return static_cast<uint32_t>(Locale::COUNT); }

    /// Set the active locale
    void SetLocale(Locale locale);
    /// Get the active locale
    Locale GetLocale() const { return m_activeLocale; }

    /// Add a string to the table
    void AddString(const std::wstring& key, Locale locale, const std::wstring& value);
    /// Retrieve a localized string (falls back to EN_US)
    std::wstring GetString(const std::wstring& key) const;
    /// Get total string count across all locales
    size_t GetStringCount() const { return m_strings.size(); }

    /// Check if current locale is RTL
    bool IsRTL() const { return GetTextDirection(m_activeLocale) == TextDirection::RTL; }

private:
    Locale m_activeLocale = Locale::EN_US;
    /// Map: key -> (locale -> value)
    std::unordered_map<std::wstring, std::unordered_map<uint32_t, std::wstring>> m_strings;
};

}} // namespace DarkThumbs::Engine
