//==============================================================================
// DarkThumbs Engine — Sprint 44: Network & Remote Thumbnail Provider
//
// Provides URL-based thumbnail fetching, network-aware caching with TTL,
// bandwidth throttling, proxy support, retry logic, and remote storage
// integration (SMB, WebDAV, cloud URLs) for generating thumbnails from
// network-hosted images.
//==============================================================================
#pragma once

#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <algorithm>

namespace DarkThumbs::Engine::Cloud {

//==============================================================================
// Network Protocol — Supported remote access methods
//==============================================================================

enum class NetworkProtocol : uint8_t {
    HTTP,
    HTTPS,
    SMB,       // Windows file sharing (\\server\share)
    WebDAV,
    FTP,
    Local      // file:// or direct path
};

inline const char* NetworkProtocolName(NetworkProtocol p) {
    switch (p) {
        case NetworkProtocol::HTTP:   return "HTTP";
        case NetworkProtocol::HTTPS:  return "HTTPS";
        case NetworkProtocol::SMB:    return "SMB";
        case NetworkProtocol::WebDAV: return "WebDAV";
        case NetworkProtocol::FTP:    return "FTP";
        case NetworkProtocol::Local:  return "Local";
        default:                      return "Unknown";
    }
}

inline bool IsSecureProtocol(NetworkProtocol p) {
    return p == NetworkProtocol::HTTPS || p == NetworkProtocol::SMB;
}

//==============================================================================
// Remote URL — Parsed URL with protocol detection
//==============================================================================

struct RemoteURL {
    std::string   rawUrl;
    NetworkProtocol protocol = NetworkProtocol::Local;
    std::string   host;
    uint16_t      port = 0;
    std::string   path;
    std::string   query;

    bool IsRemote() const {
        return protocol != NetworkProtocol::Local;
    }

    bool IsSecure() const {
        return IsSecureProtocol(protocol);
    }

    std::string HostWithPort() const {
        if (port == 0) return host;
        return host + ":" + std::to_string(port);
    }

    static NetworkProtocol DetectProtocol(const std::string& url) {
        if (url.empty()) return NetworkProtocol::Local;
        if (url.substr(0, 8) == "https://") return NetworkProtocol::HTTPS;
        if (url.substr(0, 7) == "http://")  return NetworkProtocol::HTTP;
        if (url.substr(0, 6) == "ftp://")   return NetworkProtocol::FTP;
        if (url.substr(0, 2) == "\\\\" || url.substr(0, 2) == "//")
            return NetworkProtocol::SMB;
        if (url.find("webdav") != std::string::npos ||
            url.find("dav://") != std::string::npos)
            return NetworkProtocol::WebDAV;
        return NetworkProtocol::Local;
    }

    static RemoteURL Parse(const std::string& url) {
        RemoteURL r;
        r.rawUrl = url;
        r.protocol = DetectProtocol(url);

        // Simple host extraction for HTTP/HTTPS
        if (r.protocol == NetworkProtocol::HTTP || r.protocol == NetworkProtocol::HTTPS) {
            size_t start = url.find("://");
            if (start != std::string::npos) {
                start += 3;
                size_t pathStart = url.find('/', start);
                if (pathStart != std::string::npos) {
                    r.host = url.substr(start, pathStart - start);
                    r.path = url.substr(pathStart);
                } else {
                    r.host = url.substr(start);
                    r.path = "/";
                }
                // Extract port if present
                size_t colonPos = r.host.find(':');
                if (colonPos != std::string::npos) {
                    r.port = static_cast<uint16_t>(
                        std::stoi(r.host.substr(colonPos + 1)));
                    r.host = r.host.substr(0, colonPos);
                }
            }
        }

        // SMB path: \\server\share\path
        if (r.protocol == NetworkProtocol::SMB && url.size() > 2) {
            auto path = url.substr(2);
            auto sep = path.find('\\');
            if (sep == std::string::npos) sep = path.find('/');
            if (sep != std::string::npos) {
                r.host = path.substr(0, sep);
                r.path = path.substr(sep);
            } else {
                r.host = path;
            }
        }

        return r;
    }
};

//==============================================================================
// Network Request Status
//==============================================================================

enum class RequestStatus : uint8_t {
    Pending,
    Connecting,
    Downloading,
    Completed,
    Failed,
    TimedOut,
    Cancelled
};

inline const char* RequestStatusName(RequestStatus s) {
    switch (s) {
        case RequestStatus::Pending:     return "Pending";
        case RequestStatus::Connecting:  return "Connecting";
        case RequestStatus::Downloading: return "Downloading";
        case RequestStatus::Completed:   return "Completed";
        case RequestStatus::Failed:      return "Failed";
        case RequestStatus::TimedOut:    return "Timed Out";
        case RequestStatus::Cancelled:   return "Cancelled";
        default:                         return "Unknown";
    }
}

//==============================================================================
// Download Progress — Transfer progress tracking
//==============================================================================

struct DownloadProgress {
    uint64_t totalBytes     = 0;
    uint64_t receivedBytes  = 0;
    double   elapsedMs      = 0.0;

    double PercentComplete() const {
        if (totalBytes == 0) return 0.0;
        return (static_cast<double>(receivedBytes) / static_cast<double>(totalBytes)) * 100.0;
    }

    double SpeedBytesPerSecond() const {
        if (elapsedMs <= 0.0) return 0.0;
        return (static_cast<double>(receivedBytes) / elapsedMs) * 1000.0;
    }

    std::string SpeedHuman() const {
        double bps = SpeedBytesPerSecond();
        if (bps >= 1024.0 * 1024.0) {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(1) << (bps / (1024.0 * 1024.0)) << " MB/s";
            return ss.str();
        }
        if (bps >= 1024.0) {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(1) << (bps / 1024.0) << " KB/s";
            return ss.str();
        }
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(0) << bps << " B/s";
        return ss.str();
    }

    bool IsComplete() const {
        return totalBytes > 0 && receivedBytes >= totalBytes;
    }
};

//==============================================================================
// Network Cache Entry — Cached remote resource with TTL
//==============================================================================

struct NetworkCacheEntry {
    std::string url;
    std::string localPath;       // Cached file on disk
    std::string etag;            // HTTP ETag for conditional requests
    std::string contentType;
    uint64_t    contentSize = 0;
    int64_t     fetchedAt   = 0; // Unix timestamp ms
    int64_t     expiresAt   = 0; // Unix timestamp ms
    int         httpStatus  = 0;

    bool IsExpired(int64_t nowMs) const {
        if (expiresAt <= 0) return true;
        return nowMs > expiresAt;
    }

    bool HasETag() const {
        return !etag.empty();
    }

    bool IsValid() const {
        return !url.empty() && !localPath.empty() && httpStatus >= 200 && httpStatus < 400;
    }
};

//==============================================================================
// Network Cache — TTL-based cache for remote resources
//==============================================================================

class NetworkCache {
public:
    void Put(const NetworkCacheEntry& entry) {
        m_entries[entry.url] = entry;
    }

    const NetworkCacheEntry* Get(const std::string& url) const {
        auto it = m_entries.find(url);
        if (it == m_entries.end()) return nullptr;
        return &it->second;
    }

    bool Has(const std::string& url) const {
        return m_entries.count(url) > 0;
    }

    bool HasValid(const std::string& url, int64_t nowMs) const {
        auto entry = Get(url);
        if (!entry) return false;
        return entry->IsValid() && !entry->IsExpired(nowMs);
    }

    size_t Size() const { return m_entries.size(); }

    size_t Evict(int64_t nowMs) {
        size_t evicted = 0;
        for (auto it = m_entries.begin(); it != m_entries.end();) {
            if (it->second.IsExpired(nowMs)) {
                it = m_entries.erase(it);
                evicted++;
            } else {
                ++it;
            }
        }
        return evicted;
    }

    void Clear() { m_entries.clear(); }

private:
    std::unordered_map<std::string, NetworkCacheEntry> m_entries;
};

//==============================================================================
// Proxy Config — Corporate proxy settings
//==============================================================================

struct ProxyConfig {
    std::string proxyUrl;
    uint16_t    proxyPort     = 0;
    std::string proxyUser;
    std::string proxyPassword;
    bool        useSystemProxy = true;
    bool        bypassForLocal = true;
    std::vector<std::string> bypassList; // Hostnames that skip proxy

    bool IsConfigured() const {
        return useSystemProxy || !proxyUrl.empty();
    }

    bool ShouldBypass(const std::string& host) const {
        if (bypassForLocal && (host == "localhost" || host == "127.0.0.1"))
            return true;
        for (const auto& bypass : bypassList) {
            if (host == bypass) return true;
            // Simple wildcard suffix match
            if (!bypass.empty() && bypass[0] == '*' && host.size() >= bypass.size() - 1) {
                auto suffix = bypass.substr(1);
                if (host.size() >= suffix.size() &&
                    host.compare(host.size() - suffix.size(), suffix.size(), suffix) == 0)
                    return true;
            }
        }
        return false;
    }

    static ProxyConfig SystemDefault() {
        ProxyConfig c;
        c.useSystemProxy = true;
        c.bypassForLocal = true;
        return c;
    }

    static ProxyConfig Direct() {
        ProxyConfig c;
        c.useSystemProxy = false;
        c.bypassForLocal = false;
        return c;
    }

    static ProxyConfig Corporate(const std::string& url, uint16_t port) {
        ProxyConfig c;
        c.proxyUrl = url;
        c.proxyPort = port;
        c.useSystemProxy = false;
        c.bypassForLocal = true;
        c.bypassList = {"*.local", "*.internal"};
        return c;
    }
};

//==============================================================================
// Retry Policy — Configurable retry behavior for transient failures
//==============================================================================

struct RetryPolicy {
    uint32_t maxRetries       = 3;
    uint32_t initialDelayMs   = 500;
    double   backoffMultiplier = 2.0;
    uint32_t maxDelayMs       = 30000;

    uint32_t DelayForAttempt(uint32_t attempt) const {
        if (attempt == 0) return 0;
        double delay = static_cast<double>(initialDelayMs);
        for (uint32_t i = 1; i < attempt && i < maxRetries; i++) {
            delay *= backoffMultiplier;
        }
        return std::min(static_cast<uint32_t>(delay), maxDelayMs);
    }

    bool ShouldRetry(uint32_t attempt) const {
        return attempt < maxRetries;
    }

    static RetryPolicy Default()     { return {}; }
    static RetryPolicy Aggressive()  { return {5, 200, 1.5, 60000}; }
    static RetryPolicy NoRetry()     { return {0, 0, 1.0, 0}; }
};

//==============================================================================
// Bandwidth Throttle — Limit download speed
//==============================================================================

struct BandwidthThrottle {
    uint64_t maxBytesPerSecond = 0; // 0 = unlimited
    uint64_t maxTotalBytes     = 0; // 0 = unlimited

    bool IsThrottled() const { return maxBytesPerSecond > 0; }
    bool HasQuota()    const { return maxTotalBytes > 0; }

    bool WithinQuota(uint64_t currentBytes) const {
        if (!HasQuota()) return true;
        return currentBytes < maxTotalBytes;
    }

    static BandwidthThrottle Unlimited() { return {}; }
    static BandwidthThrottle Metered()   { return {512 * 1024, 100 * 1024 * 1024}; }
    static BandwidthThrottle Low()       { return {128 * 1024, 50 * 1024 * 1024}; }

    std::string LimitHuman() const {
        if (!IsThrottled()) return "Unlimited";
        if (maxBytesPerSecond >= 1024 * 1024) {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(1)
               << (static_cast<double>(maxBytesPerSecond) / (1024.0 * 1024.0)) << " MB/s";
            return ss.str();
        }
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(0)
           << (static_cast<double>(maxBytesPerSecond) / 1024.0) << " KB/s";
        return ss.str();
    }
};

//==============================================================================
// Network Thumbnail Request — Remote image decode request
//==============================================================================

struct NetworkThumbnailRequest {
    RemoteURL     url;
    uint32_t      targetWidth   = 256;
    uint32_t      targetHeight  = 256;
    RequestStatus status        = RequestStatus::Pending;
    uint32_t      attemptCount  = 0;
    std::string   errorMessage;
    DownloadProgress progress;

    bool IsComplete() const {
        return status == RequestStatus::Completed ||
               status == RequestStatus::Failed ||
               status == RequestStatus::Cancelled;
    }
};

//==============================================================================
// Network Config — Overall network settings
//==============================================================================

struct NetworkConfig {
    ProxyConfig       proxy;
    RetryPolicy       retry;
    BandwidthThrottle bandwidth;
    uint32_t          connectTimeoutMs = 10000;
    uint32_t          readTimeoutMs    = 30000;
    int64_t           cacheTTLMs       = 3600000; // 1 hour default
    bool              enableCache      = true;
    bool              enableRemote     = true;

    static NetworkConfig Default() {
        return {};
    }

    static NetworkConfig OfflineOnly() {
        NetworkConfig c;
        c.enableRemote = false;
        c.enableCache  = true;
        return c;
    }

    static NetworkConfig MeteredConnection() {
        NetworkConfig c;
        c.bandwidth = BandwidthThrottle::Metered();
        c.retry.maxRetries = 1;
        c.cacheTTLMs = 86400000; // 24 hours
        return c;
    }

    static NetworkConfig Corporate() {
        NetworkConfig c;
        c.proxy = ProxyConfig::SystemDefault();
        c.retry = RetryPolicy::Default();
        c.cacheTTLMs = 7200000; // 2 hours
        return c;
    }
};

} // namespace DarkThumbs::Engine::Cloud
