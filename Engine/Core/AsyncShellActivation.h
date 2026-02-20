//==============================================================================
// DarkThumbs Engine — Sprint 263: Async Shell Extension
// Activate AsyncThumbnailProvider for non-blocking Explorer integration.
// Progress indication for slow decodes, timeout management, priority scheduling.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <chrono>

namespace DarkThumbs { namespace Engine {

/// Async decode request state
enum class AsyncDecodeState : uint8_t {
    Queued,         // Waiting in queue
    Dispatched,     // Sent to decode thread
    Decoding,       // Actively decoding
    GPUProcessing,  // GPU resize/effects in progress
    Completed,      // Successfully completed
    Failed,         // Decode failed
    TimedOut,       // Exceeded timeout
    Cancelled       // Cancelled by caller
};

/// Async decode priority
enum class DecodePriority : uint8_t {
    Critical = 0,   // Visible in viewport — immediate
    High     = 1,   // About to scroll into view
    Normal   = 2,   // Background prefetch
    Low      = 3,   // Speculative decode
    Idle     = 4    // When nothing else to do
};

/// Async thumbnail request
struct AsyncThumbnailRequest {
    uint64_t            requestId   = 0;
    std::wstring        filePath;
    uint32_t            requestedSize = 256;
    DecodePriority      priority    = DecodePriority::Normal;
    AsyncDecodeState    state       = AsyncDecodeState::Queued;
    std::chrono::milliseconds timeout{ 5000 };
    std::chrono::steady_clock::time_point queuedAt;
    std::chrono::steady_clock::time_point completedAt;
};

/// Async provider configuration
struct AsyncProviderConfig {
    uint32_t maxConcurrent      = 4;        // Max parallel decodes
    uint32_t queueCapacity      = 256;      // Max queued requests
    uint32_t defaultTimeoutMs   = 5000;     // Default timeout
    uint32_t criticalTimeoutMs  = 10000;    // Critical priority timeout
    bool     enablePriority     = true;
    bool     enableProgress     = true;
    bool     cancelOnScroll     = true;     // Cancel off-screen requests
};

/// Async thumbnail provider activation
class AsyncShellActivation {
public:
    /// State display name
    static const wchar_t* StateName(AsyncDecodeState s) {
        switch (s) {
            case AsyncDecodeState::Queued:        return L"Queued";
            case AsyncDecodeState::Dispatched:    return L"Dispatched";
            case AsyncDecodeState::Decoding:      return L"Decoding";
            case AsyncDecodeState::GPUProcessing: return L"GPU Processing";
            case AsyncDecodeState::Completed:     return L"Completed";
            case AsyncDecodeState::Failed:        return L"Failed";
            case AsyncDecodeState::TimedOut:      return L"Timed Out";
            case AsyncDecodeState::Cancelled:     return L"Cancelled";
            default: return L"Unknown";
        }
    }

    /// Priority display name
    static const wchar_t* PriorityName(DecodePriority p) {
        switch (p) {
            case DecodePriority::Critical: return L"Critical";
            case DecodePriority::High:     return L"High";
            case DecodePriority::Normal:   return L"Normal";
            case DecodePriority::Low:      return L"Low";
            case DecodePriority::Idle:     return L"Idle";
            default: return L"Unknown";
        }
    }

    /// State count
    static constexpr size_t StateCount() { return 8; }

    /// Priority count
    static constexpr size_t PriorityCount() { return 5; }

    /// Validate config
    static bool ValidateConfig(const AsyncProviderConfig& cfg) {
        if (cfg.maxConcurrent == 0 || cfg.maxConcurrent > 16) return false;
        if (cfg.queueCapacity == 0 || cfg.queueCapacity > 4096) return false;
        if (cfg.defaultTimeoutMs < 1000 || cfg.defaultTimeoutMs > 30000) return false;
        return true;
    }

    /// Calculate effective timeout based on priority
    static uint32_t EffectiveTimeout(const AsyncProviderConfig& cfg, DecodePriority priority) {
        switch (priority) {
            case DecodePriority::Critical: return cfg.criticalTimeoutMs;
            case DecodePriority::High:     return cfg.defaultTimeoutMs;
            case DecodePriority::Normal:   return cfg.defaultTimeoutMs;
            case DecodePriority::Low:      return cfg.defaultTimeoutMs / 2;
            case DecodePriority::Idle:     return cfg.defaultTimeoutMs / 4;
            default: return cfg.defaultTimeoutMs;
        }
    }
};

}} // namespace DarkThumbs::Engine
