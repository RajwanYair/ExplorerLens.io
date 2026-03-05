// PluginAPIVersionNegotiator.h — Plugin API Version Compatibility Negotiation
// Copyright (c) 2026 ExplorerLens Project
//
// Negotiates API version compatibility between the host engine and plugins,
// supporting version ranges and multi-version compatibility matrices.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <string>
#include <mutex>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

struct APIVersionRange {
    uint32_t major = 0;
    uint32_t minor = 0;
    uint32_t minMajor = 0;
    uint32_t minMinor = 0;

    uint64_t Pack() const {
        return (static_cast<uint64_t>(major) << 32) | minor;
    }

    uint64_t PackMin() const {
        return (static_cast<uint64_t>(minMajor) << 32) | minMinor;
    }

    bool Contains(uint32_t maj, uint32_t min) const {
        uint64_t v = (static_cast<uint64_t>(maj) << 32) | min;
        return v >= PackMin() && v <= Pack();
    }

    std::string ToString() const {
        return std::to_string(major) + "." + std::to_string(minor);
    }

    std::string RangeString() const {
        return std::to_string(minMajor) + "." + std::to_string(minMinor) +
            " - " + ToString();
    }
};

struct PluginNegotiationResult {
    bool        compatible = false;
    uint32_t    agreedMajor = 0;
    uint32_t    agreedMinor = 0;
    std::string pluginId;
    std::string reason;

    std::string AgreedVersion() const {
        return std::to_string(agreedMajor) + "." + std::to_string(agreedMinor);
    }
};

class PluginAPIVersionNegotiator {
public:
    static PluginAPIVersionNegotiator& Instance() {
        static PluginAPIVersionNegotiator s;
        return s;
    }

    void SetHostVersion(uint32_t major, uint32_t minor,
        uint32_t minMajor, uint32_t minMinor) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_hostRange.major = major;
        m_hostRange.minor = minor;
        m_hostRange.minMajor = minMajor;
        m_hostRange.minMinor = minMinor;
    }

    PluginNegotiationResult Negotiate(const std::string& pluginId,
        const APIVersionRange& pluginRange) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        PluginNegotiationResult result;
        result.pluginId = pluginId;

        // Find highest version in intersection of ranges
        uint32_t maxMajor = (std::min)(m_hostRange.major, pluginRange.major);
        uint32_t maxMinor = (maxMajor == m_hostRange.major && maxMajor == pluginRange.major)
            ? (std::min)(m_hostRange.minor, pluginRange.minor)
            : (maxMajor == m_hostRange.major ? m_hostRange.minor : pluginRange.minor);

        uint32_t minMajor = (std::max)(m_hostRange.minMajor, pluginRange.minMajor);
        uint32_t minMinor = (minMajor == m_hostRange.minMajor && minMajor == pluginRange.minMajor)
            ? (std::max)(m_hostRange.minMinor, pluginRange.minMinor)
            : (minMajor == m_hostRange.minMajor ? m_hostRange.minMinor : pluginRange.minMinor);

        uint64_t maxVer = (static_cast<uint64_t>(maxMajor) << 32) | maxMinor;
        uint64_t minVer = (static_cast<uint64_t>(minMajor) << 32) | minMinor;

        if (maxVer >= minVer) {
            result.compatible = true;
            result.agreedMajor = maxMajor;
            result.agreedMinor = maxMinor;
            result.reason = "Compatible at " + result.AgreedVersion();
        }
        else {
            result.compatible = false;
            result.reason = "No overlapping version range. Host: " +
                m_hostRange.RangeString() + ", Plugin: " + pluginRange.RangeString();
        }

        return result;
    }

    bool IsCompatible(const std::string& pluginId, uint32_t major, uint32_t minor) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_hostRange.Contains(major, minor);
    }

    std::vector<APIVersionRange> GetSupportedVersions() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<APIVersionRange> versions;
        // Generate list of supported major versions
        for (uint32_t maj = m_hostRange.minMajor; maj <= m_hostRange.major; ++maj) {
            APIVersionRange v;
            v.major = maj;
            v.minor = (maj == m_hostRange.major) ? m_hostRange.minor : 99;
            v.minMajor = maj;
            v.minMinor = (maj == m_hostRange.minMajor) ? m_hostRange.minMinor : 0;
            versions.push_back(v);
        }
        return versions;
    }

    APIVersionRange GetHostRange() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_hostRange;
    }

    void Reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_hostRange = APIVersionRange{};
    }

    bool Validate() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_hostRange.Pack() < m_hostRange.PackMin()) return false;
        // Host should support latest version
        return m_hostRange.Contains(m_hostRange.major, m_hostRange.minor);
    }

private:
    PluginAPIVersionNegotiator() {
        m_hostRange.major = 15;
        m_hostRange.minor = 0;
        m_hostRange.minMajor = 12;
        m_hostRange.minMinor = 0;
    }
    ~PluginAPIVersionNegotiator() = default;
    PluginAPIVersionNegotiator(const PluginAPIVersionNegotiator&) = delete;
    PluginAPIVersionNegotiator& operator=(const PluginAPIVersionNegotiator&) = delete;

    mutable std::mutex m_mutex;
    APIVersionRange m_hostRange;
};

}
} // namespace ExplorerLens::Engine
