#include "PerformanceProfiler.h"
#include <sstream>
#include <iomanip>
#include <fstream>
#include <algorithm>

namespace DarkThumbs {

    // ScopedTimer Implementation
    ScopedTimer::ScopedTimer(ProfileComponent component)
        : m_component(component)
        , m_startTime(std::chrono::high_resolution_clock::now())
    {
    }

    ScopedTimer::~ScopedTimer()
    {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - m_startTime);
        double timeMs = duration.count() / 1000.0;

        PerformanceProfiler::GetInstance().RecordSample(m_component, timeMs);
    }

    // PerformanceProfiler Implementation
    PerformanceProfiler::PerformanceProfiler()
    {
        InitializeStats();
    }

    PerformanceProfiler& PerformanceProfiler::GetInstance()
    {
        static PerformanceProfiler instance;
        return instance;
    }

    void PerformanceProfiler::InitializeStats()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_stats[ProfileComponent::CACHE_LOOKUP].name = L"Cache Lookup";
        m_stats[ProfileComponent::CACHE_STORE].name = L"Cache Store";
        m_stats[ProfileComponent::DECODE_IMAGE].name = L"Decode Image";
        m_stats[ProfileComponent::DECODE_WEBP].name = L"Decode WebP";
        m_stats[ProfileComponent::DECODE_AVIF].name = L"Decode AVIF";
        m_stats[ProfileComponent::DECODE_ARCHIVE].name = L"Decode Archive";
        m_stats[ProfileComponent::DECODE_JXL].name = L"Decode JXL";
        m_stats[ProfileComponent::DECODE_HEIF].name = L"Decode HEIF";
        m_stats[ProfileComponent::DECODE_RAW].name = L"Decode RAW";
        m_stats[ProfileComponent::DECODE_ICO].name = L"Decode ICO";
        m_stats[ProfileComponent::DECODE_TGA].name = L"Decode TGA";
        m_stats[ProfileComponent::DECODE_QOI].name = L"Decode QOI";
        m_stats[ProfileComponent::DECODE_PSD].name = L"Decode PSD";
        m_stats[ProfileComponent::DECODE_DDS].name = L"Decode DDS";
        m_stats[ProfileComponent::DECODE_HDR].name = L"Decode HDR";
        m_stats[ProfileComponent::DECODE_PPM].name = L"Decode PPM";
        m_stats[ProfileComponent::DECODE_EXR].name = L"Decode EXR";
        m_stats[ProfileComponent::DECODE_SVG].name = L"Decode SVG";
        m_stats[ProfileComponent::DECODE_VIDEO].name = L"Decode Video";
        m_stats[ProfileComponent::DECODE_AUDIO].name = L"Decode Audio";
        m_stats[ProfileComponent::DECODE_PDF].name = L"Decode PDF";
        m_stats[ProfileComponent::DECODE_DOCUMENT].name = L"Decode Document";
        m_stats[ProfileComponent::DECODE_FONT].name = L"Decode Font";
        m_stats[ProfileComponent::GPU_RENDER_D3D11].name = L"GPU Render (D3D11)";
        m_stats[ProfileComponent::GPU_RENDER_GDI].name = L"GPU Render (GDI+)";
        m_stats[ProfileComponent::SIMD_SCALE_BILINEAR].name = L"SIMD Scale Bilinear";
        m_stats[ProfileComponent::SIMD_SCALE_BICUBIC].name = L"SIMD Scale Bicubic";
        m_stats[ProfileComponent::SIMD_COLOR_CONVERT].name = L"SIMD Color Convert";
        m_stats[ProfileComponent::PIPELINE_TOTAL].name = L"Pipeline Total";
    }

    void PerformanceProfiler::RecordSample(ProfileComponent component, double timeMs)
    {
        if (!m_enabled) return;

        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats[component].AddSample(timeMs);
    }

    const ComponentStats& PerformanceProfiler::GetStats(ProfileComponent component) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats.at(component);
    }

    std::map<ProfileComponent, ComponentStats> PerformanceProfiler::GetAllStats() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    void PerformanceProfiler::Reset()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& [component, stats] : m_stats) {
            stats.Reset();
        }
    }

    std::wstring PerformanceProfiler::ComponentToString(ProfileComponent component) const
    {
        auto it = m_stats.find(component);
        if (it != m_stats.end()) {
            return it->second.name;
        }
        return L"Unknown";
    }

    std::wstring PerformanceProfiler::GenerateReport() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::wostringstream report;

        report << L"=== Performance Profile Summary ===" << std::endl << std::endl;

        // Sort by total time (descending)
        std::vector<std::pair<ProfileComponent, ComponentStats>> sortedStats;
        for (const auto& [component, stats] : m_stats) {
            if (stats.callCount > 0) {
                sortedStats.push_back({ component, stats });
            }
        }

        std::sort(sortedStats.begin(), sortedStats.end(),
            [](const auto& a, const auto& b) {
                return a.second.totalTimeMs > b.second.totalTimeMs;
            });

        // Header
        report << std::left << std::setw(25) << L"Component"
               << std::right << std::setw(10) << L"Calls"
               << std::setw(12) << L"Total(ms)"
               << std::setw(12) << L"Avg(ms)"
               << std::setw(12) << L"Min(ms)"
               << std::setw(12) << L"Max(ms)" << std::endl;
        report << std::wstring(83, L'-') << std::endl;

        // Data rows
        for (const auto& [component, stats] : sortedStats) {
            report << std::left << std::setw(25) << stats.name
                   << std::right << std::setw(10) << stats.callCount
                   << std::setw(12) << std::fixed << std::setprecision(2) << stats.totalTimeMs
                   << std::setw(12) << std::fixed << std::setprecision(2) << stats.avgTimeMs
                   << std::setw(12) << std::fixed << std::setprecision(2) << stats.minTimeMs
                   << std::setw(12) << std::fixed << std::setprecision(2) << stats.maxTimeMs
                   << std::endl;
        }

        return report.str();
    }

    std::wstring PerformanceProfiler::GenerateDetailedReport() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::wostringstream report;

        report << L"=== Detailed Performance Analysis ===" << std::endl << std::endl;

        // Calculate total time and call count
        double totalTimeMs = 0.0;
        size_t totalCalls = 0;

        for (const auto& [component, stats] : m_stats) {
            if (stats.callCount > 0) {
                totalTimeMs += stats.totalTimeMs;
                totalCalls += stats.callCount;
            }
        }

        report << L"Total Operations: " << totalCalls << std::endl;
        report << L"Total Time: " << std::fixed << std::setprecision(2) << totalTimeMs << L" ms" << std::endl;
        report << L"Average per Operation: " << std::fixed << std::setprecision(2) 
               << (totalCalls > 0 ? totalTimeMs / totalCalls : 0.0) << L" ms" << std::endl;
        report << std::endl;

        // Component breakdown with percentages
        report << L"Component Breakdown:" << std::endl;
        report << std::wstring(83, L'-') << std::endl;

        for (const auto& [component, stats] : m_stats) {
            if (stats.callCount > 0) {
                double percentage = totalTimeMs > 0.0 ? (stats.totalTimeMs / totalTimeMs * 100.0) : 0.0;
                
                report << stats.name << L":" << std::endl;
                report << L"  Calls: " << stats.callCount << std::endl;
                report << L"  Total Time: " << std::fixed << std::setprecision(2) << stats.totalTimeMs 
                       << L" ms (" << std::fixed << std::setprecision(1) << percentage << L"%)" << std::endl;
                report << L"  Average: " << std::fixed << std::setprecision(2) << stats.avgTimeMs << L" ms" << std::endl;
                report << L"  Min: " << std::fixed << std::setprecision(2) << stats.minTimeMs << L" ms" << std::endl;
                report << L"  Max: " << std::fixed << std::setprecision(2) << stats.maxTimeMs << L" ms" << std::endl;
                report << L"  Range: " << std::fixed << std::setprecision(2) 
                       << (stats.maxTimeMs - stats.minTimeMs) << L" ms" << std::endl;
                report << std::endl;
            }
        }

        return report.str();
    }

    bool PerformanceProfiler::ExportToFile(const std::wstring& filePath) const
    {
        try {
            std::wofstream file(filePath);
            if (!file.is_open()) {
                return false;
            }

            file << GenerateDetailedReport();
            file.close();
            return true;
        }
        catch (...) {
            return false;
        }
    }

} // namespace DarkThumbs
