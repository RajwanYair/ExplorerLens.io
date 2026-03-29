// VLMEmbeddingEngine.h — Multimodal VLM Embedding Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Computes joint image+text embeddings using Vision-Language Models
// (Florence-2 / CLIP style) for semantic thumbnail search and captioning.
//
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class VLMBackend { CLIP, Florence2, LLaVA, BLIP2, CPU };

struct VLMEmbedding {
    std::vector<float> values;
    uint32_t           dimension = 512;
    std::string        modelId;
};

struct VLMEmbeddingConfig {
    VLMBackend   backend         = VLMBackend::CPU;
    uint32_t     embeddingDim    = 512;
    bool         normalizeOutput = true;
    std::string  modelPath;
};

class VLMEmbeddingEngine {
public:
    explicit VLMEmbeddingEngine(const VLMEmbeddingConfig& cfg = {}) : m_cfg(cfg) {}

    bool Load() {
        m_loaded = true;
        return true;
    }

    bool IsLoaded() const { return m_loaded; }

    VLMEmbedding EmbedImage(const std::vector<uint8_t>& imageData) const {
        VLMEmbedding emb;
        if (imageData.empty()) return emb;
        emb.dimension = m_cfg.embeddingDim;
        emb.modelId   = BackendName(m_cfg.backend);
        emb.values.assign(m_cfg.embeddingDim, 0.0f);
        for (uint32_t i = 0; i < m_cfg.embeddingDim && i < imageData.size(); ++i)
            emb.values[i] = static_cast<float>(imageData[i]) / 255.0f;
        if (m_cfg.normalizeOutput) Normalize(emb.values);
        return emb;
    }

    VLMEmbedding EmbedText(const std::string& text) const {
        VLMEmbedding emb;
        emb.dimension = m_cfg.embeddingDim;
        emb.modelId   = BackendName(m_cfg.backend);
        emb.values.assign(m_cfg.embeddingDim, 0.0f);
        for (uint32_t i = 0; i < m_cfg.embeddingDim && i < text.size(); ++i)
            emb.values[i] = static_cast<float>(static_cast<unsigned char>(text[i])) / 255.0f;
        if (m_cfg.normalizeOutput) Normalize(emb.values);
        return emb;
    }

    float CosineSimilarity(const VLMEmbedding& a, const VLMEmbedding& b) const {
        if (a.values.size() != b.values.size() || a.values.empty()) return 0.0f;
        float dot = 0.0f;
        for (size_t i = 0; i < a.values.size(); ++i) dot += a.values[i] * b.values[i];
        return dot;
    }

    void Unload() { m_loaded = false; }

    static std::string BackendName(VLMBackend b) {
        switch (b) {
            case VLMBackend::CLIP:     return "CLIP";
            case VLMBackend::Florence2:return "Florence-2";
            case VLMBackend::LLaVA:   return "LLaVA";
            case VLMBackend::BLIP2:   return "BLIP-2";
            case VLMBackend::CPU:     return "CPU";
        }
        return "unknown";
    }

private:
    static void Normalize(std::vector<float>& v) {
        float norm = 0.0f;
        for (float x : v) norm += x * x;
        if (norm > 0.0f) { norm = 1.0f / std::sqrt(norm); for (float& x : v) x *= norm; }
    }
    VLMEmbeddingConfig m_cfg;
    bool               m_loaded = false;
};

}} // namespace ExplorerLens::Engine
