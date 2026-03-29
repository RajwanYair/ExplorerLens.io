// VirtualDesktopAwareness.h — Virtual Desktop Manager Awareness
// Copyright (c) 2026 ExplorerLens Project
//
// Integrates with the Windows Virtual Desktop Manager to scope thumbnail caches
// per-virtual-desktop, preventing cross-desktop cache pollution and enabling
// independent layout/sort persistence per virtual workspace.
//
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <stdint.h>

namespace ExplorerLens {
namespace Engine {

using VirtualDesktopId = uint64_t;
static constexpr VirtualDesktopId VDID_GLOBAL = 0;
static constexpr VirtualDesktopId VDID_INVALID = UINT64_MAX;

struct VirtualDesktopInfo {
    VirtualDesktopId id      = VDID_INVALID;
    std::wstring     name;
    bool             isActive = false;
    int              index    = -1;
};

enum class VDScopePolicy { Global, PerDesktop, ActiveOnly };

class VirtualDesktopAwareness {
public:
    static VirtualDesktopAwareness& Instance() {
        static VirtualDesktopAwareness inst;
        return inst;
    }

    VirtualDesktopId GetCurrentDesktopId() const noexcept { return m_currentId; }
    void             SetCurrentDesktopId(VirtualDesktopId id) noexcept { m_currentId = id; }

    void             RegisterDesktop(const VirtualDesktopInfo& vd) { m_desktops[vd.id] = vd; }

    VirtualDesktopId GetCacheScopeId(VDScopePolicy policy) const noexcept {
        switch (policy) {
        case VDScopePolicy::PerDesktop:  return m_currentId;
        case VDScopePolicy::ActiveOnly:  return m_currentId;
        case VDScopePolicy::Global:      return VDID_GLOBAL;
        }
        return VDID_GLOBAL;
    }

    std::string BuildCacheKeyPrefix(VDScopePolicy policy) const {
        if (policy == VDScopePolicy::Global) return "global";
        return "vd:" + std::to_string(GetCacheScopeId(policy));
    }

    int  DesktopCount()            const noexcept { return (int)m_desktops.size(); }
    bool HasDesktop(VirtualDesktopId id) const noexcept { return m_desktops.count(id) > 0; }

    std::vector<VirtualDesktopInfo> AllDesktops() const {
        std::vector<VirtualDesktopInfo> v;
        for (const auto& kv : m_desktops) v.push_back(kv.second);
        return v;
    }

private:
    VirtualDesktopAwareness() = default;
    VirtualDesktopId m_currentId = VDID_GLOBAL;
    std::unordered_map<VirtualDesktopId, VirtualDesktopInfo> m_desktops;
};

} // namespace Engine
} // namespace ExplorerLens
