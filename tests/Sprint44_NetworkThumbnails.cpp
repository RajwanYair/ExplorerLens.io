//==============================================================================
// ExplorerLens — Sprint 44 Tests: Network & Remote Thumbnail Provider
// Tests URL parsing, protocol detection, download progress, network cache,
// proxy config, retry policy, bandwidth throttle, network config.
//==============================================================================

#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "../Engine/Cloud/NetworkThumbnailProvider.h"

using namespace ExplorerLens::Engine::Cloud;

//==============================================================================
// Network Protocol Tests
//==============================================================================

TEST(NetworkProtocol, Names)
{
    EXPECT_STREQ(NetworkProtocolName(NetworkProtocol::HTTP), "HTTP");
    EXPECT_STREQ(NetworkProtocolName(NetworkProtocol::HTTPS), "HTTPS");
    EXPECT_STREQ(NetworkProtocolName(NetworkProtocol::SMB), "SMB");
    EXPECT_STREQ(NetworkProtocolName(NetworkProtocol::WebDAV), "WebDAV");
    EXPECT_STREQ(NetworkProtocolName(NetworkProtocol::FTP), "FTP");
    EXPECT_STREQ(NetworkProtocolName(NetworkProtocol::Local), "Local");
}

TEST(NetworkProtocol, SecureProtocols)
{
    EXPECT_TRUE(IsSecureProtocol(NetworkProtocol::HTTPS));
    EXPECT_TRUE(IsSecureProtocol(NetworkProtocol::SMB));
    EXPECT_FALSE(IsSecureProtocol(NetworkProtocol::HTTP));
    EXPECT_FALSE(IsSecureProtocol(NetworkProtocol::FTP));
}

//==============================================================================
// Remote URL Tests
//==============================================================================

TEST(RemoteURL, DetectHTTPS)
{
    auto p = RemoteURL::DetectProtocol("https://example.com/image.jpg");
    EXPECT_EQ(p, NetworkProtocol::HTTPS);
}

TEST(RemoteURL, DetectHTTP)
{
    auto p = RemoteURL::DetectProtocol("http://example.com/image.jpg");
    EXPECT_EQ(p, NetworkProtocol::HTTP);
}

TEST(RemoteURL, DetectSMB)
{
    auto p = RemoteURL::DetectProtocol("\\\\server\\share\\file.jpg");
    EXPECT_EQ(p, NetworkProtocol::SMB);
}

TEST(RemoteURL, DetectFTP)
{
    auto p = RemoteURL::DetectProtocol("ftp://files.example.com/img.png");
    EXPECT_EQ(p, NetworkProtocol::FTP);
}

TEST(RemoteURL, DetectLocal)
{
    auto p = RemoteURL::DetectProtocol("C:\\Users\\test\\image.jpg");
    EXPECT_EQ(p, NetworkProtocol::Local);
}

TEST(RemoteURL, DetectEmpty)
{
    auto p = RemoteURL::DetectProtocol("");
    EXPECT_EQ(p, NetworkProtocol::Local);
}

TEST(RemoteURL, ParseHTTPS)
{
    auto url = RemoteURL::Parse("https://cdn.example.com/images/photo.jpg");
    EXPECT_EQ(url.protocol, NetworkProtocol::HTTPS);
    EXPECT_EQ(url.host, "cdn.example.com");
    EXPECT_EQ(url.path, "/images/photo.jpg");
    EXPECT_TRUE(url.IsRemote());
    EXPECT_TRUE(url.IsSecure());
}

TEST(RemoteURL, ParseHTTPWithPort)
{
    auto url = RemoteURL::Parse("http://localhost:8080/thumb.png");
    EXPECT_EQ(url.protocol, NetworkProtocol::HTTP);
    EXPECT_EQ(url.host, "localhost");
    EXPECT_EQ(url.port, 8080);
    EXPECT_EQ(url.path, "/thumb.png");
    EXPECT_EQ(url.HostWithPort(), "localhost:8080");
}

TEST(RemoteURL, ParseSMB)
{
    auto url = RemoteURL::Parse("\\\\fileserver\\photos\\album1\\cover.jpg");
    EXPECT_EQ(url.protocol, NetworkProtocol::SMB);
    EXPECT_EQ(url.host, "fileserver");
    EXPECT_TRUE(url.IsRemote());
}

TEST(RemoteURL, LocalIsNotRemote)
{
    auto url = RemoteURL::Parse("C:\\local\\file.jpg");
    EXPECT_FALSE(url.IsRemote());
    EXPECT_FALSE(url.IsSecure());
}

TEST(RemoteURL, HostWithPortNoPort)
{
    RemoteURL u;
    u.host = "example.com";
    u.port = 0;
    EXPECT_EQ(u.HostWithPort(), "example.com");
}

//==============================================================================
// Request Status Tests
//==============================================================================

TEST(RequestStatus, Names)
{
    EXPECT_STREQ(RequestStatusName(RequestStatus::Pending), "Pending");
    EXPECT_STREQ(RequestStatusName(RequestStatus::Connecting), "Connecting");
    EXPECT_STREQ(RequestStatusName(RequestStatus::Downloading), "Downloading");
    EXPECT_STREQ(RequestStatusName(RequestStatus::Completed), "Completed");
    EXPECT_STREQ(RequestStatusName(RequestStatus::Failed), "Failed");
    EXPECT_STREQ(RequestStatusName(RequestStatus::TimedOut), "Timed Out");
    EXPECT_STREQ(RequestStatusName(RequestStatus::Cancelled), "Cancelled");
}

//==============================================================================
// Download Progress Tests
//==============================================================================

TEST(DownloadProgress, Zero)
{
    DownloadProgress p;
    EXPECT_DOUBLE_EQ(p.PercentComplete(), 0.0);
    EXPECT_DOUBLE_EQ(p.SpeedBytesPerSecond(), 0.0);
    EXPECT_FALSE(p.IsComplete());
}

TEST(DownloadProgress, HalfDone)
{
    DownloadProgress p;
    p.totalBytes = 1000;
    p.receivedBytes = 500;
    EXPECT_DOUBLE_EQ(p.PercentComplete(), 50.0);
    EXPECT_FALSE(p.IsComplete());
}

TEST(DownloadProgress, Complete)
{
    DownloadProgress p;
    p.totalBytes = 1000;
    p.receivedBytes = 1000;
    EXPECT_DOUBLE_EQ(p.PercentComplete(), 100.0);
    EXPECT_TRUE(p.IsComplete());
}

TEST(DownloadProgress, SpeedMBps)
{
    DownloadProgress p;
    p.receivedBytes = 10 * 1024 * 1024; // 10 MB
    p.elapsedMs = 1000.0;               // 1 second
    auto speed = p.SpeedHuman();
    EXPECT_NE(speed.find("MB/s"), std::string::npos);
}

TEST(DownloadProgress, SpeedKBps)
{
    DownloadProgress p;
    p.receivedBytes = 50 * 1024; // 50 KB
    p.elapsedMs = 1000.0;
    auto speed = p.SpeedHuman();
    EXPECT_NE(speed.find("KB/s"), std::string::npos);
}

TEST(DownloadProgress, SpeedBps)
{
    DownloadProgress p;
    p.receivedBytes = 500;
    p.elapsedMs = 1000.0;
    auto speed = p.SpeedHuman();
    EXPECT_NE(speed.find("B/s"), std::string::npos);
}

//==============================================================================
// Network Cache Entry Tests
//==============================================================================

TEST(NetworkCacheEntry, NotExpired)
{
    NetworkCacheEntry e;
    e.expiresAt = 2000;
    EXPECT_FALSE(e.IsExpired(1000));
}

TEST(NetworkCacheEntry, Expired)
{
    NetworkCacheEntry e;
    e.expiresAt = 1000;
    EXPECT_TRUE(e.IsExpired(2000));
}

TEST(NetworkCacheEntry, Valid)
{
    NetworkCacheEntry e;
    e.url = "https://example.com/img.jpg";
    e.localPath = "C:\\cache\\abc123.jpg";
    e.httpStatus = 200;
    EXPECT_TRUE(e.IsValid());
}

TEST(NetworkCacheEntry, InvalidNoPath)
{
    NetworkCacheEntry e;
    e.url = "https://example.com/img.jpg";
    e.httpStatus = 200;
    EXPECT_FALSE(e.IsValid());
}

TEST(NetworkCacheEntry, InvalidStatus)
{
    NetworkCacheEntry e;
    e.url = "https://example.com/img.jpg";
    e.localPath = "cache.jpg";
    e.httpStatus = 404;
    EXPECT_FALSE(e.IsValid());
}

TEST(NetworkCacheEntry, HasETag)
{
    NetworkCacheEntry e;
    EXPECT_FALSE(e.HasETag());
    e.etag = "\"abc123\"";
    EXPECT_TRUE(e.HasETag());
}

//==============================================================================
// Network Cache Tests
//==============================================================================

TEST(NetworkCache, Empty)
{
    NetworkCache c;
    EXPECT_EQ(c.Size(), 0u);
    EXPECT_FALSE(c.Has("anything"));
    EXPECT_EQ(c.Get("nothing"), nullptr);
}

TEST(NetworkCache, PutAndGet)
{
    NetworkCache c;
    NetworkCacheEntry e;
    e.url = "https://example.com/img.jpg";
    e.localPath = "cache.jpg";
    e.httpStatus = 200;
    c.Put(e);
    EXPECT_EQ(c.Size(), 1u);
    EXPECT_TRUE(c.Has("https://example.com/img.jpg"));
    auto* found = c.Get("https://example.com/img.jpg");
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->localPath, "cache.jpg");
}

TEST(NetworkCache, HasValid)
{
    NetworkCache c;
    NetworkCacheEntry e;
    e.url = "https://example.com/img.jpg";
    e.localPath = "cache.jpg";
    e.httpStatus = 200;
    e.expiresAt = 5000;
    c.Put(e);
    EXPECT_TRUE(c.HasValid("https://example.com/img.jpg", 1000));
    EXPECT_FALSE(c.HasValid("https://example.com/img.jpg", 6000));
}

TEST(NetworkCache, Evict)
{
    NetworkCache c;

    NetworkCacheEntry e1;
    e1.url = "url1"; e1.localPath = "1"; e1.httpStatus = 200; e1.expiresAt = 1000;
    c.Put(e1);

    NetworkCacheEntry e2;
    e2.url = "url2"; e2.localPath = "2"; e2.httpStatus = 200; e2.expiresAt = 5000;
    c.Put(e2);

    auto evicted = c.Evict(2000);
    EXPECT_EQ(evicted, 1u);
    EXPECT_EQ(c.Size(), 1u);
    EXPECT_FALSE(c.Has("url1"));
    EXPECT_TRUE(c.Has("url2"));
}

TEST(NetworkCache, Clear)
{
    NetworkCache c;
    NetworkCacheEntry e;
    e.url = "url1"; e.localPath = "p"; e.httpStatus = 200;
    c.Put(e);
    c.Clear();
    EXPECT_EQ(c.Size(), 0u);
}

//==============================================================================
// Proxy Config Tests
//==============================================================================

TEST(ProxyConfig, SystemDefault)
{
    auto p = ProxyConfig::SystemDefault();
    EXPECT_TRUE(p.useSystemProxy);
    EXPECT_TRUE(p.bypassForLocal);
    EXPECT_TRUE(p.IsConfigured());
}

TEST(ProxyConfig, Direct)
{
    auto p = ProxyConfig::Direct();
    EXPECT_FALSE(p.useSystemProxy);
    EXPECT_FALSE(p.IsConfigured());
}

TEST(ProxyConfig, Corporate)
{
    auto p = ProxyConfig::Corporate("proxy.corp.com", 8080);
    EXPECT_EQ(p.proxyUrl, "proxy.corp.com");
    EXPECT_EQ(p.proxyPort, 8080);
    EXPECT_TRUE(p.IsConfigured());
}

TEST(ProxyConfig, BypassLocalhost)
{
    auto p = ProxyConfig::SystemDefault();
    EXPECT_TRUE(p.ShouldBypass("localhost"));
    EXPECT_TRUE(p.ShouldBypass("127.0.0.1"));
    EXPECT_FALSE(p.ShouldBypass("example.com"));
}

TEST(ProxyConfig, BypassWildcard)
{
    auto p = ProxyConfig::Corporate("proxy", 80);
    EXPECT_TRUE(p.ShouldBypass("server.internal"));
    EXPECT_TRUE(p.ShouldBypass("host.local"));
    EXPECT_FALSE(p.ShouldBypass("example.com"));
}

//==============================================================================
// Retry Policy Tests
//==============================================================================

TEST(RetryPolicy, Default)
{
    auto r = RetryPolicy::Default();
    EXPECT_EQ(r.maxRetries, 3u);
    EXPECT_TRUE(r.ShouldRetry(0));
    EXPECT_TRUE(r.ShouldRetry(2));
    EXPECT_FALSE(r.ShouldRetry(3));
}

TEST(RetryPolicy, NoRetry)
{
    auto r = RetryPolicy::NoRetry();
    EXPECT_FALSE(r.ShouldRetry(0));
}

TEST(RetryPolicy, ExponentialBackoff)
{
    auto r = RetryPolicy::Default();
    EXPECT_EQ(r.DelayForAttempt(0), 0u);
    EXPECT_EQ(r.DelayForAttempt(1), 500u);   // initial
    EXPECT_EQ(r.DelayForAttempt(2), 1000u);  // 500 * 2
}

TEST(RetryPolicy, MaxDelayCap)
{
    RetryPolicy r;
    r.initialDelayMs = 10000;
    r.backoffMultiplier = 10.0;
    r.maxRetries = 5;
    r.maxDelayMs = 30000;
    auto delay = r.DelayForAttempt(4);
    EXPECT_LE(delay, 30000u);
}

//==============================================================================
// Bandwidth Throttle Tests
//==============================================================================

TEST(BandwidthThrottle, Unlimited)
{
    auto b = BandwidthThrottle::Unlimited();
    EXPECT_FALSE(b.IsThrottled());
    EXPECT_FALSE(b.HasQuota());
    EXPECT_TRUE(b.WithinQuota(999999999));
    EXPECT_EQ(b.LimitHuman(), "Unlimited");
}

TEST(BandwidthThrottle, Metered)
{
    auto b = BandwidthThrottle::Metered();
    EXPECT_TRUE(b.IsThrottled());
    EXPECT_TRUE(b.HasQuota());
    EXPECT_TRUE(b.WithinQuota(0));
    EXPECT_FALSE(b.WithinQuota(200 * 1024 * 1024));
}

TEST(BandwidthThrottle, LimitHumanKB)
{
    BandwidthThrottle b;
    b.maxBytesPerSecond = 128 * 1024;
    auto s = b.LimitHuman();
    EXPECT_NE(s.find("KB/s"), std::string::npos);
}

TEST(BandwidthThrottle, LimitHumanMB)
{
    BandwidthThrottle b;
    b.maxBytesPerSecond = 5 * 1024 * 1024;
    auto s = b.LimitHuman();
    EXPECT_NE(s.find("MB/s"), std::string::npos);
}

//==============================================================================
// Network Thumbnail Request Tests
//==============================================================================

TEST(NetworkThumbnailRequest, DefaultPending)
{
    NetworkThumbnailRequest req;
    EXPECT_EQ(req.status, RequestStatus::Pending);
    EXPECT_EQ(req.attemptCount, 0u);
    EXPECT_FALSE(req.IsComplete());
}

TEST(NetworkThumbnailRequest, Completed)
{
    NetworkThumbnailRequest req;
    req.status = RequestStatus::Completed;
    EXPECT_TRUE(req.IsComplete());
}

TEST(NetworkThumbnailRequest, Failed)
{
    NetworkThumbnailRequest req;
    req.status = RequestStatus::Failed;
    req.errorMessage = "Connection refused";
    EXPECT_TRUE(req.IsComplete());
}

//==============================================================================
// Network Config Tests
//==============================================================================

TEST(NetworkConfig, Default)
{
    auto c = NetworkConfig::Default();
    EXPECT_TRUE(c.enableRemote);
    EXPECT_TRUE(c.enableCache);
    EXPECT_EQ(c.connectTimeoutMs, 10000u);
    EXPECT_EQ(c.cacheTTLMs, 3600000);
}

TEST(NetworkConfig, OfflineOnly)
{
    auto c = NetworkConfig::OfflineOnly();
    EXPECT_FALSE(c.enableRemote);
    EXPECT_TRUE(c.enableCache);
}

TEST(NetworkConfig, MeteredConnection)
{
    auto c = NetworkConfig::MeteredConnection();
    EXPECT_TRUE(c.bandwidth.IsThrottled());
    EXPECT_EQ(c.retry.maxRetries, 1u);
    EXPECT_EQ(c.cacheTTLMs, 86400000); // 24 hours
}

TEST(NetworkConfig, Corporate)
{
    auto c = NetworkConfig::Corporate();
    EXPECT_TRUE(c.proxy.IsConfigured());
    EXPECT_EQ(c.cacheTTLMs, 7200000);
}

