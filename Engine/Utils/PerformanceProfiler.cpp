#include "PerformanceProfiler.h"
#include <sstream>
#include <iomanip>
#include <fstream>
#include <algorithm>

namespace ExplorerLens {

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
        m_stats[ProfileComponent::PIPELINE_DECODER_INIT].name = L"Pipeline Decoder Init";
        m_stats[ProfileComponent::PIPELINE_DECODER_LOOKUP].name = L"Pipeline Decoder Lookup";
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

    // ============================================================================
    // Memory Tracking
    // ============================================================================

    void PerformanceProfiler::RecordMemoryUsage(ProfileComponent component, size_t bytes)
    {
        if (!m_enabled) return;

        std::lock_guard<std::mutex> lock(m_mutex);
        m_memoryUsage[component] += bytes;
    }

    size_t PerformanceProfiler::GetTotalMemoryUsage() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        size_t total = 0;
        for (const auto& [component, bytes] : m_memoryUsage) {
            total += bytes;
        }
        return total;
    }

    size_t PerformanceProfiler::GetMemoryUsage(ProfileComponent component) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_memoryUsage.find(component);
        return (it != m_memoryUsage.end()) ? it->second : 0;
    }

    // ============================================================================
    // Performance Analysis
    // ============================================================================

    std::vector<ProfileComponent> PerformanceProfiler::GetSlowestComponents(size_t count) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Collect all components with data
        std::vector<std::pair<ProfileComponent, double>> components;
        for (const auto& [component, stats] : m_stats) {
            if (stats.callCount > 0) {
                components.push_back({ component, stats.avgTimeMs });
            }
        }

        // Sort by average time (descending)
        std::sort(components.begin(), components.end(),
            [](const auto& a, const auto& b) {
                return a.second > b.second;
            });

        // Extract top N components
        std::vector<ProfileComponent> result;
        for (size_t i = 0; i < (std::min)(count, components.size()); i++) {
            result.push_back(components[i].first);
        }
        return result;
    }

    std::vector<ProfileComponent> PerformanceProfiler::GetMostCalledComponents(size_t count) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Collect all components with data
        std::vector<std::pair<ProfileComponent, size_t>> components;
        for (const auto& [component, stats] : m_stats) {
            if (stats.callCount > 0) {
                components.push_back({ component, stats.callCount });
            }
        }

        // Sort by call count (descending)
        std::sort(components.begin(), components.end(),
            [](const auto& a, const auto& b) {
                return a.second > b.second;
            });

        // Extract top N components
        std::vector<ProfileComponent> result;
        for (size_t i = 0; i < (std::min)(count, components.size()); i++) {
            result.push_back(components[i].first);
        }
        return result;
    }

    double PerformanceProfiler::GetTotalTimeMs() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        double total = 0.0;
        for (const auto& [component, stats] : m_stats) {
            if (stats.callCount > 0) {
                total += stats.totalTimeMs;
            }
        }
        return total;
    }

    // ============================================================================
    // HTML Report Generation
    // ============================================================================

    std::wstring PerformanceProfiler::GenerateHTMLReport() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::wostringstream html;

        html << L"<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>ExplorerLens Performance Report</title>" << std::endl;
        html << L"<style>body{font-family:Arial,sans-serif;margin:20px;background:#f5f5f5;}" << std::endl;
        html << L"h1{color:#2c3e50;}table{border-collapse:collapse;width:100%;background:white;box-shadow:0 2px 4px rgba(0,0,0,0.1);}" << std::endl;
        html << L"th{background:#3498db;color:white;padding:12px;text-align:left;}td{padding:10px;border-bottom:1px solid #ddd;}" << std::endl;
        html << L"tr:hover{background:#f8f9fa;}.metric{font-weight:bold;color:#27ae60;}" << std::endl;
        html << L"</style></head><body>" << std::endl;

        html << L"<h1>ExplorerLens Performance Report</h1>" << std::endl;

        // Summary metrics
        double totalTime = GetTotalTimeMs();
        size_t totalMem = GetTotalMemoryUsage();
        
        html << L"<div style=\"background:white;padding:20px;margin-bottom:20px;border-radius:8px;\">" << std::endl;
        html << L"<h2>Summary</h2>" << std::endl;
        html << L"<p><span class=\"metric\">Total Time:</span> " << std::fixed << std::setprecision(2) << totalTime << L" ms</p>" << std::endl;
        html << L"<p><span class=\"metric\">Total Memory:</span> " << (totalMem / 1024.0 / 1024.0) << L" MB</p>" << std::endl;
        html << L"</div>" << std::endl;

        // Performance table
        html << L"<table>" << std::endl;
        html << L"<tr><th>Component</th><th>Calls</th><th>Total (ms)</th><th>Avg (ms)</th><th>Min (ms)</th><th>Max (ms)</th><th>Memory (KB)</th></tr>" << std::endl;

        // Sort by total time
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

        for (const auto& [component, stats] : sortedStats) {
            size_t mem = GetMemoryUsage(component);
            html << L"<tr><td>" << stats.name << L"</td>";
            html << L"<td>" << stats.callCount << L"</td>";
            html << L"<td>" << std::fixed << std::setprecision(2) << stats.totalTimeMs << L"</td>";
            html << L"<td>" << std::fixed << std::setprecision(2) << stats.avgTimeMs << L"</td>";
            html << L"<td>" << std::fixed << std::setprecision(2) << stats.minTimeMs << L"</td>";
            html << L"<td>" << std::fixed << std::setprecision(2) << stats.maxTimeMs << L"</td>";
            html << L"<td>" << (mem / 1024.0) << L"</td></tr>" << std::endl;
        }

        html << L"</table></body></html>" << std::endl;
        return html.str();
    }

    bool PerformanceProfiler::ExportHTMLReport(const std::wstring& filePath) const
    {
        try {
            std::wofstream file(filePath);
            if (!file.is_open()) {
                return false;
            }
            file << GenerateHTMLReport();
            file.close();
            return true;
        }
        catch (...) {
            return false;
        }
    }

    // ============================================================================
    // CSV Report Generation
    // ============================================================================

    std::wstring PerformanceProfiler::GenerateCSVReport() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::wostringstream csv;

        // Header
        csv << L"Component,Calls,Total(ms),Avg(ms),Min(ms),Max(ms),Memory(KB)" << std::endl;

        // Data rows
        for (const auto& [component, stats] : m_stats) {
            if (stats.callCount > 0) {
                size_t mem = GetMemoryUsage(component);
                csv << stats.name << L",";
                csv << stats.callCount << L",";
                csv << std::fixed << std::setprecision(2) << stats.totalTimeMs << L",";
                csv << std::fixed << std::setprecision(2) << stats.avgTimeMs << L",";
                csv << std::fixed << std::setprecision(2) << stats.minTimeMs << L",";
                csv << std::fixed << std::setprecision(2) << stats.maxTimeMs << L",";
                csv << (mem / 1024.0) << std::endl;
            }
        }

        return csv.str();
    }

    bool PerformanceProfiler::ExportCSVReport(const std::wstring& filePath) const
    {
        try {
            std::wofstream file(filePath);
            if (!file.is_open()) {
                return false;
            }
            file << GenerateCSVReport();
            file.close();
            return true;
        }
        catch (...) {
            return false;
        }
    }

    // ============================================================================
    // Performance Hints
    // ============================================================================

    std::vector<std::wstring> PerformanceProfiler::GetPerformanceHints() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<std::wstring> hints;

        // Analyze slow decoders
        auto slowest = GetSlowestComponents(3);
        for (auto component : slowest) {
            const auto& stats = m_stats.at(component);
            if (stats.avgTimeMs > 100.0) {
                hints.push_back(L"Slow decoder detected: " + stats.name + L" (avg " +
                               std::to_wstring(static_cast<int>(stats.avgTimeMs)) + L"ms). Consider enabling thumbnail extraction or GPU acceleration.");
            }
        }

        // Check cache effectiveness
        auto cacheStats = m_stats.find(ProfileComponent::CACHE_LOOKUP);
        if (cacheStats != m_stats.end() && cacheStats->second.callCount > 100) {
            double cacheTime = cacheStats->second.avgTimeMs;
            if (cacheTime > 5.0) {
                hints.push_back(L"Cache lookup is slow (" + std::to_wstring(static_cast<int>(cacheTime)) +
                               L"ms). Consider optimizing cache storage or indexing.");
            }
        }

        // Check memory usage
        size_t totalMem = GetTotalMemoryUsage();
        if (totalMem > 512 * 1024 * 1024) { // > 512 MB
            hints.push_back(L"High memory usage detected (" + std::to_wstring(totalMem / 1024 / 1024) +
                           L" MB). Consider implementing memory pooling or reducing cache size.");
        }

        // Check GPU usage
        auto gpuStats = m_stats.find(ProfileComponent::GPU_RENDER_D3D11);
        auto gdiStats = m_stats.find(ProfileComponent::GPU_RENDER_GDI);
        if (gpuStats != m_stats.end() && gdiStats != m_stats.end()) {
            if (gpuStats->second.callCount < gdiStats->second.callCount / 10) {
                hints.push_back(L"GPU acceleration is underutilized. Most rendering uses GDI+. Enable D3D11 for better performance.");
            }
        }

        // Check for performance outliers (high max vs avg)
        for (const auto& [component, stats] : m_stats) {
            if (stats.callCount > 10 && stats.maxTimeMs > stats.avgTimeMs * 5.0) {
                hints.push_back(L"Performance outlier detected in " + stats.name + L". Max time (" +
                               std::to_wstring(static_cast<int>(stats.maxTimeMs)) + L"ms) much higher than average (" +
                               std::to_wstring(static_cast<int>(stats.avgTimeMs)) + L"ms). Investigate edge cases.");
            }
        }

        if (hints.empty()) {
            hints.push_back(L"Performance looks good! No optimization suggestions at this time.");
        }

        return hints;
    }

} // namespace ExplorerLens

