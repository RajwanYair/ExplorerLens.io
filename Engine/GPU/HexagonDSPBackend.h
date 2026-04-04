// HexagonDSPBackend.h — Qualcomm Hexagon DSP Inference Backend (ONNX QNN EP)
// Copyright (c) 2026 ExplorerLens Project
//
// Stub backend routing inference to the Qualcomm Hexagon DSP via the ONNX
// Runtime QNN Execution Provider on Snapdragon-class hardware.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class HexagonVariant {
    V65,
    V66,
    V68,
    V69,
    V73,
    Unknown
};
enum class QNNBackend {
    HTP,
    HTA,
    CPU
};

struct HexagonDSPConfig
{
    HexagonVariant variant = HexagonVariant::Unknown;
    QNNBackend preferredBE = QNNBackend::HTP;
    bool enableHVX = true;
    bool enableHMX = false;
    bool cacheQnnContext = true;
    uint32_t soclockMhz = 0;
};

struct HexagonRunResult
{
    bool success = false;
    uint64_t latencyUs = 0;
    float powerMw = 0.0f;
    std::string errorMsg;
};

class HexagonDSPBackend
{
  public:
    explicit HexagonDSPBackend(const HexagonDSPConfig& cfg = {}) : m_cfg(cfg) {}

    bool Initialize()
    {
        m_ready = true;
        return true;
    }
    bool IsReady() const
    {
        return m_ready;
    }
    HexagonVariant GetVariant() const
    {
        return m_cfg.variant;
    }
    QNNBackend GetQNNBackend() const
    {
        return m_cfg.preferredBE;
    }
    bool HVXEnabled() const
    {
        return m_cfg.enableHVX;
    }
    bool HMXEnabled() const
    {
        return m_cfg.enableHMX;
    }

    HexagonRunResult RunInference(const uint8_t* input, size_t inputSize, uint8_t* output, size_t outputSize)
    {
        (void)input;
        (void)output;
        (void)outputSize;
        HexagonRunResult r;
        if (!m_ready || !input || inputSize == 0) {
            r.errorMsg = "Not ready";
            return r;
        }
        r.success = true;
        r.latencyUs = 800;
        r.powerMw = 150.0f;
        ++m_runCount;
        return r;
    }

    uint32_t RunCount() const
    {
        return m_runCount;
    }
    const HexagonDSPConfig& GetConfig() const
    {
        return m_cfg;
    }
    void SetConfig(const HexagonDSPConfig& cfg)
    {
        m_cfg = cfg;
    }
    void Reset()
    {
        m_ready = false;
        m_runCount = 0;
    }

  private:
    HexagonDSPConfig m_cfg;
    bool m_ready = false;
    uint32_t m_runCount = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
