// ShaderCompileListener.h — Monitors async shader compilation progress
// Copyright (c) 2026 ExplorerLens Project
//
// Receives callbacks from the shader compilation pipeline, tracking
// compilation status, errors, and timing for PSO creation diagnostics.
//
#pragma once
#include <string>
#include <cstdint>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct ShaderCompileListenerConfig {
    bool enabled = true;
    uint32_t maxTrackedCompiles = 256;
    std::string label = "ShaderCompileListener";
};

class ShaderCompileListener {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    ShaderCompileListenerConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    struct CompileResult {
        std::string shaderName;
        bool success = false;
        double compileTimeMs = 0.0;
        std::string errorMsg;
    };

    bool RecordResult(const CompileResult& r) {
        if (m_results.size() >= m_config.maxTrackedCompiles) return false;
        m_results.push_back(r);
        if (r.success) m_successCount++;
        return true;
    }

    size_t GetTotalCompiles() const { return m_results.size(); }
    uint32_t GetSuccessCount() const { return m_successCount; }

private:
    bool m_initialized = false;
    ShaderCompileListenerConfig m_config;
    std::vector<CompileResult> m_results;
    uint32_t m_successCount = 0;
};

}
} // namespace ExplorerLens::Engine
