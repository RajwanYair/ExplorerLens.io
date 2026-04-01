// MarketplaceClientV4.h — Plugin Marketplace Client v4
// Copyright (c) 2026 ExplorerLens Project
//
// REST + gRPC dual-protocol marketplace client with offline cache support.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>
#include <optional>

namespace ExplorerLens::Engine {

enum class MarketplaceProtocol : uint8_t {
    REST    = 0,
    gRPC    = 1,
    Hybrid  = 2,
};

enum class CachePolicy : uint8_t {
    NoCache     = 0,
    MemOnly     = 1,
    Persistent  = 2,
    Aggressive  = 3,
};

struct MarketplaceListing {
    std::string id;
    std::string name;
    std::string version;
    std::string author;
    std::string description;
    float       rating        = 0.0f;
    uint64_t    downloadCount = 0;
    float       price         = 0.0f;
    bool        isFree        = true;

    [[nodiscard]] bool IsValid() const noexcept {
        return !id.empty() && !name.empty() && !version.empty();
    }
};

struct SearchQuery {
    std::string              keyword;
    std::string              category;
    float                    minRating   = 0.0f;
    bool                     freeOnly    = false;
    uint32_t                 maxResults  = 50;
};

class MarketplaceClientV4 {
public:
    explicit MarketplaceClientV4(MarketplaceProtocol protocol = MarketplaceProtocol::Hybrid);
    ~MarketplaceClientV4() noexcept;

    MarketplaceClientV4(const MarketplaceClientV4&)            = delete;
    MarketplaceClientV4& operator=(const MarketplaceClientV4&) = delete;
    MarketplaceClientV4(MarketplaceClientV4&&)                 = default;
    MarketplaceClientV4& operator=(MarketplaceClientV4&&)      = default;

    // Search the marketplace catalog.
    [[nodiscard]] std::vector<MarketplaceListing> Search(const SearchQuery& query) const;

    // Retrieve full metadata for a single plugin.
    [[nodiscard]] std::optional<MarketplaceListing> GetListing(const std::string& pluginId) const;

    // Download plugin archive; returns local file path on success.
    [[nodiscard]] std::string Download(
        const std::string& pluginId,
        const std::string& destDirectory,
        std::function<void(uint64_t, uint64_t)> progressCallback = nullptr) const;

    // Return all top-level category names.
    [[nodiscard]] std::vector<std::string> GetCategories() const;

    void SetCachePolicy(CachePolicy policy) noexcept;
    void SetBaseUrl(const std::string& url);

    [[nodiscard]] CachePolicy         GetCachePolicy()  const noexcept { return m_cachePolicy; }
    [[nodiscard]] MarketplaceProtocol GetProtocol()     const noexcept { return m_protocol; }
    [[nodiscard]] const std::string&  GetBaseUrl()      const noexcept { return m_baseUrl; }

    // Invalidate the in-memory and on-disk cache.
    void InvalidateCache() noexcept;

    // Returns the age of the most recent cached response.
    [[nodiscard]] std::chrono::seconds CacheAge() const noexcept;

private:
    MarketplaceProtocol              m_protocol;
    CachePolicy                      m_cachePolicy  = CachePolicy::Persistent;
    std::string                      m_baseUrl      = "https://plugins.explorerlens.io/api/v4";
    std::chrono::steady_clock::time_point m_lastFetch{};

    struct Impl {};
    std::unique_ptr<Impl>            m_impl;

    [[nodiscard]] std::vector<MarketplaceListing> FetchFromCache(const SearchQuery& query) const;
    bool StoreToCache(const std::string& key, const std::vector<MarketplaceListing>& results) const;
};

} // namespace ExplorerLens::Engine
