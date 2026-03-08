// FileSizeEstimator.h — Quick archive size estimation without full extraction
// Copyright (c) 2026 ExplorerLens Project
//
// Reads archive headers to estimate uncompressed content size and file count
// without performing full extraction, enabling informed decode decisions.
//
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

struct FileSizeEstimatorConfig {
    bool enabled = true;
    uint64_t maxHeaderRead = 4096;
    std::string label = "FileSizeEstimator";
};

class FileSizeEstimator {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    FileSizeEstimatorConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    struct SizeEstimate {
        uint64_t compressedSize = 0;
        uint64_t uncompressedSize = 0;
        uint32_t fileCount = 0;
        double compressionRatio = 1.0;
        bool isEstimate = true;
    };

    SizeEstimate EstimateFromHeader(const uint8_t* header, size_t len) const {
        SizeEstimate est;
        if (len >= 30 && header[0] == 0x50 && header[1] == 0x4B) {
            // ZIP local file header — read compressed/uncompressed sizes
            est.compressedSize = *reinterpret_cast<const uint32_t*>(header + 18);
            est.uncompressedSize = *reinterpret_cast<const uint32_t*>(header + 22);
            est.fileCount = 1;
            if (est.compressedSize > 0)
                est.compressionRatio = static_cast<double>(est.uncompressedSize) / est.compressedSize;
        }
        return est;
    }

    bool IsLargeArchive(const SizeEstimate& est, uint64_t threshold = 100 * 1024 * 1024) const {
        return est.uncompressedSize > threshold;
    }

private:
    bool m_initialized = false;
    FileSizeEstimatorConfig m_config;
};

}
} // namespace ExplorerLens::Engine
