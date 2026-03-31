// FormatCapabilityMatrix.h — Format Capability Matrix
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks which capability level each format achieves on each platform,
// enabling runtime feature gating and upgrade-path reporting.
//
#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class CapabilityLevel : uint8_t {
    None            = 0,
    Basic           = 1,
    Full            = 2,
    GPUAccelerated  = 3
};

enum class PlatformType : uint8_t {
    Windows = 0,
    Linux   = 1,
    macOS   = 2,
    WASM    = 3
};

struct FormatCapability {
    std::string    format;
    CapabilityLevel level      = CapabilityLevel::None;
    std::string    minVersion;
    PlatformType   platform    = PlatformType::Windows;
};

class FormatCapabilityMatrix {
public:
    int GetTotalFormatCount() const noexcept { return 0; }
};

} // namespace Engine
} // namespace ExplorerLens
