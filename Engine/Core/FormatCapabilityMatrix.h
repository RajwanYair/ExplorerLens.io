// FormatCapabilityMatrix.h — Runtime Format x Platform x Decoder Capability Matrix
// Copyright (c) 2026 ExplorerLens Project
//
// Queryable matrix mapping every supported format to its capability level on each
// platform, enabling runtime feature negotiation and graceful degradation.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

enum class CapabilityLevel : uint8_t {
    None = 0,
    Basic = 1,
    Full = 2,
    GPUAccelerated = 3
};

enum class PlatformType : uint8_t {
    Windows = 0,
    Linux = 1,
    macOS = 2,
    WASM = 3,
    Count = 4
};

struct FormatCapability {
    std::string formatName;
    PlatformType platform = PlatformType::Windows;
    CapabilityLevel level = CapabilityLevel::None;
    std::string decoderName;
    std::string minVersion;
    uint32_t estimatedDecodeMs = 0;
    bool requiresExternalLib = false;

    bool IsAvailable() const { return level != CapabilityLevel::None; }
    bool IsGPU() const { return level == CapabilityLevel::GPUAccelerated; }
};

class FormatCapabilityMatrix {
public:
    FormatCapabilityMatrix() : m_strictMode(false) {
        PopulateDefaults();
    }

    ~FormatCapabilityMatrix() = default;

    void RegisterCapability(const FormatCapability& cap) {
        m_capabilities.push_back(cap);
    }

    CapabilityLevel QueryCapability(const std::string& format, PlatformType platform) const {
        for (const auto& cap : m_capabilities)
            if (cap.formatName == format && cap.platform == platform)
                return cap.level;
        return CapabilityLevel::None;
    }

    std::vector<FormatCapability> GetPlatformFormats(PlatformType platform) const {
        std::vector<FormatCapability> result;
        for (const auto& cap : m_capabilities)
            if (cap.platform == platform && cap.IsAvailable())
                result.push_back(cap);
        return result;
    }

    std::string ExportToJson() const {
        std::string json = "{\n  \"capabilities\": [\n";
        for (size_t i = 0; i < m_capabilities.size(); ++i) {
            const auto& c = m_capabilities[i];
            json += "    {\"format\":\"" + c.formatName +
                    "\",\"platform\":" + std::to_string(static_cast<int>(c.platform)) +
                    ",\"level\":" + std::to_string(static_cast<int>(c.level)) +
                    ",\"decoder\":\"" + c.decoderName + "\"}";
            if (i + 1 < m_capabilities.size()) json += ",";
            json += "\n";
        }
        json += "  ],\n  \"totalFormats\":" + std::to_string(GetTotalFormatCount()) + "\n}";
        return json;
    }

    std::vector<FormatCapability> GetGPUAcceleratedFormats() const {
        std::vector<FormatCapability> result;
        for (const auto& cap : m_capabilities)
            if (cap.IsGPU())
                result.push_back(cap);
        return result;
    }

    size_t GetTotalFormatCount() const {
        std::vector<std::string> unique;
        for (const auto& cap : m_capabilities) {
            if (std::find(unique.begin(), unique.end(), cap.formatName) == unique.end())
                unique.push_back(cap.formatName);
        }
        return unique.size();
    }

    void SetStrictMode(bool strict) { m_strictMode = strict; }
    size_t GetEntryCount() const { return m_capabilities.size(); }

private:
    void PopulateDefaults() {
        auto add = [this](const std::string& fmt, const std::string& dec,
                          CapabilityLevel winLvl, CapabilityLevel linLvl) {
            m_capabilities.push_back({fmt, PlatformType::Windows, winLvl, dec, "1.0.0", 15, false});
            m_capabilities.push_back({fmt, PlatformType::Linux,   linLvl, dec, "1.0.0", 18, false});
        };
        add("JPEG", "LibJPEG-Turbo", CapabilityLevel::GPUAccelerated, CapabilityLevel::Full);
        add("PNG",  "LibPNG",        CapabilityLevel::Full,            CapabilityLevel::Full);
        add("WebP", "LibWebP",       CapabilityLevel::GPUAccelerated, CapabilityLevel::Full);
        add("AVIF", "LibAVIF",       CapabilityLevel::GPUAccelerated, CapabilityLevel::Basic);
        add("JXL",  "LibJXL",        CapabilityLevel::Full,            CapabilityLevel::Full);
        add("HEIF", "LibHEIF",       CapabilityLevel::Full,            CapabilityLevel::Basic);
        add("PDF",  "MuPDF",         CapabilityLevel::Full,            CapabilityLevel::Full);
        add("RAW",  "LibRaw",        CapabilityLevel::Full,            CapabilityLevel::Full);
    }

    std::vector<FormatCapability> m_capabilities;
    bool m_strictMode;
};

}} // namespace ExplorerLens::Engine
