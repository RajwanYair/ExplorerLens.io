#pragma once
// =============================================================================
// NetworkDiagnostics.h — Network Connectivity Testing
// ExplorerLens Engine — Utils Module
// =============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {

/// Network test type
enum class NetTestType : uint32_t {
    Ping = 0,          ///< ICMP ping
    DNSResolve = 1,    ///< DNS resolution
    HTTPGet = 2,       ///< HTTP GET request
    ProxyCheck = 3,    ///< Proxy connectivity
    TLSHandshake = 4,  ///< TLS/SSL handshake
    Count = 5
};

/// Network test status
enum class NetTestStatus : uint32_t {
    NotRun = 0,
    Running = 1,
    Passed = 2,
    Failed = 3,
    Timeout = 4,
    Skipped = 5
};

/// Proxy configuration
struct DiagnosticsProxyConfig
{
    std::wstring host;
    uint16_t port = 0;
    std::wstring username;
    bool enabled = false;
};

/// Network test result
struct NetTestResult
{
    NetTestType type = NetTestType::Ping;
    NetTestStatus status = NetTestStatus::NotRun;
    std::wstring target;  ///< Host/URL tested
    double latencyMs = 0.0;
    uint32_t statusCode = 0;  ///< HTTP status code (for HTTP tests)
    std::wstring errorMessage;
};

/// Overall diagnostics report
struct NetDiagReport
{
    bool allPassed = false;
    uint32_t passedCount = 0;
    uint32_t failedCount = 0;
    double totalLatencyMs = 0.0;
    std::vector<NetTestResult> results;
    DiagnosticsProxyConfig proxyConfig;
};

/// NetworkDiagnostics — tests network connectivity for update/cloud features
class NetworkDiagnostics
{
  public:
    NetworkDiagnostics();

    // Configuration
    void SetProxy(const DiagnosticsProxyConfig& config);
    void SetTimeout(uint32_t timeoutMs)
    {
        m_timeoutMs = timeoutMs;
    }
    void AddTarget(const std::wstring& target);

    // Run tests
    NetTestResult RunTest(NetTestType type, const std::wstring& target);
    NetDiagReport RunAllTests();

    // Queries
    const DiagnosticsProxyConfig& GetProxy() const
    {
        return m_proxy;
    }
    uint32_t GetTargetCount() const
    {
        return static_cast<uint32_t>(m_targets.size());
    }
    uint32_t GetTimeout() const
    {
        return m_timeoutMs;
    }

    // Static
    static const wchar_t* GetTestTypeName(NetTestType type);
    static const wchar_t* GetTestStatusName(NetTestStatus status);
    static constexpr uint32_t GetTestTypeCount()
    {
        return static_cast<uint32_t>(NetTestType::Count);
    }

  private:
    DiagnosticsProxyConfig m_proxy;
    std::vector<std::wstring> m_targets;
    uint32_t m_timeoutMs = 5000;
};

}  // namespace ExplorerLens
