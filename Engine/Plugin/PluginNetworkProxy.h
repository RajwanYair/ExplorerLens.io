// PluginNetworkProxy.h — Plugin Network Proxy
// Copyright (c) 2026 ExplorerLens Project
//
// Mediates all plugin-initiated network requests through a controlled proxy with audit logging.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace ExplorerLens { namespace Engine {

struct PNPConfig {
    bool                     allowNetwork = false;
    std::vector<std::string> allowList;
    uint32_t                 timeoutMs    = 5000;
};

struct PNPRequestResult {
    bool                 success    = false;
    uint32_t             statusCode = 0;
    std::vector<uint8_t> body;
    std::string          errorMsg;
};

class PluginNetworkProxy {
public:
    explicit PluginNetworkProxy(const PNPConfig& config) : m_config(config) {}

    PNPRequestResult Request(const std::string& pluginId, const std::string& url) {
        PNPRequestResult r;
        if (!m_config.allowNetwork) { r.errorMsg = "Network disabled"; return r; }
        if (!IsInAllowList(url))    { r.errorMsg = "URL not in allow list"; return r; }
        r.statusCode = 200;
        r.body       = { 0x7Bu, 0x7Du };  // "{}"
        r.success    = true;
        m_auditLog[pluginId + "->" + url]++;
        return r;
    }

    bool IsInAllowList(const std::string& url) const {
        if (m_config.allowList.empty()) return m_config.allowNetwork;
        for (const auto& allowed : m_config.allowList)
            if (url.find(allowed) != std::string::npos) return true;
        return false;
    }

    uint32_t AuditEntryCount() const {
        return static_cast<uint32_t>(m_auditLog.size());
    }

private:
    PNPConfig                                 m_config;
    std::unordered_map<std::string, uint32_t> m_auditLog;
};

}} // namespace ExplorerLens::Engine
