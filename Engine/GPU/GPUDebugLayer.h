// GPUDebugLayer.h — GPU Diagnostic and Validation Layer
// Copyright (c) 2026 ExplorerLens Project
//
// Wraps D3D11/D3D12 debug layers and Vulkan validation layers to capture
// GPU errors, resource leaks, and performance warnings during development.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class GPUDebugSeverity : uint8_t {
    Info, Warning, Error, Corruption, COUNT
};

enum class GPUDebugCategory : uint8_t {
    ResourceLeak, StateError, PerformanceWarning, ShaderError, Miscellaneous, COUNT
};

struct GPUDebugMessage {
    GPUDebugSeverity severity = GPUDebugSeverity::Info;
    GPUDebugCategory category = GPUDebugCategory::Miscellaneous;
    uint32_t id = 0;
    std::wstring description;
    uint64_t timestampUs = 0;
};

struct GPUDebugStats {
    uint32_t totalMessages = 0;
    uint32_t errorCount = 0;
    uint32_t warningCount = 0;
    uint32_t leakCount = 0;
    bool debugLayerActive = false;
};

class GPUDebugLayer {
public:
    void Enable(bool enable) { m_enabled = enable; }
    bool IsEnabled() const { return m_enabled; }

    void SetBreakOnError(bool brk) { m_breakOnError = brk; }
    bool GetBreakOnError() const { return m_breakOnError; }

    void RecordMessage(const GPUDebugMessage& msg) {
        m_messages.push_back(msg);
        m_stats.totalMessages++;
        if (msg.severity == GPUDebugSeverity::Error) m_stats.errorCount++;
        if (msg.severity == GPUDebugSeverity::Warning) m_stats.warningCount++;
        if (msg.category == GPUDebugCategory::ResourceLeak) m_stats.leakCount++;
    }

    const std::vector<GPUDebugMessage>& GetMessages() const { return m_messages; }
    const GPUDebugStats& GetStats() const { return m_stats; }

    void Clear() {
        m_messages.clear();
        m_stats = {};
        m_stats.debugLayerActive = m_enabled;
    }

    bool HasErrors() const { return m_stats.errorCount > 0; }
    bool HasLeaks() const { return m_stats.leakCount > 0; }

    static size_t SeverityCount() { return static_cast<size_t>(GPUDebugSeverity::COUNT); }
    static size_t CategoryCount() { return static_cast<size_t>(GPUDebugCategory::COUNT); }

private:
    bool m_enabled = false;
    bool m_breakOnError = false;
    std::vector<GPUDebugMessage> m_messages;
    GPUDebugStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
