// DecoderCompatLayer.h — Decoder Compatibility Layer
// Copyright (c) 2026 ExplorerLens Project
//
// Provides backward-compatibility shims between decoder API versions, tracks
// known issues, and generates migration reports for upgrade paths.
//
#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class CompatMode : uint8_t {
    Strict     = 0,
    Relaxed    = 1,
    Legacy     = 2,
    AutoDetect = 3
};

struct CompatIssue {
    std::string decoderName;
    int         severity      = 0;
    std::string workaround;
    std::string sourceVersion;
    std::string targetVersion;
};

struct CompatReport {
    bool                    isCompatible       = false;
    uint32_t                migrationsRequired = 0;
    std::vector<CompatIssue> issues;
};

class DecoderCompatLayer {
public:
    DecoderCompatLayer() = default;
};

} // namespace Engine
} // namespace ExplorerLens
