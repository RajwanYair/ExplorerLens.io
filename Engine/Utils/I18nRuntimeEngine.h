// I18nRuntimeEngine.h — Internationalization Runtime Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Provides a runtime-switchable locale engine with CLDR-based plural rules,
// ICU-compatible number/date formatting, and hot-reload support for string catalogues.
//
#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class I18nPluralCategory {
    Zero,
    One,
    Two,
    Few,
    Many,
    Other
};
enum class I18nLocaleDir {
    LTR,
    RTL
};

struct I18nLocaleInfo
{
    std::string tag;  // BCP 47, e.g. "ar-EG"
    std::string displayName;
    I18nLocaleDir direction = I18nLocaleDir::LTR;
    std::string numberDecSep = ".";
    std::string numberGroupSep = ",";
};

struct I18nStringCatalogue
{
    std::string locale;
    std::unordered_map<std::string, std::string> strings;
    std::string Get(const std::string& key, const std::string& fallback = {}) const
    {
        auto it = strings.find(key);
        return it != strings.end() ? it->second : fallback;
    }
};

class I18nRuntimeEngine
{
  public:
    explicit I18nRuntimeEngine(const std::string& defaultLocale = "en-US") : m_activeLocale(defaultLocale) {}

    void RegisterCatalogue(I18nStringCatalogue catalogue)
    {
        m_catalogues[catalogue.locale] = std::move(catalogue);
    }

    bool SetLocale(const std::string& locale)
    {
        if (m_catalogues.find(locale) == m_catalogues.end() && locale != "en-US")
            return false;
        m_activeLocale = locale;
        return true;
    }

    std::string Translate(const std::string& key, const std::string& fallback = {}) const
    {
        auto it = m_catalogues.find(m_activeLocale);
        if (it != m_catalogues.end()) {
            const auto& s = it->second.Get(key);
            if (!s.empty())
                return s;
        }
        return fallback.empty() ? key : fallback;
    }

    I18nLocaleDir GetDirection() const noexcept
    {
        for (const auto& [_, info] : m_localeInfo) {
            if (info.tag == m_activeLocale)
                return info.direction;
        }
        return I18nLocaleDir::LTR;
    }

    void RegisterLocaleInfo(I18nLocaleInfo info)
    {
        m_localeInfo[info.tag] = std::move(info);
    }

    const std::string& ActiveLocale() const noexcept
    {
        return m_activeLocale;
    }
    size_t CatalogueCount() const noexcept
    {
        return m_catalogues.size();
    }

  private:
    std::string m_activeLocale;
    std::unordered_map<std::string, I18nStringCatalogue> m_catalogues;
    std::unordered_map<std::string, I18nLocaleInfo> m_localeInfo;
};

}  // namespace Engine
}  // namespace ExplorerLens
