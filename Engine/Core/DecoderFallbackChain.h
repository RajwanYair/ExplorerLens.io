// DecoderFallbackChain.h — Ordered decoder fallback resolution chain
// Copyright (c) 2026 ExplorerLens Project
//
// Defines an ordered chain of decoders to try for a given format. If the
// primary decoder fails, the chain walks to the next available fallback.
//
#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct DecoderFallbackChainConfig
{
    bool enabled = true;
    uint32_t maxChainLength = 8;
    std::string label = "DecoderFallbackChain";
};

class DecoderFallbackChain
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
    DecoderFallbackChainConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    struct ChainEntry
    {
        std::string decoderName;
        int priority = 0;
        bool isAvailable = true;
    };

    bool AddDecoder(const ChainEntry& entry)
    {
        if (m_chain.size() >= m_config.maxChainLength)
            return false;
        m_chain.push_back(entry);
        return true;
    }

    const ChainEntry* GetFirstAvailable() const
    {
        for (const auto& e : m_chain) {
            if (e.isAvailable)
                return &e;
        }
        return nullptr;
    }

    size_t GetChainLength() const
    {
        return m_chain.size();
    }

  private:
    bool m_initialized = false;
    DecoderFallbackChainConfig m_config;
    std::vector<ChainEntry> m_chain;
};

}  // namespace Engine
}  // namespace ExplorerLens
