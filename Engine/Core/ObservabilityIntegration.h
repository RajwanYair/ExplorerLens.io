#pragma once
//==============================================================================
// ObservabilityIntegration.h
// Connects ETW tracing + Structured Logger to the decode pipeline.
// Provides unified request tracing from ScopedTimer profiling hooks.
//
// Usage:
//   #include "Core/ObservabilityIntegration.h"
//   ObservabilityIntegration::Get().OnRequestStart(filePath, requestedSize);
//   ...
//   ObservabilityIntegration::Get().OnRequestComplete(filePath, result, elapsedMs);
//==============================================================================

#include <string>
#include <chrono>
#include <atomic>
#include <mutex>
#include <functional>
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#endif

namespace ExplorerLens {

/// Event severity levels matching ETW and JSON-lines logger
enum class ObservabilityLevel : uint8_t {
    Verbose  = 0,  // Full file paths, timing details
    Info     = 1,  // Normal request lifecycle events
    Warning  = 2,  // Decoder fallback, timeout, soft failures
    Error    = 3,  // Decode failure, crash caught by SEH
    Critical = 4   // Explorer-impacting failure
};

/// Privacy mode for file path handling
enum class PathPrivacy : uint8_t {
    Hashed   = 0,  // SHA-256(path) — default for ETW
    Full     = 1,  // Full path — only in Verbose mode or file logger
    Redacted = 2   // "[REDACTED]" — enterprise privacy mode
};

/// Structured event for pipeline observability
struct PipelineEvent {
    uint64_t    requestId   = 0;
    const wchar_t* filePath = nullptr;
    std::wstring extension;
    std::wstring decoderName;
    double      elapsedMs   = 0.0;
    bool        cacheHit    = false;
    bool        gpuUsed     = false;
    HRESULT     status      = E_PENDING;
    uint32_t    outputWidth = 0;
    uint32_t    outputHeight = 0;
    size_t      outputBytes = 0;
};

/// Sink interface — ETW, JSON-lines, or custom sinks can implement
class IObservabilitySink {
public:
    virtual ~IObservabilitySink() = default;
    virtual void EmitEvent(ObservabilityLevel level, const PipelineEvent& event, const wchar_t* eventName) = 0;
    virtual void Flush() = 0;
    virtual bool IsEnabled(ObservabilityLevel level) const = 0;
};

/// Singleton integration point connecting pipeline → ETW + file logger
class ObservabilityIntegration {
public:
    static ObservabilityIntegration& Get() {
        static ObservabilityIntegration instance;
        return instance;
    }

    // ─── Configuration ───────────────────────────────────────────
    void SetPrivacyMode(PathPrivacy mode) { m_privacy = mode; }
    PathPrivacy GetPrivacyMode() const { return m_privacy; }

    void SetMinLevel(ObservabilityLevel level) { m_minLevel = level; }
    ObservabilityLevel GetMinLevel() const { return m_minLevel; }

    void SetEnabled(bool enabled) { m_enabled = enabled; }
    bool IsEnabled() const { return m_enabled; }

    // ─── Sink Registration ───────────────────────────────────────
    void RegisterSink(IObservabilitySink* sink) {
        std::lock_guard<std::mutex> lock(m_sinkMutex);
        m_sinks.push_back(sink);
    }

    void UnregisterSink(IObservabilitySink* sink) {
        std::lock_guard<std::mutex> lock(m_sinkMutex);
        m_sinks.erase(
            std::remove(m_sinks.begin(), m_sinks.end(), sink),
            m_sinks.end());
    }

    // ─── Pipeline Events ─────────────────────────────────────────
    uint64_t OnRequestStart(const wchar_t* filePath, uint32_t requestedSize) {
        if (!m_enabled) return 0;
        uint64_t id = m_nextRequestId.fetch_add(1);
        PipelineEvent evt;
        evt.requestId = id;
        evt.filePath = filePath;
        evt.outputWidth = requestedSize;
        evt.outputHeight = requestedSize;
        Emit(ObservabilityLevel::Info, evt, L"RequestStart");
        return id;
    }

    void OnCacheHit(uint64_t requestId, const wchar_t* filePath) {
        if (!m_enabled) return;
        PipelineEvent evt;
        evt.requestId = requestId;
        evt.filePath = filePath;
        evt.cacheHit = true;
        Emit(ObservabilityLevel::Info, evt, L"CacheHit");
    }

    void OnCacheMiss(uint64_t requestId, const wchar_t* filePath) {
        if (!m_enabled) return;
        PipelineEvent evt;
        evt.requestId = requestId;
        evt.filePath = filePath;
        evt.cacheHit = false;
        Emit(ObservabilityLevel::Verbose, evt, L"CacheMiss");
    }

    void OnDecoderSelected(uint64_t requestId, const wchar_t* decoderName) {
        if (!m_enabled) return;
        PipelineEvent evt;
        evt.requestId = requestId;
        evt.decoderName = decoderName ? decoderName : L"unknown";
        Emit(ObservabilityLevel::Verbose, evt, L"DecoderSelected");
    }

    void OnDecodeFailure(uint64_t requestId, const wchar_t* filePath, HRESULT hr) {
        if (!m_enabled) return;
        PipelineEvent evt;
        evt.requestId = requestId;
        evt.filePath = filePath;
        evt.status = hr;
        Emit(ObservabilityLevel::Error, evt, L"DecodeFail");
    }

    void OnCrashCaught(uint64_t requestId, const wchar_t* filePath, uint32_t exceptionCode) {
        if (!m_enabled) return;
        PipelineEvent evt;
        evt.requestId = requestId;
        evt.filePath = filePath;
        evt.status = static_cast<HRESULT>(exceptionCode);
        Emit(ObservabilityLevel::Critical, evt, L"CrashCaught");
    }

    void OnRequestComplete(uint64_t requestId, const wchar_t* filePath,
                           HRESULT status, double elapsedMs,
                           uint32_t width, uint32_t height, size_t bytes) {
        if (!m_enabled) return;
        PipelineEvent evt;
        evt.requestId = requestId;
        evt.filePath = filePath;
        evt.status = status;
        evt.elapsedMs = elapsedMs;
        evt.outputWidth = width;
        evt.outputHeight = height;
        evt.outputBytes = bytes;
        Emit(ObservabilityLevel::Info, evt, L"RequestComplete");
    }

    // ─── Diagnostics ─────────────────────────────────────────────
    uint64_t GetTotalRequests() const { return m_totalRequests.load(); }
    uint64_t GetTotalFailures() const { return m_totalFailures.load(); }
    uint64_t GetTotalCacheHits() const { return m_totalCacheHits.load(); }

    void ResetCounters() {
        m_totalRequests = 0;
        m_totalFailures = 0;
        m_totalCacheHits = 0;
    }

private:
    ObservabilityIntegration() = default;
    ObservabilityIntegration(const ObservabilityIntegration&) = delete;
    ObservabilityIntegration& operator=(const ObservabilityIntegration&) = delete;

    void Emit(ObservabilityLevel level, const PipelineEvent& evt, const wchar_t* eventName) {
        if (level < m_minLevel) return;

        // Update counters
        if (wcscmp(eventName, L"RequestStart") == 0)    m_totalRequests++;
        if (wcscmp(eventName, L"DecodeFail") == 0)      m_totalFailures++;
        if (wcscmp(eventName, L"CrashCaught") == 0)     m_totalFailures++;
        if (wcscmp(eventName, L"CacheHit") == 0)        m_totalCacheHits++;

        // Dispatch to all registered sinks
        std::lock_guard<std::mutex> lock(m_sinkMutex);
        for (auto* sink : m_sinks) {
            if (sink && sink->IsEnabled(level)) {
                sink->EmitEvent(level, evt, eventName);
            }
        }
    }

    std::atomic<bool>         m_enabled{true};
    std::atomic<uint64_t>     m_nextRequestId{1};
    ObservabilityLevel        m_minLevel{ObservabilityLevel::Info};
    PathPrivacy               m_privacy{PathPrivacy::Hashed};
    std::mutex                m_sinkMutex;
    std::vector<IObservabilitySink*> m_sinks;

    // Counters
    std::atomic<uint64_t>     m_totalRequests{0};
    std::atomic<uint64_t>     m_totalFailures{0};
    std::atomic<uint64_t>     m_totalCacheHits{0};
};

} // namespace ExplorerLens

