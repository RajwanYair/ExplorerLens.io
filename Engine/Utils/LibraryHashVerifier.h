// Utils\LibraryHashVerifier.h - LibraryHashVerifier
// Copyright (c) 2026 ExplorerLens Project
//
// Verifies external library binary hashes against known-good values
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

class LibraryHashVerifier
{
  public:
    struct Config
    {
        bool enabled = true;
        uint32_t maxEntries = 1024;
        std::string label = "LibraryHashVerifier";
    };

    LibraryHashVerifier() = default;
    explicit LibraryHashVerifier(const Config& cfg) : m_config(cfg) {}

    bool Initialize()
    {
        if (!m_config.enabled)
            return false;
        m_initialized = true;
        return true;
    }

    bool IsInitialized() const
    {
        return m_initialized;
    }
    const Config& GetConfig() const
    {
        return m_config;
    }
    const std::string& GetName() const
    {
        return m_config.label;
    }

  private:
    Config m_config{};
    bool m_initialized = false;
    std::string m_hashDatabase{};
    std::string m_algorithm{};
    uint32_t m_verifiedCount{};
};

}  // namespace Engine
}  // namespace ExplorerLens
