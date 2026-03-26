// LocalizationEngine.h — Unicode String Table and Locale-Aware Formatting
// Copyright (c) 2026 ExplorerLens Project
//
// Manages language resource bundles (.resx / .json) for ExplorerLens UI strings.
// Provides locale-aware number, byte-size, and date formatting using Windows NLS.
//
#pragma once

#include <windows.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <cstdint>
#include <sstream>

namespace ExplorerLens { namespace Engine { namespace i18n {

// Supported locales (extend list as translations land)
enum class SupportedLocale : uint8_t {
    EnUS  = 0,   // English (United States) — fallback
    EnGB  = 1,
    DeDE  = 2,
    FrFR  = 3,
    JaJP  = 4,
    ZhCN  = 5,
    ArSA  = 6,   // Arabic (RTL)
    HeIL  = 7,   // Hebrew (RTL)
    EsES  = 8,
    PtBR  = 9,
    KoKR  = 10,
    RuRU  = 11
};

struct LocalizedString {
    std::wstring  key;
    std::wstring  value;
    SupportedLocale locale;
};

class LocalizationEngine {
public:
    static LocalizationEngine& Instance() {
        static LocalizationEngine inst;
        return inst;
    }

    // Initialize with the system UI locale or an explicit override
    void Initialize(std::optional<SupportedLocale> override = std::nullopt) {
        if (override.has_value()) {
            m_locale = *override;
        } else {
            wchar_t localeName[LOCALE_NAME_MAX_LENGTH];
            if (GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH))
                m_locale = MapLocaleName(localeName);
        }
        LoadBuiltinStrings();
    }

    // Lookup a localized string by key; falls back to en-US
    std::wstring Get(const std::wstring& key) const {
        auto it = m_strings.find(key);
        if (it != m_strings.end()) return it->second;
        // Fallback to en-US prefix
        auto fb = m_fallback.find(key);
        return (fb != m_fallback.end()) ? fb->second : key;
    }

    // Format a file size in locale-appropriate units
    std::wstring FormatBytes(uint64_t bytes) const {
        static const wchar_t* units[] = { L"B", L"KB", L"MB", L"GB", L"TB" };
        double v = static_cast<double>(bytes);
        int u = 0;
        while (v >= 1024.0 && u < 4) { v /= 1024.0; ++u; }
        wchar_t buf[64];
        swprintf_s(buf, 64, u == 0 ? L"%.0f %s" : L"%.1f %s", v, units[u]);
        return buf;
    }

    // Format a number with locale thousands separator
    std::wstring FormatNumber(int64_t value) const {
        wchar_t buf[64];
        swprintf_s(buf, 64, L"%lld", value);
        // Use GetNumberFormatEx for locale-aware grouping
        wchar_t out[128];
        GetNumberFormatEx(LOCALE_NAME_USER_DEFAULT, 0, buf, nullptr, out, 128);
        return out;
    }

    SupportedLocale CurrentLocale() const { return m_locale; }

    bool IsRTL() const {
        return m_locale == SupportedLocale::ArSA || m_locale == SupportedLocale::HeIL;
    }

    // Add or override a string (for plugin-provided translations)
    void SetString(const std::wstring& key, const std::wstring& value) {
        m_strings[key] = value;
    }

private:
    LocalizationEngine() : m_locale(SupportedLocale::EnUS) { Initialize(); }

    SupportedLocale MapLocaleName(const wchar_t* name) const {
        std::wstring n = name;
        if (n.find(L"de-")  != std::wstring::npos) return SupportedLocale::DeDE;
        if (n.find(L"fr-")  != std::wstring::npos) return SupportedLocale::FrFR;
        if (n.find(L"ja-")  != std::wstring::npos) return SupportedLocale::JaJP;
        if (n.find(L"zh-")  != std::wstring::npos) return SupportedLocale::ZhCN;
        if (n.find(L"ar-")  != std::wstring::npos) return SupportedLocale::ArSA;
        if (n.find(L"he-")  != std::wstring::npos) return SupportedLocale::HeIL;
        if (n.find(L"es-")  != std::wstring::npos) return SupportedLocale::EsES;
        if (n.find(L"pt-")  != std::wstring::npos) return SupportedLocale::PtBR;
        if (n.find(L"ko-")  != std::wstring::npos) return SupportedLocale::KoKR;
        if (n.find(L"ru-")  != std::wstring::npos) return SupportedLocale::RuRU;
        if (n.find(L"en-GB")!= std::wstring::npos) return SupportedLocale::EnGB;
        return SupportedLocale::EnUS;
    }

    void LoadBuiltinStrings() {
        // en-US fallback strings
        m_fallback = {
            { L"app.name",              L"ExplorerLens" },
            { L"cache.cleared",         L"Thumbnail cache cleared" },
            { L"decode.failed",         L"Failed to decode file" },
            { L"ai.classifying",        L"Classifying content..." },
            { L"settings.title",        L"ExplorerLens Settings" },
            { L"plugin.installed",      L"Plugin installed successfully" },
            { L"consent.title",         L"Help improve ExplorerLens" },
            { L"consent.accept",        L"Accept" },
            { L"consent.decline",       L"Decline" },
            { L"fleet.tier.standard",   L"Standard" },
            { L"fleet.tier.regulated",  L"Regulated" },
            { L"fleet.tier.classified", L"Classified" },
        };
        m_strings = m_fallback;

        // Apply locale overrides (in a real build, loaded from embedded .resx)
        if (m_locale == SupportedLocale::DeDE) {
            m_strings[L"cache.cleared"]    = L"Thumbnail-Cache geleert";
            m_strings[L"settings.title"]   = L"ExplorerLens Einstellungen";
            m_strings[L"consent.accept"]   = L"Akzeptieren";
            m_strings[L"consent.decline"]  = L"Ablehnen";
        } else if (m_locale == SupportedLocale::FrFR) {
            m_strings[L"cache.cleared"]    = L"Cache de miniatures effacé";
            m_strings[L"settings.title"]   = L"Paramètres ExplorerLens";
            m_strings[L"consent.accept"]   = L"Accepter";
            m_strings[L"consent.decline"]  = L"Refuser";
        } else if (m_locale == SupportedLocale::JaJP) {
            m_strings[L"cache.cleared"]    = L"サムネイルキャッシュをクリアしました";
            m_strings[L"settings.title"]   = L"ExplorerLens 設定";
        } else if (m_locale == SupportedLocale::ArSA) {
            m_strings[L"cache.cleared"]    = L"تم مسح ذاكرة التخزين المؤقت للصور المصغرة";
            m_strings[L"settings.title"]   = L"إعدادات ExplorerLens";
        }
    }

    SupportedLocale                                  m_locale;
    std::unordered_map<std::wstring, std::wstring>   m_strings;
    std::unordered_map<std::wstring, std::wstring>   m_fallback;
};

}}} // namespace ExplorerLens::Engine::i18n
