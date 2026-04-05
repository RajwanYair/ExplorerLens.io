// CLIPQueryProcessor.h — Natural language to CLIP embedding query processor
// Copyright (c) 2026 ExplorerLens Project
//
// Converts a natural-language text query ("sunset over water") into a 512-
// dimension CLIP ViT-B/32 text embedding via DirectML or ONNX Runtime, then
// dispatches the embedding to HNSWIndexEngine for ANN retrieval. GPU-backed
// tokenizer runs on Intel NPU (INT8) when available, CPU otherwise.
//
#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class CLIPTextBackend : uint8_t { DIRECTML, ONNX, CPU };

struct CLIPQueryRequest {
    std::wstring    queryText;
    uint32_t        topK      = 10;
    float           minScore  = 0.20f;
    CLIPTextBackend backend   = CLIPTextBackend::DIRECTML;
};

struct CLIPQueryMatch {
    std::wstring filePath;
    float        score    = 0.0f;
    uint32_t     itemId   = 0;
};

class CLIPQueryProcessor {
public:
    static CLIPQueryProcessor& Instance();

    bool IsModelLoaded()                              const noexcept { return m_modelLoaded; }
    bool LoadModel(const std::wstring& modelPath);
    std::vector<CLIPQueryMatch> Query(const CLIPQueryRequest& req) const;
    float LastEmbedMs()                               const noexcept { return m_lastEmbedMs; }

    static std::string_view BackendName(CLIPTextBackend b) noexcept;

private:
    bool          m_modelLoaded = false;
    mutable float m_lastEmbedMs = 0.0f;
    char  m_modelVersion[32] = "ViT-B/32-INT8";
};

}} // namespace ExplorerLens::Engine
