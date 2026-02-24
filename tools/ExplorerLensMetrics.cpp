// ExplorerLens Metrics Viewer - Command-line tool to view performance metrics
// Reads shared metrics from the shell extension and displays them

#include <windows.h>
#include <iostream>
#include <iomanip>
#include <psapi.h>
#include "../LENSShell/metrics_collector.h"

using namespace ExplorerLens;

void PrintHeader(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << title << "\n";
    std::cout << std::string(60, '=') << "\n";
}

void DisplayMetrics() {
    const auto& metrics = MetricsCollector::Instance().GetMetrics();
    
    PrintHeader("ExplorerLens Performance Metrics");
    
    std::cout << "\n📊 Thumbnail Generation\n";
    std::cout << "  Total Attempts:  " << metrics.totalAttempts.load() << "\n";
    std::cout << "  Successful:      " << metrics.successCount.load() 
              << " (" << std::fixed << std::setprecision(1) 
              << metrics.GetSuccessRate() << "%)\n";
    std::cout << "  Failed:          " << metrics.failureCount.load() << "\n";
    
    if (metrics.cacheHits.load() + metrics.cacheMisses.load() > 0) {
        std::cout << "\n💾 Cache Performance\n";
        std::cout << "  Cache Hits:      " << metrics.cacheHits.load() << "\n";
        std::cout << "  Cache Misses:    " << metrics.cacheMisses.load() << "\n";
        std::cout << "  Hit Rate:        " << metrics.GetCacheHitRate() << "%\n";
    }
    
    uint64_t totalFormats = metrics.zipCount + metrics.rarCount + metrics.cbzCount + 
                            metrics.cbrCount + metrics.sevenzCount + metrics.webpCount +
                            metrics.avifCount + metrics.jxlCount;
    
    if (totalFormats > 0) {
        std::cout << "\n📁 Format Breakdown\n";
        if (metrics.zipCount > 0) 
            std::cout << "  ZIP:   " << std::setw(6) << metrics.zipCount.load() << "\n";
        if (metrics.rarCount > 0) 
            std::cout << "  RAR:   " << std::setw(6) << metrics.rarCount.load() << "\n";
        if (metrics.cbzCount > 0) 
            std::cout << "  CBZ:   " << std::setw(6) << metrics.cbzCount.load() << "\n";
        if (metrics.cbrCount > 0) 
            std::cout << "  CBR:   " << std::setw(6) << metrics.cbrCount.load() << "\n";
        if (metrics.sevenzCount > 0) 
            std::cout << "  7Z:    " << std::setw(6) << metrics.sevenzCount.load() << "\n";
        if (metrics.webpCount > 0) 
            std::cout << "  WebP:  " << std::setw(6) << metrics.webpCount.load() << "\n";
        if (metrics.avifCount > 0) 
            std::cout << "  AVIF:  " << std::setw(6) << metrics.avifCount.load() << "\n";
        if (metrics.jxlCount > 0) 
            std::cout << "  JXL:   " << std::setw(6) << metrics.jxlCount.load() << "\n";
    }
    
    std::cout << "\n💻 Memory Usage\n";
    std::cout << "  Peak Memory:     " << std::fixed << std::setprecision(2)
              << (metrics.peakMemoryBytes.load() / 1024.0 / 1024.0) << " MB\n";
    std::cout << "  Total Allocated: " 
              << (metrics.totalMemoryAllocated.load() / 1024.0 / 1024.0) << " MB\n";
    
    // Get current process memory
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        std::cout << "  Current Usage:   " 
                  << (pmc.WorkingSetSize / 1024.0 / 1024.0) << " MB\n";
    }
    
    std::cout << "\n" << std::string(60, '=') << "\n\n";
}

void ExportMetrics(const std::string& format, const std::wstring& filename) {
    bool success = false;
    
    if (format == "csv") {
        success = MetricsCollector::Instance().ExportToCSV(filename);
    } else if (format == "json") {
        success = MetricsCollector::Instance().ExportToJSON(filename);
    }
    
    if (success) {
        std::wcout << L"✓ Metrics exported to: " << filename << "\n";
    } else {
        std::wcout << L"✗ Failed to export metrics to: " << filename << "\n";
    }
}

void ResetMetrics() {
    MetricsCollector::Instance().Reset();
    std::cout << "✓ Metrics reset\n";
}

void PrintUsage() {
    std::cout << "ExplorerLens Metrics Viewer\n";
    std::cout << "Usage: ExplorerLensMetrics.exe [command] [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  show              Display current metrics (default)\n";
    std::cout << "  export csv <file> Export metrics to CSV file\n";
    std::cout << "  export json <file> Export metrics to JSON file\n";
    std::cout << "  reset             Reset all metrics\n";
    std::cout << "  help              Show this help\n\n";
    std::cout << "Examples:\n";
    std::cout << "  ExplorerLensMetrics.exe\n";
    std::cout << "  ExplorerLensMetrics.exe show\n";
    std::cout << "  ExplorerLensMetrics.exe export csv metrics.csv\n";
    std::cout << "  ExplorerLensMetrics.exe export json metrics.json\n";
    std::cout << "  ExplorerLensMetrics.exe reset\n";
}

int wmain(int argc, wchar_t* argv[]) {
    // Set console to UTF-8 for emoji support
    SetConsoleOutputCP(CP_UTF8);
    
    if (argc == 1) {
        // No arguments - show metrics
        DisplayMetrics();
        return 0;
    }
    
    std::wstring command = argv[1];
    
    if (command == L"show") {
        DisplayMetrics();
    }
    else if (command == L"export" && argc >= 4) {
        std::wstring format = argv[2];
        std::wstring filename = argv[3];
        
        if (format == L"csv") {
            ExportMetrics("csv", filename);
        } else if (format == L"json") {
            ExportMetrics("json", filename);
        } else {
            std::wcout << L"Unknown export format: " << format << "\n";
            std::wcout << L"Supported formats: csv, json\n";
            return 1;
        }
    }
    else if (command == L"reset") {
        ResetMetrics();
    }
    else if (command == L"help" || command == L"-h" || command == L"--help") {
        PrintUsage();
    }
    else {
        std::wcout << L"Unknown command: " << command << "\n\n";
        PrintUsage();
        return 1;
    }
    
    return 0;
}

