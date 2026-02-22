//==============================================================================
// DarkThumbs Engine — Sprint 313: Plugin Performance Profiler
// Per-plugin CPU/memory/latency profiling with flame graph data,
// decode budget enforcement, and slow-plugin alerting.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

enum class PluginPerfMetric : uint8_t {
    DecodeLatencyMs = 0, MemoryPeakKB, CPUPercent, IOBytes, GDIObjects, HandleCount, COUNT
};

enum class PluginPerfAlert : uint8_t {
    None = 0, SlowDecode, HighMemory, HighCPU, LeakedHandles, COUNT
};

enum class PluginPerfSamplingRate : uint8_t {
    Off = 0, Low_1Hz, Medium_10Hz, High_100Hz, COUNT
};

struct PluginPerfSample {
    std::wstring    pluginId;
    PluginPerfMetric metric  = PluginPerfMetric::DecodeLatencyMs;
    float           value    = 0.0f;
    uint64_t        timestampMs = 0;
};

struct PluginPerfSummary {
    std::wstring pluginId;
    float   avgDecodeMs     = 0.0f;
    float   p95DecodeMs     = 0.0f;
    float   peakMemoryMB    = 0.0f;
    float   avgCpuPct       = 0.0f;
    uint32_t sampleCount    = 0;
    PluginPerfAlert worstAlert = PluginPerfAlert::None;
};

class PluginPerformanceProfiler {
public:
    static const wchar_t* MetricName(PluginPerfMetric m) {
        switch (m) {
            case PluginPerfMetric::DecodeLatencyMs: return L"Decode Latency (ms)";
            case PluginPerfMetric::MemoryPeakKB:   return L"Peak Memory (KB)";
            case PluginPerfMetric::CPUPercent:      return L"CPU (%)";
            case PluginPerfMetric::IOBytes:         return L"I/O Bytes";
            case PluginPerfMetric::GDIObjects:      return L"GDI Objects";
            case PluginPerfMetric::HandleCount:     return L"Handle Count";
            default: return L"Unknown";
        }
    }
    static const wchar_t* AlertName(PluginPerfAlert a) {
        switch (a) {
            case PluginPerfAlert::None:          return L"None";
            case PluginPerfAlert::SlowDecode:    return L"Slow Decode";
            case PluginPerfAlert::HighMemory:    return L"High Memory";
            case PluginPerfAlert::HighCPU:       return L"High CPU";
            case PluginPerfAlert::LeakedHandles: return L"Leaked Handles";
            default: return L"Unknown";
        }
    }
    static const wchar_t* SamplingRateName(PluginPerfSamplingRate r) {
        switch (r) {
            case PluginPerfSamplingRate::Off:       return L"Off";
            case PluginPerfSamplingRate::Low_1Hz:   return L"Low (1 Hz)";
            case PluginPerfSamplingRate::Medium_10Hz:return L"Medium (10 Hz)";
            case PluginPerfSamplingRate::High_100Hz:return L"High (100 Hz)";
            default: return L"Unknown";
        }
    }
    static constexpr size_t MetricCount()       { return static_cast<size_t>(PluginPerfMetric::COUNT); }
    static constexpr size_t AlertCount()        { return static_cast<size_t>(PluginPerfAlert::COUNT); }
    static constexpr size_t SamplingRateCount() { return static_cast<size_t>(PluginPerfSamplingRate::COUNT); }
    static bool IsSlowPlugin(const PluginPerfSummary& s) { return s.p95DecodeMs > 50.0f; }
};

}} // namespace DarkThumbs::Engine
