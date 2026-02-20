// =============================================================================
// NetworkDiagnostics.cpp — Sprint 246: Network Connectivity Testing
// DarkThumbs Engine — Utils Module
// =============================================================================

#include "NetworkDiagnostics.h"
#include <algorithm>

namespace DarkThumbs {

NetworkDiagnostics::NetworkDiagnostics() {
    // Default targets for connectivity checks
    m_targets.push_back(L"https://github.com");
    m_targets.push_back(L"https://api.github.com");
}

void NetworkDiagnostics::SetProxy(const ProxyConfig& config) {
    m_proxy = config;
}

void NetworkDiagnostics::AddTarget(const std::wstring& target) {
    m_targets.push_back(target);
}

NetTestResult NetworkDiagnostics::RunTest(NetTestType type, const std::wstring& target) {
    NetTestResult result;
    result.type = type;
    result.target = target;

    if (target.empty()) {
        result.status = NetTestStatus::Failed;
        result.errorMessage = L"Empty target";
        return result;
    }

    // Simulate test execution (real implementation would use WinHTTP/WinSock)
    switch (type) {
        case NetTestType::Ping:
            result.status = NetTestStatus::Passed;
            result.latencyMs = 12.5;
            break;
        case NetTestType::DNSResolve:
            result.status = NetTestStatus::Passed;
            result.latencyMs = 3.2;
            break;
        case NetTestType::HTTPGet:
            result.status = NetTestStatus::Passed;
            result.latencyMs = 85.0;
            result.statusCode = 200;
            break;
        case NetTestType::ProxyCheck:
            if (m_proxy.enabled) {
                result.status = NetTestStatus::Passed;
                result.latencyMs = 15.0;
            } else {
                result.status = NetTestStatus::Skipped;
                result.errorMessage = L"Proxy not configured";
            }
            break;
        case NetTestType::TLSHandshake:
            result.status = NetTestStatus::Passed;
            result.latencyMs = 45.0;
            break;
        default:
            result.status = NetTestStatus::Failed;
            result.errorMessage = L"Unknown test type";
            break;
    }

    return result;
}

NetDiagReport NetworkDiagnostics::RunAllTests() {
    NetDiagReport report;
    report.proxyConfig = m_proxy;

    for (const auto& target : m_targets) {
        for (uint32_t t = 0; t < static_cast<uint32_t>(NetTestType::Count); ++t) {
            auto result = RunTest(static_cast<NetTestType>(t), target);
            if (result.status == NetTestStatus::Passed) {
                report.passedCount++;
                report.totalLatencyMs += result.latencyMs;
            } else if (result.status == NetTestStatus::Failed || result.status == NetTestStatus::Timeout) {
                report.failedCount++;
            }
            report.results.push_back(result);
        }
    }

    report.allPassed = (report.failedCount == 0);
    return report;
}

const wchar_t* NetworkDiagnostics::GetTestTypeName(NetTestType type) {
    switch (type) {
        case NetTestType::Ping:         return L"Ping";
        case NetTestType::DNSResolve:   return L"DNS Resolve";
        case NetTestType::HTTPGet:      return L"HTTP GET";
        case NetTestType::ProxyCheck:   return L"Proxy Check";
        case NetTestType::TLSHandshake: return L"TLS Handshake";
        default:                        return L"Unknown";
    }
}

const wchar_t* NetworkDiagnostics::GetTestStatusName(NetTestStatus status) {
    switch (status) {
        case NetTestStatus::NotRun:  return L"Not Run";
        case NetTestStatus::Running: return L"Running";
        case NetTestStatus::Passed:  return L"Passed";
        case NetTestStatus::Failed:  return L"Failed";
        case NetTestStatus::Timeout: return L"Timeout";
        case NetTestStatus::Skipped: return L"Skipped";
        default:                     return L"Unknown";
    }
}

} // namespace DarkThumbs
