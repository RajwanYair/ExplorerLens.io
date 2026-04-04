// LocaleFallbackResolver.h — Locale Fallback Chain Resolver
// Copyright (c) 2026 ExplorerLens Project
//
// Resolves string keys through a BCP 47 locale fallback chain
// (e.g. zh-Hant-TW → zh-Hant → zh → en), returning the best available translation.
//
#pragma once
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct LocaleFallbackResult
{
    bool found = false;
    std::string resolvedLocale;
    std::string value;
};

class LocaleFallbackResolver
{
  public:
    explicit LocaleFallbackResolver() = default;

    void AddString(const std::string& locale, const std::string& key, const std::string& value)
    {
        m_store[locale][key] = value;
    }

    LocaleFallbackResult Resolve(const std::string& locale, const std::string& key) const
    {
        for (const auto& candidate : BuildChain(locale)) {
            auto locIt = m_store.find(candidate);
            if (locIt == m_store.end())
                continue;
            auto keyIt = locIt->second.find(key);
            if (keyIt != locIt->second.end())
                return {true, candidate, keyIt->second};
        }
        return {false, {}, key};  // return key as fallback
    }

    static std::vector<std::string> BuildChain(const std::string& locale)
    {
        std::vector<std::string> chain;
        chain.push_back(locale);
        // Walk up: zh-Hant-TW → zh-Hant → zh → en-US → en
        std::string cur = locale;
        while (true) {
            auto pos = cur.rfind('-');
            if (pos == std::string::npos)
                break;
            cur = cur.substr(0, pos);
            chain.push_back(cur);
        }
        if (locale != "en")
            chain.push_back("en");
        return chain;
    }

    size_t LocaleCount() const noexcept
    {
        return m_store.size();
    }
    bool HasLocale(const std::string& l) const noexcept
    {
        return m_store.count(l) > 0;
    }

  private:
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_store;
};

}  // namespace Engine
}  // namespace ExplorerLens
