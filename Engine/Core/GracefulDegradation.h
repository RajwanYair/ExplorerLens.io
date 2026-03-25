// GracefulDegradation.h — Failure Mode Catalog and Fallback Orchestrator
// Copyright (c) 2026 ExplorerLens Project
//
// Sprint 13 (v15.3.0 "Zenith-T"): Catalogs the six canonical failure modes for
// the decode pipeline and exposes a uniform Degrade() entry point.  Each mode
// maps to a documented fallback strategy so all callers behave consistently when
// a decoder cannot produce a valid thumbnail.
//
#pragma once

#include <functional>
#include <string_view>

namespace ExplorerLens {
namespace Engine {

enum class DegradationMode
{
    NullBitmap,           // Return an empty transparent bitmap
    PlaceholderIcon,      // Return the Windows file-type icon (shell fallback)
    CorruptFileOverlay,   // Return a placeholder with a "corrupt" badge overlay
    TimeoutFallback,      // Return a clock/timeout badge overlay
    PasswordProtected,    // Return a lock-icon placeholder
    UnsupportedFormat     // Return a question-mark placeholder
};

struct DegradationResult
{
    DegradationMode mode     = DegradationMode::NullBitmap;
    bool            occupied = false;
};

using DegradationHandler = std::function<DegradationResult(DegradationMode)>;

class GracefulDegradation
{
public:
    static constexpr int MODE_COUNT = 6;

    static std::string_view ModeName(DegradationMode mode) noexcept
    {
        switch (mode)
        {
            case DegradationMode::NullBitmap:         return "NullBitmap";
            case DegradationMode::PlaceholderIcon:    return "PlaceholderIcon";
            case DegradationMode::CorruptFileOverlay: return "CorruptFileOverlay";
            case DegradationMode::TimeoutFallback:    return "TimeoutFallback";
            case DegradationMode::PasswordProtected:  return "PasswordProtected";
            case DegradationMode::UnsupportedFormat:  return "UnsupportedFormat";
            default:                                  return "Unknown";
        }
    }

    static DegradationResult Degrade(
        DegradationMode mode,
        const DegradationHandler& customHandler = nullptr) noexcept
    {
        if (customHandler)
        {
            try { return customHandler(mode); }
            catch (...) { /* fall through to default */ }
        }
        return DefaultDegrade(mode);
    }

    static DegradationResult InjectFault(DegradationMode mode) noexcept
    {
        return Degrade(mode);
    }

private:
    static DegradationResult DefaultDegrade(DegradationMode mode) noexcept
    {
        DegradationResult result;
        result.mode     = mode;
        result.occupied = true;
        return result;
    }
};

} // namespace Engine
} // namespace ExplorerLens
