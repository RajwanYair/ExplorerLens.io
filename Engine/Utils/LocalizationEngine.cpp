#include "LocalizationEngine.h"

namespace ExplorerLens { namespace Engine {

LocalizationEngine::LocalizationEngine() {
    m_activeLocale = Locale::EN_US;
}

const wchar_t* LocalizationEngine::GetLocaleName(Locale locale) {
    switch (locale) {
        case Locale::EN_US: return L"English (US)";
        case Locale::EN_GB: return L"English (UK)";
        case Locale::DE_DE: return L"German";
        case Locale::FR_FR: return L"French";
        case Locale::ES_ES: return L"Spanish";
        case Locale::JA_JP: return L"Japanese";
        case Locale::ZH_CN: return L"Chinese Simplified";
        case Locale::AR_SA: return L"Arabic";
        case Locale::HE_IL: return L"Hebrew";
        case Locale::KO_KR: return L"Korean";
        default:            return L"Unknown";
    }
}

TextDirection LocalizationEngine::GetTextDirection(Locale locale) {
    switch (locale) {
        case Locale::AR_SA:
        case Locale::HE_IL:
            return TextDirection::RTL;
        default:
            return TextDirection::LTR;
    }
}

void LocalizationEngine::SetLocale(Locale locale) {
    m_activeLocale = locale;
}

void LocalizationEngine::AddString(const std::wstring& key, Locale locale, const std::wstring& value) {
    m_strings[key][static_cast<uint32_t>(locale)] = value;
}

std::wstring LocalizationEngine::GetString(const std::wstring& key) const {
    auto it = m_strings.find(key);
    if (it == m_strings.end()) return key; // Return key as fallback

    auto localeKey = static_cast<uint32_t>(m_activeLocale);
    auto locIt = it->second.find(localeKey);
    if (locIt != it->second.end()) return locIt->second;

    // Fallback to EN_US
    auto enIt = it->second.find(static_cast<uint32_t>(Locale::EN_US));
    if (enIt != it->second.end()) return enIt->second;

    return key;
}

}} // namespace ExplorerLens::Engine

