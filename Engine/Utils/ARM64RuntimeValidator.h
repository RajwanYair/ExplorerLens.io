#pragma once
// Sprint 157 — ARM64 Runtime Validator
// Platform probe, decoder availability, GPU capability, and shell registration
// verification for ARM64 target.

#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs::Platform {

// ─── Platform probe ──────────────────────────────────────────────────────────

struct PlatformProbeResult {
    std::string     osVersion;
    std::string     processorArch;       // from SYSTEM_INFO
    bool            isNativeARM64       { false };
    bool            isARM64EC           { false };
    bool            isX64OnARM64        { false };   // x64 emulation
    bool            isWow64             { false };
    uint32_t        cpuCoreCount        { 0 };
    uint64_t        availableMemoryMB   { 0 };

    std::string Summary() const {
        return "Platform: " + processorArch + " | OS: " + osVersion +
               " | Native=" + (isNativeARM64 ? "yes" : "no") +
               " | Cores=" + std::to_string(cpuCoreCount);
    }
};

// ─── Decoder confidence test ─────────────────────────────────────────────────

enum class DecoderConfidenceLevel : uint32_t {
    FullPass    = 0,   // decode produced correct output
    Partial     = 1,   // decode succeeded but with degraded quality
    Fallback    = 2,   // used alternative decode path
    Skip        = 3,   // decoder not available on ARM64
    Fail        = 4,
};

inline std::string ToString(DecoderConfidenceLevel l) {
    switch (l) {
        case DecoderConfidenceLevel::FullPass: return "FullPass";
        case DecoderConfidenceLevel::Partial:  return "Partial";
        case DecoderConfidenceLevel::Fallback: return "Fallback";
        case DecoderConfidenceLevel::Skip:     return "Skip";
        case DecoderConfidenceLevel::Fail:     return "Fail";
        default: return "Unknown";
    }
}

struct DecoderConfidenceEntry {
    std::string             extension;
    std::string             decoderName;
    DecoderConfidenceLevel  level       { DecoderConfidenceLevel::Fail };
    double                  decodeMs    { 0.0 };
    std::string             notes;

    bool Passed() const {
        return level == DecoderConfidenceLevel::FullPass ||
               level == DecoderConfidenceLevel::Partial  ||
               level == DecoderConfidenceLevel::Fallback;
    }
};

// ─── GPU capability on ARM64 ──────────────────────────────────────────────────

struct ARM64GPUCapability {
    bool        d3d11Available  { false };
    bool        d3d12Available  { false };
    std::string adapterName;
    uint64_t    dedicatedVRAMMB { 0 };
    bool        isIntegratedGPU { true };

    bool CanRenderThumbnails() const { return d3d11Available || d3d12Available; }
    std::string Summary() const {
        return "GPU: " + adapterName + " | D3D11=" + (d3d11Available ? "yes" : "no") +
               " D3D12=" + (d3d12Available ? "yes" : "no");
    }
};

// ─── COM registration check ──────────────────────────────────────────────────

struct COMRegistrationCheck {
    std::string clsid           { "9E6ECB90-5A61-42BD-B851-D3297D9C7F39" };
    bool        isRegistered    { false };
    std::string registeredPath;
    bool        pathExists      { false };

    bool IsValid() const { return isRegistered && pathExists; }
};

// ─── Runtime validation report ───────────────────────────────────────────────

struct ARM64RuntimeValidationReport {
    PlatformProbeResult                 platform;
    ARM64GPUCapability                  gpu;
    COMRegistrationCheck                comRegistration;
    std::vector<DecoderConfidenceEntry> decoderTests;
    uint32_t                            decoderPassCount { 0 };
    uint32_t                            decoderFailCount { 0 };

    static constexpr uint32_t kMinDecoderPassCount = 5;  // JPEG/PNG/WebP/HEIF/RAW

    bool MeetsCriteria() const {
        return decoderPassCount >= kMinDecoderPassCount &&
               gpu.CanRenderThumbnails();
    }

    std::string Summary() const {
        return "ARM64 Runtime: decoders=" + std::to_string(decoderPassCount) + "/" +
               std::to_string(decoderTests.size()) + " | GPU=" +
               (gpu.CanRenderThumbnails() ? "OK" : "NoGPU") + " | COM=" +
               (comRegistration.IsValid() ? "OK" : "NotReg") + " — " +
               (MeetsCriteria() ? "PASS" : "FAIL");
    }

    static ARM64RuntimeValidationReport CreateMock() {
        ARM64RuntimeValidationReport r;
        r.platform = { "Windows 11 24H2", "ARM64", true, false, false, false, 8, 8192 };
        r.gpu = { true, true, "Qualcomm Adreno 690", 0, true };
        r.comRegistration = { "9E6ECB90-5A61-42BD-B851-D3297D9C7F39", true,
                              "C:\\Windows\\System32\\CBXShell.dll", true };
        r.decoderTests = {
            { ".jpg",  "ImageDecoder",  DecoderConfidenceLevel::FullPass, 12.0, "" },
            { ".png",  "ImageDecoder",  DecoderConfidenceLevel::FullPass, 9.0,  "" },
            { ".webp", "WebPDecoder",   DecoderConfidenceLevel::FullPass, 15.0, "" },
            { ".heic", "HEIFDecoder",   DecoderConfidenceLevel::FullPass, 22.0, "" },
            { ".raw",  "RAWDecoder",    DecoderConfidenceLevel::FullPass, 45.0, "" },
        };
        r.decoderPassCount = 5;
        r.decoderFailCount = 0;
        return r;
    }
};

} // namespace DarkThumbs::Platform
