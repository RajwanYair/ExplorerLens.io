// FormatStatusIndicator.h — Format Decoder Status Indicator
// Copyright (c) 2026 ExplorerLens Project
//
// Reports operational health of each format decoder: active, degraded, or
// unavailable, with human-readable labels for UI display.
//
#pragma once
#include <cstddef>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

enum class FormatStatus : uint8_t {
    Active,
    Degraded,
    Unavailable,
    COUNT = 3
};

class FormatStatusIndicator {
public:
    static size_t StatusCount() noexcept {
        return static_cast<size_t>(FormatStatus::COUNT);
    }
    static const wchar_t *StatusName(FormatStatus s) noexcept {
        switch (s) {
        case FormatStatus::Active:      return L"Active";
        case FormatStatus::Degraded:    return L"Degraded";
        case FormatStatus::Unavailable: return L"Unavailable";
        default: return L"Unknown";
        }
    }
};

} // namespace Engine
} // namespace ExplorerLens
