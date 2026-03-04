#pragma once
// ETWTraceProvider.h — Real Windows ETW TraceLogging Provider
// Copyright (c) 2026 ExplorerLens Project
//
// Provides actual Windows ETW integration using TraceLogging API.
// This bridges the application-level ETWSinkComplete.h to real ETW sessions,
// enabling profiling with WPA/PerfView/xperf/tracelog.
//
// Provider GUID: auto-generated from name "ExplorerLens-Engine-Provider"
// Usage:
//   1. ETWTraceProvider::Instance().Initialize()
//   2. Use LENS_ETW_EVENT / LENS_ETW_TIMING macros
//   3. Capture: tracelog -start ExplorerLens -guid *ExplorerLens-Engine-Provider
//
// Safe under WIN32_LEAN_AND_MEAN — TraceLogging is self-contained.

#ifdef _WIN32

#include <windows.h>
#include <TraceLoggingProvider.h>
#include <evntrace.h>
#include <chrono>
#include <string>
#include <atomic>

#pragma comment(lib, "advapi32.lib")

// ============================================================================
// TraceLogging Provider Declaration
// ============================================================================

// Forward-declare the provider handle
TRACELOGGING_DECLARE_PROVIDER(g_hExplorerLensProvider);

namespace ExplorerLens {
namespace ETW {

// ============================================================================
// ETW Event Levels (matches standard ETW levels)
// ============================================================================
enum class EventLevel : uint8_t {
    Critical = 1,
    Error = 2,
    Warning = 3,
    Info = 4,
    Verbose = 5
};

// ============================================================================
// ETW Keywords (bitmask for event filtering)
// ============================================================================
namespace Keywords {
constexpr uint64_t Pipeline = 0x0001;
constexpr uint64_t Cache = 0x0002;
constexpr uint64_t Decoder = 0x0004;
constexpr uint64_t GPU = 0x0008;
constexpr uint64_t Plugin = 0x0010;
constexpr uint64_t Memory = 0x0020;
constexpr uint64_t Config = 0x0040;
constexpr uint64_t Health = 0x0080;
constexpr uint64_t IO = 0x0100;
constexpr uint64_t Startup = 0x0200;
constexpr uint64_t UI = 0x0400;
constexpr uint64_t All = 0xFFFF;
}

// ============================================================================
// ETWTraceProvider — Singleton managing the TraceLogging provider lifecycle
// ============================================================================
class ETWTraceProvider {
public:
    static ETWTraceProvider& Instance() {
        static ETWTraceProvider s_instance;
        return s_instance;
    }

    // Register the ETW provider (call once at startup)
    bool Initialize() {
        if (m_registered.load(std::memory_order_acquire))
            return true;

        HRESULT hr = TraceLoggingRegister(g_hExplorerLensProvider);
        if (SUCCEEDED(hr)) {
            m_registered.store(true, std::memory_order_release);
            return true;
        }
        return false;
    }

    // Unregister (call at shutdown)
    void Shutdown() {
        if (m_registered.exchange(false, std::memory_order_acq_rel)) {
            TraceLoggingUnregister(g_hExplorerLensProvider);
        }
    }

    bool IsRegistered() const {
        return m_registered.load(std::memory_order_acquire);
    }

    // Check if any consumer is listening at a given level/keyword
    bool IsEnabled(EventLevel level = EventLevel::Verbose,
        uint64_t keyword = Keywords::All) const {
        if (!m_registered.load(std::memory_order_acquire))
            return false;
        return TraceLoggingProviderEnabled(g_hExplorerLensProvider,
            static_cast<UCHAR>(level), keyword) != FALSE;
    }

    // ====================================================================
    // High-level event emitters
    // ====================================================================

    // Log a decode request start
    void LogDecodeStart(const wchar_t* filePath, const char* decoder,
        uint32_t requestedSize) {
        if (!IsEnabled(EventLevel::Info, Keywords::Pipeline))
            return;

        TraceLoggingWrite(g_hExplorerLensProvider,
            "DecodeStart",
            TraceLoggingLevel(TRACE_LEVEL_INFORMATION),
            TraceLoggingKeyword(Keywords::Pipeline),
            TraceLoggingWideString(filePath, "FilePath"),
            TraceLoggingString(decoder, "Decoder"),
            TraceLoggingUInt32(requestedSize, "RequestedSize"));
        m_eventsEmitted.fetch_add(1, std::memory_order_relaxed);
    }

    // Log a decode completion with timing
    void LogDecodeComplete(const wchar_t* filePath, const char* decoder,
        double durationMs, bool success, uint32_t outputWidth,
        uint32_t outputHeight) {
        if (!IsEnabled(EventLevel::Info, Keywords::Pipeline))
            return;

        TraceLoggingWrite(g_hExplorerLensProvider,
            "DecodeComplete",
            TraceLoggingLevel(TRACE_LEVEL_INFORMATION),
            TraceLoggingKeyword(Keywords::Pipeline | Keywords::Decoder),
            TraceLoggingWideString(filePath, "FilePath"),
            TraceLoggingString(decoder, "Decoder"),
            TraceLoggingFloat64(durationMs, "DurationMs"),
            TraceLoggingBool(success, "Success"),
            TraceLoggingUInt32(outputWidth, "OutputWidth"),
            TraceLoggingUInt32(outputHeight, "OutputHeight"));
        m_eventsEmitted.fetch_add(1, std::memory_order_relaxed);
    }

    // Log a cache hit/miss
    void LogCacheAccess(const char* cacheType, bool hit, double lookupMs) {
        if (!IsEnabled(EventLevel::Verbose, Keywords::Cache))
            return;

        TraceLoggingWrite(g_hExplorerLensProvider,
            "CacheAccess",
            TraceLoggingLevel(TRACE_LEVEL_VERBOSE),
            TraceLoggingKeyword(Keywords::Cache),
            TraceLoggingString(cacheType, "CacheType"),
            TraceLoggingBool(hit, "Hit"),
            TraceLoggingFloat64(lookupMs, "LookupMs"));
        m_eventsEmitted.fetch_add(1, std::memory_order_relaxed);
    }

    // Log GPU operation
    void LogGPUOperation(const char* operation, double durationMs,
        uint64_t bytesProcessed) {
        if (!IsEnabled(EventLevel::Info, Keywords::GPU))
            return;

        TraceLoggingWrite(g_hExplorerLensProvider,
            "GPUOperation",
            TraceLoggingLevel(TRACE_LEVEL_INFORMATION),
            TraceLoggingKeyword(Keywords::GPU),
            TraceLoggingString(operation, "Operation"),
            TraceLoggingFloat64(durationMs, "DurationMs"),
            TraceLoggingUInt64(bytesProcessed, "BytesProcessed"));
        m_eventsEmitted.fetch_add(1, std::memory_order_relaxed);
    }

    // Log memory allocation/pressure event
    void LogMemoryEvent(const char* event, uint64_t currentBytes,
        uint64_t peakBytes, uint8_t pressureLevel) {
        if (!IsEnabled(EventLevel::Warning, Keywords::Memory))
            return;

        TraceLoggingWrite(g_hExplorerLensProvider,
            "MemoryEvent",
            TraceLoggingLevel(TRACE_LEVEL_WARNING),
            TraceLoggingKeyword(Keywords::Memory),
            TraceLoggingString(event, "Event"),
            TraceLoggingUInt64(currentBytes, "CurrentBytes"),
            TraceLoggingUInt64(peakBytes, "PeakBytes"),
            TraceLoggingUInt8(pressureLevel, "PressureLevel"));
        m_eventsEmitted.fetch_add(1, std::memory_order_relaxed);
    }

    // Log a plugin lifecycle event
    void LogPluginEvent(const char* pluginName, const char* event,
        bool success) {
        if (!IsEnabled(EventLevel::Info, Keywords::Plugin))
            return;

        TraceLoggingWrite(g_hExplorerLensProvider,
            "PluginEvent",
            TraceLoggingLevel(TRACE_LEVEL_INFORMATION),
            TraceLoggingKeyword(Keywords::Plugin),
            TraceLoggingString(pluginName, "PluginName"),
            TraceLoggingString(event, "Event"),
            TraceLoggingBool(success, "Success"));
        m_eventsEmitted.fetch_add(1, std::memory_order_relaxed);
    }

    // Log an error/warning
    void LogError(const char* component, const char* message,
        HRESULT hr = S_OK) {
        if (!IsEnabled(EventLevel::Error))
            return;

        TraceLoggingWrite(g_hExplorerLensProvider,
            "Error",
            TraceLoggingLevel(TRACE_LEVEL_ERROR),
            TraceLoggingKeyword(Keywords::All),
            TraceLoggingString(component, "Component"),
            TraceLoggingString(message, "Message"),
            TraceLoggingHResult(hr, "HRESULT"));
        m_eventsEmitted.fetch_add(1, std::memory_order_relaxed);
    }

    // Log startup phase completion
    void LogStartupPhase(const char* phase, double durationMs) {
        if (!IsEnabled(EventLevel::Info, Keywords::Startup))
            return;

        TraceLoggingWrite(g_hExplorerLensProvider,
            "StartupPhase",
            TraceLoggingLevel(TRACE_LEVEL_INFORMATION),
            TraceLoggingKeyword(Keywords::Startup),
            TraceLoggingString(phase, "Phase"),
            TraceLoggingFloat64(durationMs, "DurationMs"));
        m_eventsEmitted.fetch_add(1, std::memory_order_relaxed);
    }

    // Log health check result
    void LogHealthCheck(const char* dimension, double score,
        const char* grade) {
        if (!IsEnabled(EventLevel::Info, Keywords::Health))
            return;

        TraceLoggingWrite(g_hExplorerLensProvider,
            "HealthCheck",
            TraceLoggingLevel(TRACE_LEVEL_INFORMATION),
            TraceLoggingKeyword(Keywords::Health),
            TraceLoggingString(dimension, "Dimension"),
            TraceLoggingFloat64(score, "Score"),
            TraceLoggingString(grade, "Grade"));
        m_eventsEmitted.fetch_add(1, std::memory_order_relaxed);
    }

    // Statistics
    uint64_t EventsEmitted() const {
        return m_eventsEmitted.load(std::memory_order_relaxed);
    }

private:
    ETWTraceProvider() = default;
    ~ETWTraceProvider() { Shutdown(); }

    ETWTraceProvider(const ETWTraceProvider&) = delete;
    ETWTraceProvider& operator=(const ETWTraceProvider&) = delete;

    std::atomic<bool> m_registered{ false };
    std::atomic<uint64_t> m_eventsEmitted{ 0 };
};

// ============================================================================
// RAII Scoped ETW Timer — measures and logs duration automatically
// ============================================================================
class ETWScopedTimer {
public:
    ETWScopedTimer(const char* eventName, uint64_t keyword = Keywords::Pipeline)
        : m_eventName(eventName)
        , m_keyword(keyword)
        , m_start(std::chrono::high_resolution_clock::now()) {
    }

    ~ETWScopedTimer() {
        auto& provider = ETWTraceProvider::Instance();
        if (!provider.IsRegistered())
            return;

        auto elapsed = std::chrono::high_resolution_clock::now() - m_start;
        double ms = std::chrono::duration<double, std::milli>(elapsed).count();

        // TraceLoggingKeyword requires compile-time constant, so we use
        // Keywords::All and pass the intended keyword as a data field
        TraceLoggingWrite(g_hExplorerLensProvider,
            "ScopedTimer",
            TraceLoggingLevel(TRACE_LEVEL_VERBOSE),
            TraceLoggingKeyword(Keywords::All),
            TraceLoggingString(m_eventName, "EventName"),
            TraceLoggingFloat64(ms, "DurationMs"),
            TraceLoggingUInt64(m_keyword, "Keyword"));
    }

private:
    const char* m_eventName;
    uint64_t m_keyword;
    std::chrono::high_resolution_clock::time_point m_start;
};

} // namespace ETW
} // namespace ExplorerLens

// ============================================================================
// Convenience Macros
// ============================================================================

// Log a scoped timing event to ETW
#define LENS_ETW_TIMER(name) \
    ExplorerLens::ETW::ETWScopedTimer _etwTimer_##__LINE__(name)

#define LENS_ETW_TIMER_KW(name, keyword) \
    ExplorerLens::ETW::ETWScopedTimer _etwTimer_##__LINE__(name, keyword)

#else // !_WIN32

// Non-Windows stubs — lightweight implementations that track event counts
#include <atomic>
#include <cstdint>

namespace ExplorerLens {
namespace ETW {

class ETWTraceProvider {
public:
    static ETWTraceProvider& Instance() {
        static ETWTraceProvider s_instance;
        return s_instance;
    }
    bool Initialize() {
        m_initialized.store(true, std::memory_order_release);
        return true;
    }
    void Shutdown() {
        m_initialized.store(false, std::memory_order_release);
    }
    void Flush() {}
    bool IsRegistered() const {
        return m_initialized.load(std::memory_order_acquire);
    }
    bool IsEnabled(int = 0, uint64_t = 0) const {
        return m_initialized.load(std::memory_order_acquire);
    }
    void LogDecodeStart(const wchar_t*, const char*, uint32_t) {
        m_eventsEmitted.fetch_add(1, std::memory_order_relaxed);
    }
    void LogDecodeComplete(const wchar_t*, const char*, double, bool, uint32_t, uint32_t) {
        m_eventsEmitted.fetch_add(1, std::memory_order_relaxed);
    }
    void LogCacheAccess(const char*, bool, double) {
        m_eventsEmitted.fetch_add(1, std::memory_order_relaxed);
    }
    void LogGPUOperation(const char*, double, uint64_t) {
        m_eventsEmitted.fetch_add(1, std::memory_order_relaxed);
    }
    void LogMemoryEvent(const char*, uint64_t, uint64_t, uint8_t) {
        m_eventsEmitted.fetch_add(1, std::memory_order_relaxed);
    }
    void LogPluginEvent(const char*, const char*, bool) {
        m_eventsEmitted.fetch_add(1, std::memory_order_relaxed);
    }
    void LogError(const char*, const char*, long = 0) {
        m_eventsEmitted.fetch_add(1, std::memory_order_relaxed);
    }
    void LogStartupPhase(const char*, double) {
        m_eventsEmitted.fetch_add(1, std::memory_order_relaxed);
    }
    void LogHealthCheck(const char*, double, const char*) {
        m_eventsEmitted.fetch_add(1, std::memory_order_relaxed);
    }
    uint64_t EventsEmitted() const {
        return m_eventsEmitted.load(std::memory_order_relaxed);
    }

private:
    ETWTraceProvider() = default;
    ~ETWTraceProvider() { Shutdown(); }
    ETWTraceProvider(const ETWTraceProvider&) = delete;
    ETWTraceProvider& operator=(const ETWTraceProvider&) = delete;

    std::atomic<bool> m_initialized{ false };
    std::atomic<uint64_t> m_eventsEmitted{ 0 };
};

class ETWScopedTimer {
public:
    ETWScopedTimer(const char*, uint64_t = 0) {}
};

} // namespace ETW
} // namespace ExplorerLens

#define LENS_ETW_TIMER(name)
#define LENS_ETW_TIMER_KW(name, keyword)
#define LENS_ETW_EVENT(name, level, keyword, ...)

#endif // _WIN32
