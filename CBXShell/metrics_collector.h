// Enhanced Performance Metrics with Success/Failure Tracking and Memory Monitoring
// Extension to performance_profiler.h for comprehensive thumbnail generation metrics

#pragma once

#include "performance_profiler.h"
#include <psapi.h>
#include <atomic>
#include <fstream>

namespace DarkThumbs {

// Extended metrics for thumbnail generation
struct ThumbnailMetrics {
    std::atomic<uint64_t> totalAttempts{0};
    std::atomic<uint64_t> successCount{0};
    std::atomic<uint64_t> failureCount{0};
    std::atomic<uint64_t> cacheHits{0};
    std::atomic<uint64_t> cacheMisses{0};
    
    // Format-specific counters
    std::atomic<uint64_t> zipCount{0};
    std::atomic<uint64_t> rarCount{0};
    std::atomic<uint64_t> cbzCount{0};
    std::atomic<uint64_t> cbrCount{0};
    std::atomic<uint64_t> sevenzCount{0};
    std::atomic<uint64_t> webpCount{0};
    std::atomic<uint64_t> avifCount{0};
    std::atomic<uint64_t> jxlCount{0};
    
    // Memory tracking
    std::atomic<uint64_t> peakMemoryBytes{0};
    std::atomic<uint64_t> totalMemoryAllocated{0};
    
    double GetSuccessRate() const {
        uint64_t total = totalAttempts.load();
        return total > 0 ? (double)successCount.load() / total * 100.0 : 0.0;
    }
    
    double GetCacheHitRate() const {
        uint64_t total = cacheHits.load() + cacheMisses.load();
        return total > 0 ? (double)cacheHits.load() / total * 100.0 : 0.0;
    }
};

class MetricsCollector {
private:
    ThumbnailMetrics m_metrics;
    std::mutex m_mutex;
    std::chrono::system_clock::time_point m_startTime;
    
    MetricsCollector() : m_startTime(std::chrono::system_clock::now()) {}
    
    SIZE_T GetCurrentMemoryUsage() {
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
            return pmc.WorkingSetSize;
        }
        return 0;
    }

public:
    static MetricsCollector& Instance() {
        static MetricsCollector instance;
        return instance;
    }
    
    void RecordAttempt(const std::string& format, bool success, SIZE_T memoryUsed = 0) {
        m_metrics.totalAttempts++;
        
        if (success) {
            m_metrics.successCount++;
        } else {
            m_metrics.failureCount++;
        }
        
        // Track format-specific metrics
        if (format == "zip" || format == ".zip") m_metrics.zipCount++;
        else if (format == "rar" || format == ".rar") m_metrics.rarCount++;
        else if (format == "cbz" || format == ".cbz") m_metrics.cbzCount++;
        else if (format == "cbr" || format == ".cbr") m_metrics.cbrCount++;
        else if (format == "7z" || format == ".7z") m_metrics.sevenzCount++;
        else if (format == "webp" || format == ".webp") m_metrics.webpCount++;
        else if (format == "avif" || format == ".avif") m_metrics.avifCount++;
        else if (format == "jxl" || format == ".jxl") m_metrics.jxlCount++;
        
        // Track memory
        if (memoryUsed > 0) {
            m_metrics.totalMemoryAllocated += memoryUsed;
            uint64_t current = m_metrics.peakMemoryBytes.load();
            while (memoryUsed > current && 
                   !m_metrics.peakMemoryBytes.compare_exchange_weak(current, memoryUsed)) {}
        }
    }
    
    void RecordCacheHit(bool hit) {
        if (hit) {
            m_metrics.cacheHits++;
        } else {
            m_metrics.cacheMisses++;
        }
    }
    
    void UpdateMemoryPeak() {
        SIZE_T current = GetCurrentMemoryUsage();
        uint64_t peak = m_metrics.peakMemoryBytes.load();
        while (current > peak && 
               !m_metrics.peakMemoryBytes.compare_exchange_weak(peak, current)) {}
    }
    
    const ThumbnailMetrics& GetMetrics() const {
        return m_metrics;
    }
    
    void Reset() {
        m_metrics = ThumbnailMetrics();
        m_startTime = std::chrono::system_clock::now();
    }
    
    std::string GetSummary() const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        
        oss << "DarkThumbs Performance Metrics\n";
        oss << "==============================\n\n";
        
        auto now = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::hours>(now - m_startTime);
        oss << "Session Duration: " << elapsed.count() << " hours\n\n";
        
        oss << "Thumbnail Generation:\n";
        oss << "  Total Attempts:  " << m_metrics.totalAttempts.load() << "\n";
        oss << "  Successful:      " << m_metrics.successCount.load() 
            << " (" << m_metrics.GetSuccessRate() << "%)\n";
        oss << "  Failed:          " << m_metrics.failureCount.load() << "\n\n";
        
        oss << "Cache Performance:\n";
        oss << "  Cache Hits:      " << m_metrics.cacheHits.load() << "\n";
        oss << "  Cache Misses:    " << m_metrics.cacheMisses.load() << "\n";
        oss << "  Hit Rate:        " << m_metrics.GetCacheHitRate() << "%\n\n";
        
        oss << "Format Breakdown:\n";
        if (m_metrics.zipCount > 0) oss << "  ZIP:   " << m_metrics.zipCount.load() << "\n";
        if (m_metrics.rarCount > 0) oss << "  RAR:   " << m_metrics.rarCount.load() << "\n";
        if (m_metrics.cbzCount > 0) oss << "  CBZ:   " << m_metrics.cbzCount.load() << "\n";
        if (m_metrics.cbrCount > 0) oss << "  CBR:   " << m_metrics.cbrCount.load() << "\n";
        if (m_metrics.sevenzCount > 0) oss << "  7Z:    " << m_metrics.sevenzCount.load() << "\n";
        if (m_metrics.webpCount > 0) oss << "  WebP:  " << m_metrics.webpCount.load() << "\n";
        if (m_metrics.avifCount > 0) oss << "  AVIF:  " << m_metrics.avifCount.load() << "\n";
        if (m_metrics.jxlCount > 0) oss << "  JXL:   " << m_metrics.jxlCount.load() << "\n";
        oss << "\n";
        
        oss << "Memory Usage:\n";
        oss << "  Peak:            " << (m_metrics.peakMemoryBytes.load() / 1024.0 / 1024.0) << " MB\n";
        oss << "  Total Allocated: " << (m_metrics.totalMemoryAllocated.load() / 1024.0 / 1024.0) << " MB\n";
        
        return oss.str();
    }
    
    bool ExportToCSV(const std::wstring& filePath) const {
        try {
            std::ofstream file(filePath);
            if (!file.is_open()) return false;
            
            file << "Metric,Value\n";
            file << "TotalAttempts," << m_metrics.totalAttempts.load() << "\n";
            file << "SuccessCount," << m_metrics.successCount.load() << "\n";
            file << "FailureCount," << m_metrics.failureCount.load() << "\n";
            file << "SuccessRate," << m_metrics.GetSuccessRate() << "\n";
            file << "CacheHits," << m_metrics.cacheHits.load() << "\n";
            file << "CacheMisses," << m_metrics.cacheMisses.load() << "\n";
            file << "CacheHitRate," << m_metrics.GetCacheHitRate() << "\n";
            file << "ZIP_Count," << m_metrics.zipCount.load() << "\n";
            file << "RAR_Count," << m_metrics.rarCount.load() << "\n";
            file << "CBZ_Count," << m_metrics.cbzCount.load() << "\n";
            file << "CBR_Count," << m_metrics.cbrCount.load() << "\n";
            file << "7Z_Count," << m_metrics.sevenzCount.load() << "\n";
            file << "WebP_Count," << m_metrics.webpCount.load() << "\n";
            file << "AVIF_Count," << m_metrics.avifCount.load() << "\n";
            file << "JXL_Count," << m_metrics.jxlCount.load() << "\n";
            file << "PeakMemoryMB," << (m_metrics.peakMemoryBytes.load() / 1024.0 / 1024.0) << "\n";
            file << "TotalAllocatedMB," << (m_metrics.totalMemoryAllocated.load() / 1024.0 / 1024.0) << "\n";
            
            file.close();
            return true;
        } catch (...) {
            return false;
        }
    }
    
    bool ExportToJSON(const std::wstring& filePath) const {
        try {
            std::ofstream file(filePath);
            if (!file.is_open()) return false;
            
            file << "{\n";
            file << "  \"thumbnailGeneration\": {\n";
            file << "    \"totalAttempts\": " << m_metrics.totalAttempts.load() << ",\n";
            file << "    \"successCount\": " << m_metrics.successCount.load() << ",\n";
            file << "    \"failureCount\": " << m_metrics.failureCount.load() << ",\n";
            file << "    \"successRate\": " << m_metrics.GetSuccessRate() << "\n";
            file << "  },\n";
            file << "  \"cache\": {\n";
            file << "    \"hits\": " << m_metrics.cacheHits.load() << ",\n";
            file << "    \"misses\": " << m_metrics.cacheMisses.load() << ",\n";
            file << "    \"hitRate\": " << m_metrics.GetCacheHitRate() << "\n";
            file << "  },\n";
            file << "  \"formatBreakdown\": {\n";
            file << "    \"zip\": " << m_metrics.zipCount.load() << ",\n";
            file << "    \"rar\": " << m_metrics.rarCount.load() << ",\n";
            file << "    \"cbz\": " << m_metrics.cbzCount.load() << ",\n";
            file << "    \"cbr\": " << m_metrics.cbrCount.load() << ",\n";
            file << "    \"7z\": " << m_metrics.sevenzCount.load() << ",\n";
            file << "    \"webp\": " << m_metrics.webpCount.load() << ",\n";
            file << "    \"avif\": " << m_metrics.avifCount.load() << ",\n";
            file << "    \"jxl\": " << m_metrics.jxlCount.load() << "\n";
            file << "  },\n";
            file << "  \"memory\": {\n";
            file << "    \"peakMB\": " << (m_metrics.peakMemoryBytes.load() / 1024.0 / 1024.0) << ",\n";
            file << "    \"totalAllocatedMB\": " << (m_metrics.totalMemoryAllocated.load() / 1024.0 / 1024.0) << "\n";
            file << "  }\n";
            file << "}\n";
            
            file.close();
            return true;
        } catch (...) {
            return false;
        }
    }
};

// RAII helper for recording thumbnail generation attempts
class ThumbnailMetricsScope {
private:
    std::string m_format;
    SIZE_T m_memoryBefore;
    bool m_success;
    
public:
    explicit ThumbnailMetricsScope(const std::string& format) 
        : m_format(format)
        , m_success(false)
    {
        PROCESS_MEMORY_COUNTERS pmc;
        m_memoryBefore = 0;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
            m_memoryBefore = pmc.WorkingSetSize;
        }
    }
    
    ~ThumbnailMetricsScope() {
        SIZE_T memoryAfter = 0;
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
            memoryAfter = pmc.WorkingSetSize;
        }
        
        SIZE_T memoryUsed = (memoryAfter > m_memoryBefore) ? 
                            (memoryAfter - m_memoryBefore) : 0;
        
        MetricsCollector::Instance().RecordAttempt(m_format, m_success, memoryUsed);
        MetricsCollector::Instance().UpdateMemoryPeak();
    }
    
    void SetSuccess(bool success) { m_success = success; }
};

} // namespace DarkThumbs
