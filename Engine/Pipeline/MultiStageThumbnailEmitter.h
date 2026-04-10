// MultiStageThumbnailEmitter.h — Progressive Multi-Stage Thumbnail Emitter
// Copyright (c) 2026 ExplorerLens Project
//
// Emits thumbnails at increasing fidelity: placeholder (blurred 64×64) → low-res
// (256×256 fast decode) → full-res (up to 512×512 high-quality decode). Each stage
// completes independently so the shell can render immediately at any quality level.
//
#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include <atomic>

namespace ExplorerLens { namespace Engine {

/// Fidelity level for a thumbnail emission stage.
enum class EmitterFidelity : uint8_t {
    PLACEHOLDER = 0,  ///< 64×64 blurred/solid colour placeholder
    LOW_RES     = 1,  ///< 256×256 fast single-pass decode
    FULL_RES    = 2,  ///< 512×512 high-quality decode with all post-processing
    CANCELLED   = 3,  ///< Pipeline was cancelled before this stage
};

/// Result of one emit stage.
struct StageResult {
    EmitterFidelity fidelity   = EmitterFidelity::PLACEHOLDER;
    uint32_t        width      = 0;
    uint32_t        height     = 0;
    uint32_t        durationMs = 0;
    bool            success    = false;
};

/// Callback fired on each stage completion.
using StageCallback = std::function<void(const StageResult&)>;

/// Multi-stage progressive emitter.
/// Runs synchronously in unit tests; integrates with DecodePriorityQueue in production.
class MultiStageThumbnailEmitter {
public:
    explicit MultiStageThumbnailEmitter(StageCallback cb = nullptr);

    /// Emit all three stages for a file path.  Returns false if cancelled before completion.
    bool Emit(const std::wstring& filePath);

    /// Cancel a pending emission (cooperative; checked between stages).
    void Cancel();

    /// Returns true if the emitter was cancelled during the last Emit() call.
    bool WasCancelled() const;

    /// Number of stages completed in the last Emit() call (0–3).
    uint32_t StagesCompleted() const;

    /// Summary string for diagnostics.
    std::wstring Summary() const;

    struct Config {
        bool   emitPlaceholder = true;
        bool   emitLowRes      = true;
        bool   emitFullRes     = true;
        uint32_t placeholderMs = 0;   ///< simulated latency for tests
        uint32_t lowResMs      = 0;
        uint32_t fullResMs     = 0;
    };
    void SetConfig(const Config& cfg);

private:
    StageCallback           m_callback;
    Config                  m_config;
    std::atomic<bool>       m_cancel{false};
    uint32_t                m_stagesCompleted{0};
    bool                    m_wasCancelled{false};

    StageResult EmitStage(EmitterFidelity fidelity, uint32_t w, uint32_t h, uint32_t simulatedMs);
};

}} // namespace ExplorerLens::Engine
