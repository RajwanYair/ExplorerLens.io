// DecoderHotfixApplicator.h — Decoder Hotfix Applicator
// Copyright (c) 2026 ExplorerLens Project
//
// Manages an inventory of binary hotfixes for decoder modules, applies them
// in priority order, and tracks applied/pending state.
//
#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class HotfixPriority : uint8_t {
    Low = 0,
    Normal = 1,
    High = 2,
    Critical = 3,
    Emergency = 4
};

struct HotfixEntry
{
    std::string id;
    HotfixPriority priority = HotfixPriority::Normal;
    bool applied = false;
    std::string patchHash;
    std::string targetDecoder;
    std::string description;
};

class DecoderHotfixApplicator
{
  public:
    DecoderHotfixApplicator() = default;
    size_t GetPendingCount() const noexcept
    {
        return 0;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
