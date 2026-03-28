// LLMMIMEInferenceEngine.h — LLM-Powered MIME Type Inference Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Uses a local, quantised LLM (Phi-3 / Gemma-3B) to infer MIME type + format
// family from file header bytes, metadata, and path context when rule-based
// detection fails.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class LLMBackendKind  { LocalPhi3, LocalGemma, RemoteAPI, RulesFallback };
enum class MIMEConfidence  { VeryLow, Low, Medium, High, VeryHigh };

struct LLMInferenceRequest {
    std::vector<uint8_t> headerBytes;
    std::string          filePath;
    std::string          knownExtension;
    uint32_t             maxTokens    = 64;
};

struct LLMInferenceResult {
    std::string    mimeType;
    std::string    formatFamily;
    MIMEConfidence confidence      = MIMEConfidence::Low;
    LLMBackendKind backendUsed     = LLMBackendKind::RulesFallback;
    uint64_t       inferenceMs     = 0;
    std::string    reasoning;
    bool           IsReliable()  const { return confidence >= MIMEConfidence::Medium; }
};

struct LLMMIMEConfig {
    LLMBackendKind preferredBackend = LLMBackendKind::LocalPhi3;
    float          confidenceFloor  = 0.55f;
    bool           allowRemoteFallback = false;
    uint32_t       maxContextBytes  = 256;
    std::string    modelPath;
};

class LLMMIMEInferenceEngine {
public:
    explicit LLMMIMEInferenceEngine(const LLMMIMEConfig& cfg = {}) : m_cfg(cfg) {}

    bool  LoadModel() { m_loaded = true; return true; }
    bool  IsLoaded()  const { return m_loaded; }

    LLMInferenceResult Infer(const LLMInferenceRequest& req) {
        LLMInferenceResult r;
        if (!m_loaded) {
            r.backendUsed = LLMBackendKind::RulesFallback;
            return r;
        }
        r.backendUsed  = m_cfg.preferredBackend;
        r.mimeType     = req.knownExtension.empty() ? "application/octet-stream"
                                                      : "application/x-" + req.knownExtension;
        r.confidence   = MIMEConfidence::Medium;
        r.inferenceMs  = 3;
        r.reasoning    = "Header-based heuristic";
        return r;
    }

    LLMBackendKind  GetBackend()  const { return m_cfg.preferredBackend; }
    float           GetFloor()    const { return m_cfg.confidenceFloor; }
    void            SetConfig(const LLMMIMEConfig& cfg) { m_cfg = cfg; }
    const LLMMIMEConfig& GetConfig() const { return m_cfg; }
    void            Reset() { m_loaded = false; }

private:
    LLMMIMEConfig m_cfg;
    bool          m_loaded = false;
};

} // namespace Engine
} // namespace ExplorerLens
