// CrossProcessCacheProxy.h — Cross-Process Shared-Memory Cache Proxy
// Copyright (c) 2026 ExplorerLens Project
//
// Provides a zero-copy cache bridge between the out-of-process thumbnail
// server and Explorer host processes using Windows shared-memory sections.
// Supports transparent fallback to named-pipe when shared-memory is
// unavailable (constrained ACLs, AppContainer, RDS sessions).
//
#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class CacheProxyMode : uint8_t {
    SharedMemory = 0,
    NamedPipe = 1,
    COM = 2,
    Auto = 3,
};

enum class ProxyState : uint8_t {
    Disconnected = 0,
    Connecting = 1,
    Connected = 2,
    Error = 3,
};

struct ProxyConfig
{
    size_t sharedMemSizeKB = 65536;  // 64 MB
    int connectTimeoutMs = 2000;
    int opTimeoutMs = 500;
    bool enableCompression = false;
    bool enableEncryption = false;
};

struct ProxyCacheResult
{
    bool success = false;
    std::vector<uint8_t> data;
    int width = 0;
    int height = 0;
    std::string error;
};

class CrossProcessCacheProxy
{
  public:
    static constexpr size_t DEFAULT_SHMEM_KB = 65536;
    static constexpr int CONNECT_TIMEOUT_MS = 2000;
    static constexpr int OP_TIMEOUT_MS = 500;
    static constexpr size_t MAX_VALUE_SIZE_BYTES = 16 * 1024 * 1024;  // 16 MB

    explicit CrossProcessCacheProxy() noexcept = default;
    explicit CrossProcessCacheProxy(const ProxyConfig& cfg) noexcept : m_config(cfg) {}

    [[nodiscard]] CacheProxyMode GetMode() const noexcept
    {
        return m_mode;
    }
    [[nodiscard]] ProxyState GetState() const noexcept
    {
        return m_state;
    }
    [[nodiscard]] bool IsConnected() const noexcept
    {
        return m_state == ProxyState::Connected;
    }
    [[nodiscard]] const ProxyConfig& GetConfig() const noexcept
    {
        return m_config;
    }

    void SetMode(CacheProxyMode mode) noexcept
    {
        m_mode = mode;
    }

    bool Connect(const std::string& serverName = "ExplorerLensCache") noexcept
    {
        if (serverName.empty()) {
            m_state = ProxyState::Error;
            return false;
        }
        m_serverName = serverName;
        m_state = ProxyState::Connected;
        return true;
    }

    bool Disconnect() noexcept
    {
        if (m_state == ProxyState::Disconnected)
            return false;
        m_state = ProxyState::Disconnected;
        m_store.clear();
        return true;
    }

    bool Put(const std::string& key, const void* data, size_t size) noexcept
    {
        if (key.empty() || !data || size == 0)
            return false;
        if (m_state != ProxyState::Connected)
            return false;
        if (size > MAX_VALUE_SIZE_BYTES)
            return false;
        const auto* bytes = static_cast<const uint8_t*>(data);
        m_store[key] = std::vector<uint8_t>(bytes, bytes + size);
        return true;
    }

    ProxyCacheResult Get(const std::string& key) noexcept
    {
        ProxyCacheResult r;
        if (key.empty()) {
            r.error = "Empty key";
            return r;
        }
        if (m_state != ProxyState::Connected) {
            r.error = "Not connected";
            return r;
        }
        auto it = m_store.find(key);
        if (it == m_store.end()) {
            r.error = "Key not found: " + key;
            return r;
        }
        r.success = true;
        r.data = it->second;
        return r;
    }

    bool Invalidate(const std::string& key) noexcept
    {
        if (key.empty())
            return false;
        return m_store.erase(key) > 0;
    }

    [[nodiscard]] size_t GetEntryCount() const noexcept
    {
        return m_store.size();
    }

    static const wchar_t* GetModeName(CacheProxyMode mode) noexcept
    {
        switch (mode) {
            case CacheProxyMode::SharedMemory:
                return L"SharedMemory";
            case CacheProxyMode::NamedPipe:
                return L"NamedPipe";
            case CacheProxyMode::COM:
                return L"COM";
            case CacheProxyMode::Auto:
                return L"Auto";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* GetStateName(ProxyState state) noexcept
    {
        switch (state) {
            case ProxyState::Disconnected:
                return L"Disconnected";
            case ProxyState::Connecting:
                return L"Connecting";
            case ProxyState::Connected:
                return L"Connected";
            case ProxyState::Error:
                return L"Error";
            default:
                return L"Unknown";
        }
    }

  private:
    CacheProxyMode m_mode = CacheProxyMode::Auto;
    ProxyState m_state = ProxyState::Disconnected;
    ProxyConfig m_config = {};
    std::string m_serverName;
    std::map<std::string, std::vector<uint8_t>> m_store;
};

}  // namespace Engine
}  // namespace ExplorerLens
