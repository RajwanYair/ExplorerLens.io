// DecoderRegistryV2.h — Decoder Registration and Discovery System
// Copyright (c) 2026 ExplorerLens Project
//
// Centralized registry for all format decoders. Supports static registration
// (built-in decoders) and dynamic registration (plugin decoders). Provides
// format-to-decoder routing, capability negotiation, and priority-based
// decoder selection when multiple decoders support the same format.
//
#pragma once

#include <windows.h>
#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Decoder capability flags
enum class DecoderCaps : uint32_t {
    None = 0,
    Decode = 1 << 0,          // Can decode to bitmap
    Metadata = 1 << 1,        // Can extract metadata
    Animation = 1 << 2,       // Supports animated formats
    MultiPage = 1 << 3,       // Multi-page documents
    HDR = 1 << 4,             // HDR content support
    WideGamut = 1 << 5,       // Wide color gamut
    AlphaChannel = 1 << 6,    // Alpha transparency
    Streaming = 1 << 7,       // Progressive/streaming decode
    GPUAccelerated = 1 << 8,  // GPU-assisted decode
    ThreadSafe = 1 << 9,      // Safe for concurrent use
    ZeroCopy = 1 << 10,       // Supports zero-copy path
    Incremental = 1 << 11,    // Incremental decode (partial results)
};

inline DecoderCaps operator|(DecoderCaps a, DecoderCaps b)
{
    return static_cast<DecoderCaps>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline DecoderCaps operator&(DecoderCaps a, DecoderCaps b)
{
    return static_cast<DecoderCaps>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}
inline bool HasCap(DecoderCaps set, DecoderCaps cap)
{
    return (static_cast<uint32_t>(set) & static_cast<uint32_t>(cap)) != 0;
}

/// Decoder source type
enum class DecoderSource : uint8_t {
    BuiltIn,   // Compiled into the engine
    Plugin,    // Loaded via plugin SDK
    System,    // Windows Imaging Component (WIC)
    External,  // External library (e.g., MuPDF)
};

/// Decoder registration info
struct RegistryDecoderInfo
{
    std::string name;         // Unique decoder name (e.g., "JPEG_WIC")
    std::string displayName;  // Human-readable name
    std::string version;      // Decoder version string
    DecoderCaps capabilities = DecoderCaps::Decode;
    DecoderSource source = DecoderSource::BuiltIn;
    int32_t priority = 100;  // Higher = preferred (0-1000)
    bool enabled = true;
    std::vector<std::string> supportedExtensions;  // e.g., {".jpg", ".jpeg", ".jpe"}
    std::vector<std::string> supportedMimeTypes;   // e.g., {"image/jpeg"}
};

/// Abstract decoder interface for registration
class IFormatDecoder
{
  public:
    virtual ~IFormatDecoder() = default;
    virtual RegistryDecoderInfo GetInfo() const = 0;
    virtual bool CanDecode(const wchar_t* filePath, const uint8_t* header, size_t headerSize) = 0;
    virtual HBITMAP Decode(const wchar_t* filePath, uint32_t targetSize) = 0;
    virtual bool DecodeToBuffer(const wchar_t* /*filePath*/, uint8_t* /*buffer*/, uint32_t /*width*/,
                                uint32_t /*height*/, uint32_t /*stride*/)
    {
        return false;
    }
};

/// Smart pointer for decoder instances
using DecoderPtr = std::shared_ptr<IFormatDecoder>;

/// Factory function type for creating decoder instances
using DecoderCreator = std::function<DecoderPtr()>;

/// Centralized decoder registry with format routing and priority selection.
///
/// Usage:
///   auto& registry = DecoderRegistryV2::Instance();
///   registry.RegisterDecoder("JPEG_WIC", CreateJPEGDecoder, {".jpg", ".jpeg"}, 100);
///   auto decoder = registry.FindBestDecoder(".jpg");
///   if (decoder) { auto bmp = decoder->Decode(path, 256); }
///
class DecoderRegistryV2
{
  public:
    static DecoderRegistryV2& Instance()
    {
        static DecoderRegistryV2 instance;
        return instance;
    }

    /// Register a decoder with factory function
    bool RegisterDecoder(const char* name, DecoderCreator factory, const std::vector<std::string>& extensions,
                         int32_t priority = 100, DecoderCaps caps = DecoderCaps::Decode,
                         DecoderSource source = DecoderSource::BuiltIn)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_factories.count(name))
            return false;  // Already registered

        RegistryEntry entry;
        entry.info.name = name;
        entry.info.displayName = name;
        entry.info.capabilities = caps;
        entry.info.source = source;
        entry.info.priority = priority;
        entry.info.supportedExtensions = extensions;
        entry.factory = std::move(factory);

        m_factories[name] = entry;

        // Index by extension
        for (const auto& ext : extensions) {
            std::string lowerExt = ToLower(ext);
            m_extensionIndex[lowerExt].push_back(name);
        }

        return true;
    }

    /// Unregister a decoder
    bool UnregisterDecoder(const char* name)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_factories.find(name);
        if (it == m_factories.end())
            return false;

        // Remove from extension index
        for (const auto& ext : it->second.info.supportedExtensions) {
            auto& vec = m_extensionIndex[ToLower(ext)];
            vec.erase(std::remove(vec.begin(), vec.end(), std::string(name)), vec.end());
        }

        m_factories.erase(it);
        return true;
    }

    /// Find the best (highest priority) decoder for a file extension
    DecoderPtr FindBestDecoder(const char* extension)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string lowerExt = ToLower(extension);

        auto it = m_extensionIndex.find(lowerExt);
        if (it == m_extensionIndex.end() || it->second.empty())
            return nullptr;

        // Sort by priority and return best
        std::string bestName;
        int32_t bestPriority = -1;

        for (const auto& name : it->second) {
            auto fit = m_factories.find(name);
            if (fit != m_factories.end() && fit->second.info.enabled) {
                if (fit->second.info.priority > bestPriority) {
                    bestPriority = fit->second.info.priority;
                    bestName = name;
                }
            }
        }

        if (bestName.empty())
            return nullptr;
        return GetOrCreateInstance(bestName);
    }

    /// Find all decoders for a file extension, sorted by priority
    std::vector<DecoderPtr> FindDecoders(const char* extension)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string lowerExt = ToLower(extension);
        std::vector<DecoderPtr> result;

        auto it = m_extensionIndex.find(lowerExt);
        if (it == m_extensionIndex.end())
            return result;

        // Collect and sort by priority
        std::vector<std::pair<int32_t, std::string>> candidates;
        for (const auto& name : it->second) {
            auto fit = m_factories.find(name);
            if (fit != m_factories.end() && fit->second.info.enabled) {
                candidates.emplace_back(fit->second.info.priority, name);
            }
        }
        std::sort(candidates.begin(), candidates.end(), [](const auto& a, const auto& b) { return a.first > b.first; });

        for (const auto& [pri, name] : candidates) {
            auto dec = GetOrCreateInstance(name);
            if (dec)
                result.push_back(dec);
        }
        return result;
    }

    /// Get info for all registered decoders
    std::vector<RegistryDecoderInfo> GetAllDecoders() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<RegistryDecoderInfo> result;
        for (const auto& [name, entry] : m_factories) {
            result.push_back(entry.info);
        }
        return result;
    }

    /// Enable/disable a specific decoder
    void SetDecoderEnabled(const char* name, bool enabled)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_factories.find(name);
        if (it != m_factories.end()) {
            it->second.info.enabled = enabled;
        }
    }

    /// Get count of registered decoders
    size_t GetDecoderCount() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_factories.size();
    }

    /// Get count of supported extensions
    size_t GetExtensionCount() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_extensionIndex.size();
    }

    /// Check if an extension is supported
    bool IsExtensionSupported(const char* extension) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_extensionIndex.count(ToLower(extension)) > 0;
    }

  private:
    DecoderRegistryV2() = default;

    struct RegistryEntry
    {
        RegistryDecoderInfo info;
        DecoderCreator factory;
        DecoderPtr instance;  // Cached singleton instance
    };

    DecoderPtr GetOrCreateInstance(const std::string& name)
    {
        auto& entry = m_factories[name];
        if (!entry.instance && entry.factory) {
            entry.instance = entry.factory();
        }
        return entry.instance;
    }

    static std::string ToLower(const std::string& s)
    {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return result;
    }

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, RegistryEntry> m_factories;
    std::unordered_map<std::string, std::vector<std::string>> m_extensionIndex;
};

}  // namespace Engine
}  // namespace ExplorerLens
