// PipelineErrorBoundary.h — Error boundaries for pipeline stages
// Copyright (c) 2026 ExplorerLens Project
//
// Wraps each pipeline stage in an error boundary that catches failures without
// crashing the whole pipeline. Provides ErrorBoundary<T> with fallback values,
// PipelineStageResult enum, and stage-level error isolation with metrics.
//
#pragma once

#include <cstdint>
#include <string>
#include <functional>
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>
#include <windows.h>
#include "../Core/StructuredErrorDomain.h"
#include "../Core/ResultType.h"

namespace ExplorerLens {
namespace Engine {

/// Outcome of a pipeline stage execution
enum class PipelineStageResult : uint8_t {
    Success         = 0,   // Stage completed successfully
    PartialSuccess  = 1,   // Stage completed with degraded output
    NonFatalError   = 2,   // Stage failed but pipeline can continue
    FatalError      = 3,   // Stage failed fatally — pipeline must stop
    COUNT           = 4
};

/// Human-readable name for PipelineStageResult
inline const char* PipelineStageResultName(PipelineStageResult r) noexcept {
    switch (r) {
    case PipelineStageResult::Success:        return "Success";
    case PipelineStageResult::PartialSuccess: return "PartialSuccess";
    case PipelineStageResult::NonFatalError:  return "NonFatalError";
    case PipelineStageResult::FatalError:     return "FatalError";
    default:                                  return "Unknown";
    }
}

/// Configuration for how an error boundary behaves
struct ErrorBoundaryConfig {
    bool     useFallbackOnError  = true;    // Use fallback value on non-fatal error
    bool     promoteFatalOnRepeat = true;   // Promote to fatal after N consecutive errors
    uint32_t consecutiveErrorThreshold = 5; // Threshold for fatal promotion
    bool     recordMetrics       = true;    // Track success/failure metrics
};

/// Metrics tracked by an ErrorBoundary
struct ErrorBoundaryMetrics {
    std::string stageName;
    uint64_t    totalExecutions      = 0;
    uint64_t    successCount         = 0;
    uint64_t    partialSuccessCount  = 0;
    uint64_t    nonFatalErrorCount   = 0;
    uint64_t    fatalErrorCount      = 0;
    uint64_t    fallbackUsed         = 0;
    uint32_t    consecutiveErrors    = 0;  // Current streak
    double      totalExecutionTimeMs = 0.0;

    /// Success rate as fraction [0.0, 1.0]
    double SuccessRate() const noexcept {
        return (totalExecutions > 0)
            ? static_cast<double>(successCount + partialSuccessCount)
              / static_cast<double>(totalExecutions)
            : 1.0;
    }

    /// Average execution time in milliseconds
    double AverageExecutionTimeMs() const noexcept {
        return (totalExecutions > 0)
            ? totalExecutionTimeMs / static_cast<double>(totalExecutions)
            : 0.0;
    }
};

/// Error boundary that wraps a pipeline stage, providing error isolation,
/// fallback values, and execution metrics.
template <typename T>
class ErrorBoundary {
public:
    /// @param stageName  Human-readable name for the stage
    /// @param fallbackValue  Default value to return on non-fatal error
    /// @param config  Boundary behavior configuration
    ErrorBoundary(const std::string& stageName,
                  const T& fallbackValue,
                  ErrorBoundaryConfig config = {})
        : m_fallbackValue(fallbackValue)
        , m_config(config)
    {
        m_metrics.stageName = stageName;
    }

    /// Execute the stage operation within the error boundary.
    /// On success: returns the result with PipelineStageResult::Success.
    /// On error: applies recovery policy and returns fallback if configured.
    struct BoundaryResult {
        PipelineStageResult stageResult = PipelineStageResult::Success;
        T                   value{};
        StructuredError     error;
        bool                fallbackUsed = false;
    };

    BoundaryResult Execute(std::function<Result<T>()> stageOperation) {
        auto startTime = std::chrono::steady_clock::now();
        BoundaryResult boundary;

        ++m_metrics.totalExecutions;

        auto result = stageOperation();

        auto endTime = std::chrono::steady_clock::now();
        double elapsedMs = std::chrono::duration<double, std::milli>(
            endTime - startTime).count();
        m_metrics.totalExecutionTimeMs += elapsedMs;

        if (result.IsOk()) {
            boundary.stageResult = PipelineStageResult::Success;
            boundary.value = result.Value();
            m_metrics.successCount++;
            m_metrics.consecutiveErrors = 0;
            return boundary;
        }

        // Error path
        boundary.error = result.Error();
        m_metrics.consecutiveErrors++;

        // Check if this should be fatal
        bool isFatal = result.Error().GetSeverity() == ErrorSeverity::Fatal;
        if (m_config.promoteFatalOnRepeat &&
            m_metrics.consecutiveErrors >= m_config.consecutiveErrorThreshold) {
            isFatal = true;
        }

        if (isFatal) {
            boundary.stageResult = PipelineStageResult::FatalError;
            m_metrics.fatalErrorCount++;
            return boundary;
        }

        // Non-fatal error — attempt fallback
        if (m_config.useFallbackOnError) {
            boundary.stageResult = PipelineStageResult::NonFatalError;
            boundary.value = m_fallbackValue;
            boundary.fallbackUsed = true;
            m_metrics.nonFatalErrorCount++;
            m_metrics.fallbackUsed++;
        } else {
            boundary.stageResult = PipelineStageResult::NonFatalError;
            m_metrics.nonFatalErrorCount++;
        }

        return boundary;
    }

    /// Get the current metrics for this boundary
    const ErrorBoundaryMetrics& GetMetrics() const noexcept { return m_metrics; }

    /// Reset metrics (for testing or periodic reset)
    void ResetMetrics() noexcept {
        auto name = m_metrics.stageName;
        m_metrics = ErrorBoundaryMetrics{};
        m_metrics.stageName = name;
    }

    /// Get the stage name
    const std::string& GetStageName() const noexcept { return m_metrics.stageName; }

private:
    T                    m_fallbackValue;
    ErrorBoundaryConfig  m_config;
    ErrorBoundaryMetrics m_metrics;
};

/// Manages a collection of pipeline error boundaries and provides
/// aggregate metrics across all stages.
class PipelineErrorBoundaryManager {
public:
    PipelineErrorBoundaryManager() = default;

    /// Record a stage execution result (for aggregate tracking)
    void RecordStageExecution(const std::string& stageName,
                              PipelineStageResult result) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto& entry = FindOrCreateEntry(stageName);
        entry.totalExecutions++;
        switch (result) {
        case PipelineStageResult::Success:
            entry.successCount++;
            entry.consecutiveErrors = 0;
            break;
        case PipelineStageResult::PartialSuccess:
            entry.partialSuccessCount++;
            entry.consecutiveErrors = 0;
            break;
        case PipelineStageResult::NonFatalError:
            entry.nonFatalErrorCount++;
            entry.consecutiveErrors++;
            break;
        case PipelineStageResult::FatalError:
            entry.fatalErrorCount++;
            entry.consecutiveErrors++;
            break;
        default:
            break;
        }
    }

    /// Get aggregate metrics for all tracked stages
    std::vector<ErrorBoundaryMetrics> GetAllMetrics() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stageMetrics;
    }

    /// Get metrics for a specific stage (returns empty metrics if not found)
    ErrorBoundaryMetrics GetStageMetrics(const std::string& stageName) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& m : m_stageMetrics) {
            if (m.stageName == stageName)
                return m;
        }
        return ErrorBoundaryMetrics{};
    }

    /// Number of tracked stages
    size_t GetStageCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stageMetrics.size();
    }

    /// Check if any stage has reached fatal error state
    bool HasFatalStage() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& m : m_stageMetrics) {
            if (m.fatalErrorCount > 0)
                return true;
        }
        return false;
    }

    /// Overall pipeline health — true if all stages have >90% success rate
    bool IsPipelineHealthy() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& m : m_stageMetrics) {
            if (m.totalExecutions > 0 && m.SuccessRate() < 0.9)
                return false;
        }
        return true;
    }

    /// Reset all metrics
    void Reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stageMetrics.clear();
    }

private:
    ErrorBoundaryMetrics& FindOrCreateEntry(const std::string& stageName) {
        for (auto& m : m_stageMetrics) {
            if (m.stageName == stageName)
                return m;
        }
        m_stageMetrics.push_back(ErrorBoundaryMetrics{});
        m_stageMetrics.back().stageName = stageName;
        return m_stageMetrics.back();
    }

    mutable std::mutex                  m_mutex;
    std::vector<ErrorBoundaryMetrics>   m_stageMetrics;
};

} // namespace Engine
} // namespace ExplorerLens
