// FormatNegotiator.h — Output Format Negotiation
// Copyright (c) 2026 ExplorerLens Project
//
// Determines the optimal output format and pixel layout for thumbnail delivery
// based on the requesting shell context, display capabilities, and source format.
// Handles color space negotiation, alpha channel requirements, HDR tone mapping
// decisions, and DPI-aware sizing.
//
#pragma once

#include <windows.h>
#include <cstdint>
#include <string>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

/// Pixel format for thumbnail output
enum class OutputPixelFormat : uint8_t {
    BGRA32,         // Standard 32-bit BGRA (most common)
    BGR24,          // 24-bit BGR (no alpha)
    BGRA32_PreAlpha,// Pre-multiplied alpha BGRA
    RGBA32,         // RGBA order (for some contexts)
    Gray8,          // 8-bit grayscale
    Gray16,         // 16-bit grayscale (scientific)
    RGBAF16,        // Half-float RGBA (HDR preview)
};

/// Color space for output
enum class OutputColorSpace : uint8_t {
    sRGB,           // Standard sRGB (default)
    LinearRGB,      // Linear RGB (for compositing)
    DisplayP3,      // Display P3 (wide gamut monitors)
    AdobeRGB,       // Adobe RGB (photography)
    Rec2020,        // Rec. 2020 (HDR)
};

/// DPI awareness level
enum class OutputDPIMode : uint8_t {
    Unaware,        // 96 DPI assumed
    System,         // System DPI
    PerMonitor,     // Per-monitor DPI v1
    PerMonitorV2,   // Per-monitor DPI v2
};

/// Shell context requesting the thumbnail
enum class ShellContext : uint8_t {
    ExplorerView,       // Standard Explorer thumbnail/icon view
    PreviewPane,        // Explorer preview pane (larger)
    ThumbnailCache,     // Shell thumbnail cache (TDB)
    PropertyDialog,     // File properties dialog
    TaskbarPreview,     // Taskbar thumbnail preview
    StartMenu,          // Start menu tile
    SearchResults,      // Search result preview
    Custom,             // Custom application request
};

/// Negotiated output specification
struct NegotiatedOutput {
    OutputPixelFormat   pixelFormat = OutputPixelFormat::BGRA32;
    OutputColorSpace    colorSpace = OutputColorSpace::sRGB;
    uint32_t            width = 256;
    uint32_t            height = 256;
    uint32_t            stride = 0;    // 0 = auto-calculate
    uint32_t            dpi = 96;
    bool                needsAlpha = true;
    bool                needsHDRTonemap = false;
    bool                needsICCProfile = false;
    bool                needsPremulAlpha = false;
    float               qualityFactor = 1.0f; // 0.5=fast, 1.0=quality
};

/// Negotiate parameters for thumbnail output based on context and source.
///
/// Usage:
///   FormatNegotiator negotiator;
///   negotiator.SetShellContext(ShellContext::ExplorerView);
///   negotiator.SetRequestedSize(256);
///   auto output = negotiator.Negotiate(sourceWidth, sourceHeight, sourceHasAlpha, sourceIsHDR);
///
class FormatNegotiator {
public:
    FormatNegotiator() { DetectDisplayCapabilities(); }

    /// Set the requesting shell context
    void SetShellContext(ShellContext context) { m_context = context; }

    /// Set the requested thumbnail size
    void SetRequestedSize(uint32_t size) { m_requestedSize = size; }

    /// Set quality preference (0.0 = fastest, 1.0 = best quality)
    void SetQualityPreference(float quality) {
        m_qualityPref = std::clamp(quality, 0.0f, 1.0f);
    }

    /// Negotiate the optimal output format
    NegotiatedOutput Negotiate(uint32_t sourceWidth, uint32_t sourceHeight,
        bool sourceHasAlpha = false, bool sourceIsHDR = false,
        bool sourceIsWideGamut = false) {
        NegotiatedOutput out;

        // Determine output dimensions (maintain aspect ratio)
        uint32_t targetSize = GetEffectiveSize();
        if (sourceWidth == 0 || sourceHeight == 0) {
            out.width = out.height = targetSize;
        }
        else if (sourceWidth >= sourceHeight) {
            out.width = targetSize;
            out.height = (std::max)(1u, targetSize * sourceHeight / sourceWidth);
        }
        else {
            out.height = targetSize;
            out.width = (std::max)(1u, targetSize * sourceWidth / sourceHeight);
        }

        // DPI-aware scaling
        out.dpi = GetEffectiveDPI();
        if (out.dpi > 96) {
            float scale = static_cast<float>(out.dpi) / 96.0f;
            out.width = static_cast<uint32_t>(out.width * scale);
            out.height = static_cast<uint32_t>(out.height * scale);
        }

        // Determine pixel format
        out.needsAlpha = sourceHasAlpha || RequiresAlpha(m_context);
        if (sourceIsHDR && m_supportsHDR) {
            out.pixelFormat = OutputPixelFormat::RGBAF16;
            out.needsHDRTonemap = !m_supportsHDR; // Tonemap if display doesn't support HDR
        }
        else if (out.needsAlpha) {
            out.pixelFormat = OutputPixelFormat::BGRA32;
            out.needsPremulAlpha = (m_context == ShellContext::ExplorerView);
        }
        else {
            out.pixelFormat = OutputPixelFormat::BGRA32; // Always 32-bit for shell compatibility
        }

        // Color space
        if (sourceIsWideGamut && m_supportsWideGamut) {
            out.colorSpace = OutputColorSpace::DisplayP3;
        }
        else {
            out.colorSpace = OutputColorSpace::sRGB;
        }

        // Stride calculation
        out.stride = out.width * GetBytesPerPixel(out.pixelFormat);
        // Align to 4 bytes
        out.stride = (out.stride + 3) & ~3u;

        // Quality factor based on context
        out.qualityFactor = m_qualityPref;
        if (m_context == ShellContext::PreviewPane) out.qualityFactor = 1.0f;
        if (m_context == ShellContext::ThumbnailCache) out.qualityFactor = 0.8f;

        return out;
    }

    /// Get bytes per pixel for a format
    static uint32_t GetBytesPerPixel(OutputPixelFormat fmt) {
        switch (fmt) {
        case OutputPixelFormat::BGRA32:
        case OutputPixelFormat::BGRA32_PreAlpha:
        case OutputPixelFormat::RGBA32:     return 4;
        case OutputPixelFormat::BGR24:      return 3;
        case OutputPixelFormat::Gray8:      return 1;
        case OutputPixelFormat::Gray16:     return 2;
        case OutputPixelFormat::RGBAF16:    return 8;
        default:                            return 4;
        }
    }

    /// Get the display DPI
    uint32_t GetEffectiveDPI() const { return m_displayDPI; }

    /// Check if display supports HDR
    bool SupportsHDR() const { return m_supportsHDR; }

    /// Check if display supports wide gamut
    bool SupportsWideGamut() const { return m_supportsWideGamut; }

private:
    ShellContext    m_context = ShellContext::ExplorerView;
    uint32_t        m_requestedSize = 256;
    float           m_qualityPref = 1.0f;
    uint32_t        m_displayDPI = 96;
    bool            m_supportsHDR = false;
    bool            m_supportsWideGamut = false;

    uint32_t GetEffectiveSize() const {
        switch (m_context) {
        case ShellContext::PreviewPane:     return (std::max)(m_requestedSize, 512u);
        case ShellContext::TaskbarPreview:  return (std::min)(m_requestedSize, 200u);
        case ShellContext::StartMenu:       return (std::min)(m_requestedSize, 150u);
        default:                           return m_requestedSize;
        }
    }

    bool RequiresAlpha(ShellContext context) const {
        // Explorer view needs alpha for overlay icons
        return context == ShellContext::ExplorerView ||
            context == ShellContext::TaskbarPreview;
    }

    void DetectDisplayCapabilities() {
        // Get system DPI
        HDC hdc = GetDC(nullptr);
        if (hdc) {
            m_displayDPI = static_cast<uint32_t>(GetDeviceCaps(hdc, LOGPIXELSX));
            // Check color depth for wide gamut hint
            int bitsPerPixel = GetDeviceCaps(hdc, BITSPIXEL);
            m_supportsWideGamut = (bitsPerPixel >= 30);
            ReleaseDC(nullptr, hdc);
        }

        // HDR detection would require DXGI adapter enumeration
        // For now, default to false
        m_supportsHDR = false;
    }
};

} // namespace Engine
} // namespace ExplorerLens
