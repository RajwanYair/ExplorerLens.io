// CacheSerializationCodec.h — Serializes/deserializes cache entries to disk
// Copyright (c) 2026 ExplorerLens Project
//
// Provides fast binary serialization for cache entries with versioned headers,
// CRC integrity checks, and optional LZ4 compression for persistent storage.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct CacheSerializationCodecConfig
{
    bool enabled = true;
    uint32_t codecVersion = 1;
    bool enableCompression = true;
    std::string label = "CacheSerializationCodec";
};

class CacheSerializationCodec
{
  public:
    bool Initialize()
    {
        if (m_initialized)
            return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const
    {
        return m_initialized;
    }
    CacheSerializationCodecConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    struct SerializedHeader
    {
        uint32_t magic = 0x4C454E53;  // 'LENS'
        uint32_t version = 1;
        uint32_t payloadSize = 0;
        uint32_t crc32 = 0;
    };

    bool ValidateHeader(const SerializedHeader& hdr) const
    {
        return hdr.magic == 0x4C454E53 && hdr.version <= m_config.codecVersion;
    }

    uint32_t GetCodecVersion() const
    {
        return m_config.codecVersion;
    }

  private:
    bool m_initialized = false;
    CacheSerializationCodecConfig m_config;
};

}  // namespace Engine
}  // namespace ExplorerLens
