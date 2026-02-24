#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <mutex>
#include <algorithm>

namespace ExplorerLens {

    // Enum for profiling different components
    enum class ProfileComponent {
        CACHE_LOOKUP,
        CACHE_STORE,
        DECODE_IMAGE,
        DECODE_WEBP,
        DECODE_AVIF,
        DECODE_ARCHIVE,
        DECODE_JXL,
        DECODE_HEIF,
        DECODE_RAW,
        DECODE_ICO,
        DECODE_TGA,
        DECODE_QOI,
        DECODE_PSD,
        DECODE_DDS,
        DECODE_HDR,
        DECODE_PPM,
        DECODE_EXR,
        DECODE_SVG,
        DECODE_VIDEO,
        DECODE_AUDIO,
        DECODE_PDF,
        DECODE_DOCUMENT,
        DECODE_FONT,
        GPU_RENDER_D3D11,
        GPU_RENDER_GDI,
        SIMD_SCALE_BILINEAR,
        SIMD_SCALE_BICUBIC,
        SIMD_COLOR_CONVERT,
        PIPELINE_DECODER_INIT,
        PIPELINE_DECODER_LOOKUP,
        PIPELINE_TOTAL,
        COMPONENT_COUNT
    };

    // Structure to hold timing statistics for a component
    struct ComponentStats {
        std::wstring name;
        size_t callCount = 0;
        double totalTimeMs = 0.0;
        double minTimeMs = DBL_MAX;
        double maxTimeMs = 0.0;
        double avgTimeMs = 0.0;

        void AddSample(double timeMs) {
            callCount++;
            totalTimeMs += timeMs;
            minTimeMs = (std::min)(minTimeMs, timeMs);
            maxTimeMs = (std::max)(maxTimeMs, timeMs);
            avgTimeMs = totalTimeMs / callCount;
        }

        void Reset() {
            callCount = 0;
            totalTimeMs = 0.0;
            minTimeMs = DBL_MAX;
            maxTimeMs = 0.0;
            avgTimeMs = 0.0;
        }
    };

    // RAII timer for automatic timing scope
    class ScopedTimer {
    public:
        ScopedTimer(ProfileComponent component);
        ~ScopedTimer();

        // Disable copy
        ScopedTimer(const ScopedTimer&) = delete;
        ScopedTimer& operator=(const ScopedTimer&) = delete;

    private:
        ProfileComponent m_component;
        std::chrono::high_resolution_clock::time_point m_startTime;
    };

    // Singleton performance profiler
    class PerformanceProfiler {
    public:
        static PerformanceProfiler& GetInstance();

        // Enable/disable profiling
        void SetEnabled(bool enabled) { m_enabled = enabled; }
        bool IsEnabled() const { return m_enabled; }

        // Record timing sample
        void RecordSample(ProfileComponent component, double timeMs);

        // Record memory usage
        void RecordMemoryUsage(ProfileComponent component, size_t bytes);

        // Get statistics
        const ComponentStats& GetStats(ProfileComponent component) const;
        std::map<ProfileComponent, ComponentStats> GetAllStats() const;

        // Get total memory usage
        size_t GetTotalMemoryUsage() const;
        size_t GetMemoryUsage(ProfileComponent component) const;

        // Performance analysis
        std::vector<ProfileComponent> GetSlowestComponents(size_t count = 5) const;
        std::vector<ProfileComponent> GetMostCalledComponents(size_t count = 5) const;
        double GetTotalTimeMs() const;

        // Reset all statistics
        void Reset();

        // Generate reports
        std::wstring GenerateReport() const;
        std::wstring GenerateDetailedReport() const;
        std::wstring GenerateHTMLReport() const;
        std::wstring GenerateCSVReport() const;

        // Export to file
        bool ExportToFile(const std::wstring& filePath) const;
        bool ExportHTMLReport(const std::wstring& filePath) const;
        bool ExportCSVReport(const std::wstring& filePath) const;

        // Performance hints
        std::vector<std::wstring> GetPerformanceHints() const;

    private:
        PerformanceProfiler();
        ~PerformanceProfiler() = default;

        // Disable copy
        PerformanceProfiler(const PerformanceProfiler&) = delete;
        PerformanceProfiler& operator=(const PerformanceProfiler&) = delete;

        bool m_enabled = false;
        mutable std::mutex m_mutex;
        std::map<ProfileComponent, ComponentStats> m_stats;
        std::map<ProfileComponent, size_t> m_memoryUsage;

        void InitializeStats();
        std::wstring ComponentToString(ProfileComponent component) const;
    };

    // Helper macros for easy profiling
    #define PROFILE_SCOPE(component) \
        ExplorerLens::ScopedTimer _scopedTimer##__LINE__(component)

    #define PROFILE_FUNCTION(component) \
        PROFILE_SCOPE(component)

} // namespace ExplorerLens

