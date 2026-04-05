// NPUThumbnailSynthesizer.h — NPU-accelerated thumbnail synthesis from text prompt
// Copyright (c) 2026 ExplorerLens Project
//
// Generates placeholder thumbnails using on-device NPU/DirectML inference when
// the source file cannot be decoded (corrupt, unsupported format, DRM-protected).
// Uses a lightweight diffusion model stub that routes to DirectML, ONNX, or CPU.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class SynthesisBackend : uint8_t
{
    DirectML = 0,
    ONNX     = 1,
    CPU      = 2,
};

struct SynthesisRequest
{
    std::string  prompt;
    uint32_t     width    = 256;
    uint32_t     height   = 256;
    uint32_t     steps    = 4;
    float        guidance = 3.0f;
};

struct SynthesisResult
{
    bool                 success   = false;
    std::vector<uint8_t> pixels;
    uint32_t             width     = 0;
    uint32_t             height    = 0;
    float                inferenceMs = 0.0f;
    std::string          errorMsg;
};

class NPUThumbnailSynthesizer
{
public:
    NPUThumbnailSynthesizer();
    ~NPUThumbnailSynthesizer();

    NPUThumbnailSynthesizer(const NPUThumbnailSynthesizer&)            = delete;
    NPUThumbnailSynthesizer& operator=(const NPUThumbnailSynthesizer&) = delete;

    bool              Initialize(SynthesisBackend backend = SynthesisBackend::DirectML);
    void              Shutdown();
    SynthesisResult   Synthesize(const SynthesisRequest& req);
    SynthesisBackend  GetBackend()    const noexcept { return m_backend; }
    const char*       BackendName()   const noexcept;
    uint64_t          SynthesisCount() const noexcept { return m_count; }
    bool              IsReady()       const noexcept { return m_initialized; }

    static NPUThumbnailSynthesizer& Instance() noexcept;

private:
    SynthesisBackend          m_backend     = SynthesisBackend::CPU;
    bool                      m_initialized = false;
    uint64_t                  m_count       = 0;
    static NPUThumbnailSynthesizer s_instance;
};

}} // namespace ExplorerLens::Engine
