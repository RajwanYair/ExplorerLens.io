// ErrorCategorizationEngine.h — Decode Error Classification & Grouping
// Copyright (c) 2026 ExplorerLens Project
//
// Categorizes decode errors into actionable groups: format errors, I/O
// failures, memory issues, GPU faults, timeout, and permission denied.
// Tracks error rates per format and provides trend analysis for
// observability dashboards.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

enum class ErrorCategory : uint8_t {
    FormatCorruption,  // Malformed file data
    IOFailure,         // File read/seek errors
    MemoryExhaustion,  // Alloc failures
    GPUFault,          // GPU decode/render error
    Timeout,           // Decode exceeded time limit
    PermissionDenied,  // Access denied / locked file
    UnsupportedFormat, // Format not recognized
    InternalBug,       // Assertion / logic error
    Unknown
};

struct ErrorRecord {
    ErrorCategory category = ErrorCategory::Unknown;
    uint32_t      hresult = 0;
    std::wstring  filePath;
    std::wstring  formatName;
    std::string   message;
    uint64_t      timestamp = 0;
};

struct ErrorCategoryStats {
    ErrorCategory category = ErrorCategory::Unknown;
    uint64_t      count = 0;
    double         ratePerMinute = 0.0;
};

struct ErrorEngineStats {
    uint64_t totalErrors = 0;
    uint64_t totalSuccesses = 0;
    double   errorRate = 0.0;
    std::vector<ErrorCategoryStats> byCategory;
};

class ErrorCategorizationEngine {
public:
    ErrorCategorizationEngine() {
        InitializeSRWLock(&m_lock);
        m_startTick = GetTickCount64();
    }
    ~ErrorCategorizationEngine() = default;

    static const wchar_t* GetName() { return L"ErrorCategorizationEngine"; }

    /// Classify an HRESULT into an error category.
    ErrorCategory Classify(uint32_t hr) const {
        switch (hr) {
        case 0x80070005: return ErrorCategory::PermissionDenied;   // E_ACCESSDENIED
        case 0x8007000E: return ErrorCategory::MemoryExhaustion;   // E_OUTOFMEMORY
        case 0x80004005: return ErrorCategory::InternalBug;        // E_FAIL
        case 0x80070006: return ErrorCategory::IOFailure;          // E_HANDLE
        case 0x80070057: return ErrorCategory::FormatCorruption;   // E_INVALIDARG
        case 0x887A0002: return ErrorCategory::GPUFault;           // DXGI_ERROR_NOT_FOUND
        case 0x887A0005: return ErrorCategory::GPUFault;           // DXGI_ERROR_DEVICE_REMOVED
        case 0x887A0007: return ErrorCategory::GPUFault;           // DXGI_ERROR_DEVICE_RESET
        default: return ErrorCategory::Unknown;
        }
    }

    /// Record an error occurrence.
    void RecordError(const ErrorRecord& record) {
        AcquireSRWLockExclusive(&m_lock);
        m_recentErrors.push_back(record);
        if (m_recentErrors.size() > 1000) m_recentErrors.erase(m_recentErrors.begin());
        m_categoryCounts[static_cast<uint8_t>(record.category)]++;
        m_totalErrors++;
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Record a successful decode.
    void RecordSuccess() { m_totalSuccesses++; }

    /// Get per-category error stats.
    ErrorEngineStats GetStats() const {
        ErrorEngineStats stats;
        stats.totalErrors = m_totalErrors;
        stats.totalSuccesses = m_totalSuccesses;
        uint64_t total = stats.totalErrors + stats.totalSuccesses;
        stats.errorRate = total > 0 ? (100.0 * stats.totalErrors / total) : 0.0;

        double elapsedMinutes = (GetTickCount64() - m_startTick) / 60000.0;
        if (elapsedMinutes < 0.001) elapsedMinutes = 0.001;

        for (uint8_t i = 0; i <= static_cast<uint8_t>(ErrorCategory::Unknown); ++i) {
            auto it = m_categoryCounts.find(i);
            if (it != m_categoryCounts.end() && it->second > 0) {
                ErrorCategoryStats cs;
                cs.category = static_cast<ErrorCategory>(i);
                cs.count = it->second;
                cs.ratePerMinute = cs.count / elapsedMinutes;
                stats.byCategory.push_back(cs);
            }
        }
        return stats;
    }

    /// Get the category name as a string.
    static const char* CategoryName(ErrorCategory cat) {
        switch (cat) {
        case ErrorCategory::FormatCorruption:  return "FormatCorruption";
        case ErrorCategory::IOFailure:         return "IOFailure";
        case ErrorCategory::MemoryExhaustion:  return "MemoryExhaustion";
        case ErrorCategory::GPUFault:          return "GPUFault";
        case ErrorCategory::Timeout:           return "Timeout";
        case ErrorCategory::PermissionDenied:  return "PermissionDenied";
        case ErrorCategory::UnsupportedFormat: return "UnsupportedFormat";
        case ErrorCategory::InternalBug:       return "InternalBug";
        default: return "Unknown";
        }
    }

private:
    SRWLOCK m_lock{};
    uint64_t m_startTick = 0;
    uint64_t m_totalErrors = 0;
    uint64_t m_totalSuccesses = 0;
    std::vector<ErrorRecord> m_recentErrors;
    mutable std::unordered_map<uint8_t, uint64_t> m_categoryCounts;
};

} // namespace Engine
} // namespace ExplorerLens
