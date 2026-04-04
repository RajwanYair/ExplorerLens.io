#include "LocalizationEngine.h"

namespace ExplorerLens {
namespace Engine {

LocalizationEngine::LocalizationEngine()
{
    m_activeLocale = LocaleInfo::EN_US;
}

const wchar_t* LocalizationEngine::GetLocaleName(LocaleInfo locale)
{
    switch (locale) {
        case LocaleInfo::EN_US:
            return L"English (US)";
        case LocaleInfo::EN_GB:
            return L"English (UK)";
        case LocaleInfo::DE_DE:
            return L"German";
        case LocaleInfo::FR_FR:
            return L"French";
        case LocaleInfo::ES_ES:
            return L"Spanish";
        case LocaleInfo::JA_JP:
            return L"Japanese";
        case LocaleInfo::ZH_CN:
            return L"Chinese Simplified";
        case LocaleInfo::AR_SA:
            return L"Arabic";
        case LocaleInfo::HE_IL:
            return L"Hebrew";
        case LocaleInfo::KO_KR:
            return L"Korean";
        default:
            return L"Unknown";
    }
}

TextDirection LocalizationEngine::GetTextDirection(LocaleInfo locale)
{
    switch (locale) {
        case LocaleInfo::AR_SA:
        case LocaleInfo::HE_IL:
            return TextDirection::RTL;
        default:
            return TextDirection::LTR;
    }
}

void LocalizationEngine::SetLocale(LocaleInfo locale)
{
    m_activeLocale = locale;
}

void LocalizationEngine::AddString(const std::wstring& key, LocaleInfo locale, const std::wstring& value)
{
    m_strings[key][static_cast<uint32_t>(locale)] = value;
}

std::wstring LocalizationEngine::GetString(const std::wstring& key) const
{
    auto it = m_strings.find(key);
    if (it == m_strings.end())
        return key;  // Return key as fallback

    auto localeKey = static_cast<uint32_t>(m_activeLocale);
    auto locIt = it->second.find(localeKey);
    if (locIt != it->second.end())
        return locIt->second;

    // Fallback to EN_US
    auto enIt = it->second.find(static_cast<uint32_t>(LocaleInfo::EN_US));
    if (enIt != it->second.end())
        return enIt->second;

    return key;
}

}  // namespace Engine
}  // namespace ExplorerLens
