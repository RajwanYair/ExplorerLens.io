// DynamicFormatRouter.h — Runtime format detection bypassing extension-based routing
// Copyright (c) 2026 ExplorerLens Project
//
// Detects actual file format by reading magic bytes, enabling correct decode
// of misnamed files where the extension doesn't match the content.
//
#pragma once
#include <string>
#include <cstdint>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct DynamicFormatRouterConfig {
    bool enabled = true;
    uint32_t magicBytesRead = 16;
    bool fallbackToExtension = true;
    std::string label = "DynamicFormatRouter";
};

class DynamicFormatRouter {
public:
    bool Initialize() {
        if (m_initialized) return true;
        RegisterBuiltinSignatures();
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    DynamicFormatRouterConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    struct FormatSignature {
        std::vector<uint8_t> magic;
        uint32_t offset = 0;
        std::string formatName;
    };

    std::string DetectFormat(const uint8_t* header, size_t len) const {
        for (const auto& sig : m_signatures) {
            if (sig.offset + sig.magic.size() <= len) {
                bool match = true;
                for (size_t i = 0; i < sig.magic.size() && match; ++i)
                    match = (header[sig.offset + i] == sig.magic[i]);
                if (match) return sig.formatName;
            }
        }
        return "unknown";
    }

    uint32_t GetSignatureCount() const {
        return static_cast<uint32_t>(m_signatures.size());
    }

private:
    void RegisterBuiltinSignatures() {
        m_signatures.push_back({ {0x50, 0x4B, 0x03, 0x04}, 0, "zip" });
        m_signatures.push_back({ {0x89, 0x50, 0x4E, 0x47}, 0, "png" });
        m_signatures.push_back({ {0xFF, 0xD8, 0xFF}, 0, "jpeg" });
        m_signatures.push_back({ {0x52, 0x49, 0x46, 0x46}, 0, "riff" });
        m_signatures.push_back({ {0x25, 0x50, 0x44, 0x46}, 0, "pdf" });
        m_signatures.push_back({ {0x47, 0x49, 0x46, 0x38}, 0, "gif" });
    }

    bool m_initialized = false;
    DynamicFormatRouterConfig m_config;
    std::vector<FormatSignature> m_signatures;
};

}
} // namespace ExplorerLens::Engine
