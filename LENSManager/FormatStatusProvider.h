// FormatStatusProvider.h — Live format status for Settings UI traffic lights
// Copyright (c) 2026 ExplorerLens Project
//
// Bridges DecoderHealthCheck with the dialog UI to show green/yellow/red
// indicators next to each format checkbox. Provides tooltip text.
#pragma once

#include <string>
#include <unordered_map>

#include "DecoderHealthCheck.h"
#include "RegManager.h"

namespace ExplorerLens {

// ============================================================================
// Status level (traffic light)
// ============================================================================

enum class FormatStatus {
    ACTIVE,       // Green: decoder available and working
    DEGRADED,     // Yellow: partially working (e.g., using fallback decoder)
    UNAVAILABLE,  // Red: decoder not available (missing library)
    UNKNOWN       // Gray: not checked yet
};

struct FormatStatusInfo
{
    FormatStatus status = FormatStatus::UNKNOWN;
    std::wstring tooltip;     // Hover text for the checkbox
    std::wstring decoderLib;  // Which library provides decoding
    std::wstring version;     // Library version string
    bool gpuAccelerated = false;
};

// ============================================================================
// FormatStatusProvider — singleton providing live decoder status
// ============================================================================

class FormatStatusProvider
{
  public:
    static FormatStatusProvider& Instance()
    {
        static FormatStatusProvider instance;
        return instance;
    }

    // Refresh all status checks (call on dialog init and after config changes)
    void Refresh()
    {
        m_statusMap.clear();
        auto healthResults = DecoderHealthCheck::CheckAll();

        for (const auto& info : healthResults) {
            FormatStatusInfo fsi;
            if (info.isAvailable) {
                fsi.status = FormatStatus::ACTIVE;
                fsi.tooltip = info.name + L": Active";
                if (!info.version.empty() && info.version != L"detected") {
                    fsi.tooltip += L" — v" + info.version;
                }
            } else {
                fsi.status = FormatStatus::UNAVAILABLE;
                fsi.tooltip = info.name + L": Unavailable";
                if (info.hasExternalDependency) {
                    fsi.tooltip += L" — Missing: " + info.dllName;
                }
            }
            fsi.decoderLib = info.dllName;
            fsi.version = info.version;
            fsi.tooltip += L"\n" + info.statusMessage;

            // Map decoder name to status
            m_statusMap[info.name] = fsi;
        }

        // Set specific format statuses based on build configuration
        SetBuildConfigStatus();
    }

    // Get status for a specific format by LENS_* type
    FormatStatusInfo GetStatus(int lensType) const
    {
        std::wstring decoderName = MapLensTypeToDecoder(lensType);
        auto it = m_statusMap.find(decoderName);
        if (it != m_statusMap.end()) {
            return it->second;
        }
        // Default: built-in decoder, always available
        FormatStatusInfo defaultInfo;
        defaultInfo.status = FormatStatus::ACTIVE;
        defaultInfo.tooltip = L"Built-in decoder — always available";
        defaultInfo.decoderLib = L"built-in";
        return defaultInfo;
    }

    // Get COLORREF for drawing traffic light
    static COLORREF GetStatusColor(FormatStatus status)
    {
        switch (status) {
            case FormatStatus::ACTIVE:
                return RGB(34, 139, 34);  // Forest green
            case FormatStatus::DEGRADED:
                return RGB(218, 165, 32);  // Goldenrod
            case FormatStatus::UNAVAILABLE:
                return RGB(178, 34, 34);  // Firebrick
            default:
                return RGB(128, 128, 128);  // Gray
        }
    }

    // Get status label text
    static const wchar_t* GetStatusLabel(FormatStatus status)
    {
        switch (status) {
            case FormatStatus::ACTIVE:
                return L"Active";
            case FormatStatus::DEGRADED:
                return L"Degraded";
            case FormatStatus::UNAVAILABLE:
                return L"Unavailable";
            default:
                return L"Unknown";
        }
    }

  private:
    FormatStatusProvider()
    {
        Refresh();
    }

    std::unordered_map<std::wstring, FormatStatusInfo> m_statusMap;

    void SetBuildConfigStatus()
    {
        // Check for GPU acceleration availability
        // These are determined by Engine build flags
        auto setGPU = [this](const std::wstring& name) {
            auto it = m_statusMap.find(name);
            if (it != m_statusMap.end() && it->second.status == FormatStatus::ACTIVE) {
                it->second.gpuAccelerated = true;
                it->second.tooltip += L"\nGPU-accelerated resize available";
            }
        };
        // Most image decoders benefit from GPU resize
        setGPU(L"Image (WIC)");
        setGPU(L"WebP");
        setGPU(L"AVIF");
        setGPU(L"JXL");
        setGPU(L"HEIF");
        setGPU(L"RAW");
    }

    static std::wstring MapLensTypeToDecoder(int lensType)
    {
        // Map LENS_* constants (from RegManager.h) to decoder names
        switch (lensType) {
            case 1:  // LENS_ZIP
            case 2:  // LENS_RAR
            case 3:  // LENS_7Z
            case 4:  // LENS_TAR
            case 5:  // LENS_CBZ
            case 6:  // LENS_CBR
            case 7:  // LENS_CB7
            case 8:  // LENS_CBT
                return L"Archive";
            case 9:   // LENS_EPUB
            case 10:  // LENS_MOBI
            case 14:  // LENS_FB2
            case 44:  // LENS_AZW
            case 45:  // LENS_AZW3
                return L"Document";
            case 15:  // LENS_WEBP
                return L"WebP";
            case 16:  // LENS_HEIF
                return L"HEIF";
            case 17:  // LENS_AVIF
                return L"AVIF";
            case 18:  // LENS_JXL
                return L"JXL";
            case 19:  // LENS_TIFF
                return L"Image (WIC)";
            case 20:  // LENS_SVG
                return L"SVG";
            case 42:  // LENS_RAW
                return L"RAW";
            case 53:  // LENS_PDF
                return L"PDF";
            case 43:  // LENS_PSD
                return L"PSD";
            case 46:  // LENS_DDS
                return L"DDS";
            case 47:  // LENS_HDR
                return L"HDR";
            case 48:  // LENS_EXR
                return L"EXR";
            case 49:  // LENS_PPM
                return L"PPM";
            case 50:  // LENS_VIDEO
                return L"Video";
            case 51:  // LENS_AUDIO
                return L"Audio";
            case 52:  // LENS_FONT
                return L"Font";
            case 76:  // LENS_MODEL
                return L"Document";
            default:
                return L"Image (WIC)";
        }
    }
};

}  // namespace ExplorerLens
