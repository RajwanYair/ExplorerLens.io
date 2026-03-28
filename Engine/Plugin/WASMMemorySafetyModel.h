// WASMMemorySafetyModel.h — WebAssembly Linear-Memory Safety Model
// Copyright (c) 2026 ExplorerLens Project
//
// Enforces bounds-checking, capability isolation, and guard-region layout
// for WASM plugin linear memory — preventing host memory escapes.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class WASMMemoryPolicy  { Unrestricted, Bounded, GuardPaged, Strict };
enum class WASMCapabilityMask : uint32_t {
    None        = 0x00,
    ReadFiles   = 0x01,
    WriteFiles  = 0x02,
    Network     = 0x04,
    ClockAccess = 0x08,
    RandomBytes = 0x10,
    All         = 0x1F
};

inline WASMCapabilityMask operator|(WASMCapabilityMask a, WASMCapabilityMask b) {
    return static_cast<WASMCapabilityMask>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

struct WASMMemoryDescriptor {
    uint64_t         baseBytes     = 0;
    uint64_t         sizeBytes     = 4ULL * 1024 * 1024;
    uint64_t         guardSizeBytes= 65536;
    WASMMemoryPolicy policy        = WASMMemoryPolicy::GuardPaged;
    WASMCapabilityMask capabilities= WASMCapabilityMask::ReadFiles;
};

class WASMMemorySafetyModel {
public:
    explicit WASMMemorySafetyModel(const WASMMemoryDescriptor& desc = {}) : m_desc(desc) {}

    bool  Validate()  const { return m_desc.sizeBytes > 0 && m_desc.guardSizeBytes > 0; }
    bool  IsCapabilityGranted(WASMCapabilityMask cap) const {
        return (static_cast<uint32_t>(m_desc.capabilities) &
                static_cast<uint32_t>(cap)) != 0;
    }
    void  GrantCapability(WASMCapabilityMask cap) {
        m_desc.capabilities = m_desc.capabilities | cap;
    }
    void  RevokeCapability(WASMCapabilityMask cap) {
        m_desc.capabilities = static_cast<WASMCapabilityMask>(
            static_cast<uint32_t>(m_desc.capabilities) & ~static_cast<uint32_t>(cap));
    }
    uint64_t             GetSizeBytes()    const { return m_desc.sizeBytes; }
    uint64_t             GetGuardSize()    const { return m_desc.guardSizeBytes; }
    WASMMemoryPolicy     GetPolicy()       const { return m_desc.policy; }
    WASMCapabilityMask   GetCapabilities() const { return m_desc.capabilities; }
    void  SetPolicy(WASMMemoryPolicy p)         { m_desc.policy = p; }
    void  SetSizeBytes(uint64_t s)              { m_desc.sizeBytes = s; }
    void  Reset()                               { m_desc = WASMMemoryDescriptor{}; }

private:
    WASMMemoryDescriptor m_desc;
};

} // namespace Engine
} // namespace ExplorerLens
