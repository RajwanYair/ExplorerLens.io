// DecoderVersionRegistry.h — Decoder Version Registry
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks semantic versions of all registered decoder modules, enabling
// compatibility checks, upgrade management, and rollback support.
//
#pragma once
#include <cstddef>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct SemVer {
    uint32_t    major      = 0;
    uint32_t    minor      = 0;
    uint32_t    patch      = 0;
    std::string prerelease;
};

struct DecoderRegistration {
    std::string name;
    std::string author;
    std::string description;
    bool        isCompatible = false;
};

class DecoderVersionRegistry {
public:
    DecoderVersionRegistry() = default;
};

} // namespace Engine
} // namespace ExplorerLens
