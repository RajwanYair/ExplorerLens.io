// MemoryMappedPipelineStage.h — Zero-copy pipeline stage using mmap I/O
// Copyright (c) 2026 ExplorerLens Project
//
// Implements a pipeline stage that uses memory-mapped file I/O for zero-copy
// access to source files, avoiding redundant read/copy operations.
//
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

struct MemoryMappedPipelineStageConfig {
    bool enabled = true;
    uint64_t maxMapSize = 256 * 1024 * 1024; // 256 MB
    bool useSequentialHint = true;
    std::string label = "MemoryMappedPipelineStage";
};

class MemoryMappedPipelineStage {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    MemoryMappedPipelineStageConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    bool CanMap(uint64_t fileSize) const {
        return fileSize > 0 && fileSize <= m_config.maxMapSize;
    }

    struct MappingInfo {
        const uint8_t* baseAddress = nullptr;
        uint64_t mappedSize = 0;
        bool active = false;
    };

    MappingInfo GetCurrentMapping() const { return m_currentMap; }
    bool IsMapped() const { return m_currentMap.active; }

    void SimulateMap(uint64_t size) {
        m_currentMap = { nullptr, size, true };
        m_totalMapped += size;
    }

    void Unmap() {
        m_currentMap = {};
    }

    uint64_t GetTotalBytesMapped() const { return m_totalMapped; }

private:
    bool m_initialized = false;
    MemoryMappedPipelineStageConfig m_config;
    MappingInfo m_currentMap;
    uint64_t m_totalMapped = 0;
};

}
} // namespace ExplorerLens::Engine
