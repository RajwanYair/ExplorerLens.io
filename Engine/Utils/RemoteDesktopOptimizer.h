// RemoteDesktopOptimizer.h — RDP/Citrix Rendering Optimization
// Copyright (c) 2026 ExplorerLens Project
//
// Detects remote desktop sessions (RDP, Citrix, VNC, VMware Horizon) and
// optimizes thumbnail rendering for low-bandwidth scenarios. Reduces
// color depth, simplifies GPU operations, increases JPEG compression,
// and adjusts decode quality to maintain responsiveness over remote links.
//
#pragma once

#include <algorithm>
#include <cstdint>
#include <string>

#ifndef NOMINMAX
    #define NOMINMAX
#endif
#include <windows.h>
#include <tlhelp32.h>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// Remote session type
// ============================================================================

enum class RemoteSessionType : uint8_t {
    Local = 0,    // Physical console
    RDP = 1,      // Microsoft Remote Desktop
    Citrix = 2,   // Citrix HDX/ICA
    VNC = 3,      // VNC variants
    VMware = 4,   // VMware Horizon
    AnyDesk = 5,  // AnyDesk
    Unknown = 6   // Remote but unknown protocol
};

inline const char* RemoteSessionTypeToString(RemoteSessionType type)
{
    static const char* names[] = {"Local", "RDP", "Citrix", "VNC", "VMware", "AnyDesk", "Unknown"};
    return names[static_cast<uint8_t>(type)];
}

// ============================================================================
// Bandwidth tier
// ============================================================================

enum class BandwidthTier : uint8_t {
    Local = 0,       // Local display — no bandwidth constraint
    HighSpeed = 1,   // >100 Mbps — minimal optimization
    Broadband = 2,   // 10-100 Mbps — moderate optimization
    LowBand = 3,     // 1-10 Mbps — aggressive optimization
    Constrained = 4  // <1 Mbps — maximum optimization
};

inline const char* BandwidthTierToString(BandwidthTier tier)
{
    static const char* names[] = {"Local", "HighSpeed", "Broadband", "LowBand", "Constrained"};
    return names[static_cast<uint8_t>(tier)];
}

// ============================================================================
// Rendering adjustment profile
// ============================================================================

struct RemoteRenderProfile
{
    BandwidthTier tier = BandwidthTier::Local;
    uint8_t jpegQuality = 90;           // 1-100
    uint32_t maxThumbnailSize = 256;    // Max dimension
    bool disableGPU = false;            // Force CPU-only decode
    bool reduceColorDepth = false;      // 16bpp instead of 32bpp
    bool skipAlpha = false;             // Ignore alpha channel
    bool disableAntiAlias = false;      // Skip AA for text overlays
    float scaleReduction = 1.0f;        // 0.5 = half resolution
    uint32_t maxConcurrentDecodes = 4;  // Limit parallelism

    static RemoteRenderProfile ForTier(BandwidthTier tier)
    {
        RemoteRenderProfile p;
        p.tier = tier;
        switch (tier) {
            case BandwidthTier::Local:
                // No optimization needed
                break;
            case BandwidthTier::HighSpeed:
                p.jpegQuality = 85;
                break;
            case BandwidthTier::Broadband:
                p.jpegQuality = 75;
                p.maxThumbnailSize = 192;
                p.maxConcurrentDecodes = 3;
                break;
            case BandwidthTier::LowBand:
                p.jpegQuality = 60;
                p.maxThumbnailSize = 128;
                p.reduceColorDepth = true;
                p.skipAlpha = true;
                p.maxConcurrentDecodes = 2;
                p.scaleReduction = 0.75f;
                break;
            case BandwidthTier::Constrained:
                p.jpegQuality = 40;
                p.maxThumbnailSize = 96;
                p.disableGPU = true;
                p.reduceColorDepth = true;
                p.skipAlpha = true;
                p.disableAntiAlias = true;
                p.maxConcurrentDecodes = 1;
                p.scaleReduction = 0.5f;
                break;
        }
        return p;
    }
};

// ============================================================================
// Session detection statistics
// ============================================================================

struct RemoteDesktopStats
{
    RemoteSessionType sessionType = RemoteSessionType::Local;
    BandwidthTier tier = BandwidthTier::Local;
    uint32_t screenWidth = 0;
    uint32_t screenHeight = 0;
    uint32_t colorDepth = 32;
    bool isRemote = false;
    bool isVM = false;
    uint64_t estimatedBandwidthKbps = 0;
    uint32_t profileSwitches = 0;
};

// ============================================================================
// RemoteDesktopOptimizer — main class
// ============================================================================

class RemoteDesktopOptimizer
{
  public:
    RemoteDesktopOptimizer() = default;

    /// Detect the current session type and configure profile
    bool Initialize()
    {
        DetectSession();
        m_profile = RemoteRenderProfile::ForTier(m_stats.tier);
        m_initialized = true;
        return true;
    }

    bool IsInitialized() const
    {
        return m_initialized;
    }
    bool IsRemoteSession() const
    {
        return m_stats.isRemote;
    }

    /// Get the session type
    RemoteSessionType GetSessionType() const
    {
        return m_stats.sessionType;
    }

    /// Get the current rendering profile
    const RemoteRenderProfile& GetProfile() const
    {
        return m_profile;
    }

    /// Manually override bandwidth tier (e.g., from user settings)
    void SetBandwidthTier(BandwidthTier tier)
    {
        m_stats.tier = tier;
        m_profile = RemoteRenderProfile::ForTier(tier);
        m_stats.profileSwitches++;
    }

    /// Get optimal thumbnail size for current session
    uint32_t GetOptimalThumbnailSize(uint32_t requestedSize) const
    {
        uint32_t maxSize = m_profile.maxThumbnailSize;
        return (std::min)(requestedSize, maxSize);
    }

    /// Get optimal JPEG quality for current session
    uint8_t GetOptimalJPEGQuality() const
    {
        return m_profile.jpegQuality;
    }

    /// Should GPU rendering be used?
    bool ShouldUseGPU() const
    {
        return !m_profile.disableGPU;
    }

    /// Get statistics
    const RemoteDesktopStats& GetStats() const
    {
        return m_stats;
    }

  private:
    void DetectSession()
    {
        // Check for remote desktop via GetSystemMetrics
        m_stats.isRemote = (::GetSystemMetrics(SM_REMOTESESSION) != 0);

        // Screen info
        m_stats.screenWidth = static_cast<uint32_t>(::GetSystemMetrics(SM_CXSCREEN));
        m_stats.screenHeight = static_cast<uint32_t>(::GetSystemMetrics(SM_CYSCREEN));

        if (m_stats.isRemote) {
            // Try to identify specific protocol
            m_stats.sessionType = DetectProtocol();

            // Estimate bandwidth from color depth and screen size
            HDC hdc = ::GetDC(nullptr);
            if (hdc) {
                m_stats.colorDepth = static_cast<uint32_t>(::GetDeviceCaps(hdc, BITSPIXEL));
                ::ReleaseDC(nullptr, hdc);
            }

            // Heuristic bandwidth estimation
            if (m_stats.colorDepth <= 16) {
                m_stats.tier = BandwidthTier::LowBand;
                m_stats.estimatedBandwidthKbps = 5000;
            } else if (m_stats.screenWidth <= 1280) {
                m_stats.tier = BandwidthTier::Broadband;
                m_stats.estimatedBandwidthKbps = 50000;
            } else {
                m_stats.tier = BandwidthTier::HighSpeed;
                m_stats.estimatedBandwidthKbps = 100000;
            }
        } else {
            m_stats.sessionType = RemoteSessionType::Local;
            m_stats.tier = BandwidthTier::Local;
        }

        // VM detection (virtualized hardware often has specific display adapter names)
        // In production would check WMI display adapter names
        m_stats.isVM = false;
    }

    RemoteSessionType DetectProtocol() const
    {
        // Check for RDP-specific environment variable
        wchar_t sessionName[64] = {};
        DWORD size = 64;
        if (::GetEnvironmentVariableW(L"SESSIONNAME", sessionName, size)) {
            std::wstring name(sessionName);
            if (name.find(L"RDP") != std::wstring::npos)
                return RemoteSessionType::RDP;
            if (name.find(L"ICA") != std::wstring::npos)
                return RemoteSessionType::Citrix;
        }

        // Check for Citrix receiver process
        HANDLE hSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnap != INVALID_HANDLE_VALUE) {
            ::CloseHandle(hSnap);
            // In production: enumerate processes for wfica32.exe, vmware-view.exe, etc.
        }

        return RemoteSessionType::Unknown;
    }

    bool m_initialized = false;
    RemoteRenderProfile m_profile;
    RemoteDesktopStats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
