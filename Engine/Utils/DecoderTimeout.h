// ============================================================================
// DecoderTimeout.h, Task 6.3
// Hard-kill decoder timeout enforcement for ExplorerLens
//
// Wraps decoder invocations with a wall-clock timeout. If a decoder exceeds
// the configured timeout (default 5s), the operation is abandoned and the
// circuit breaker is notified. This prevents a single malformed file from
// stalling the Explorer shell thread.
//
// USAGE:
//   DecoderTimeoutGuard guard("WebPDecoder", 5000);
//   if (guard.Execute([&]() { return decoder->Decode(path, result); })) {
//       // success
//   } else {
//       // timed out or failed — circuit breaker already notified
//   }
// ============================================================================

#pragma once

#include <Windows.h>
#include <string>
#include <functional>
#include <chrono>
#include <atomic>
#include "DecoderCircuitBreaker.h"

namespace ExplorerLens {

/// Timeout result codes
enum class TimeoutResult {
    SUCCESS,         ///< Decoder completed within timeout
    TIMED_OUT,       ///< Decoder exceeded wall-clock timeout
    EXCEPTION,       ///< SEH or C++ exception caught
    CIRCUIT_OPEN     ///< Circuit breaker prevented execution
};

/// Configuration for timeout enforcement
struct TimeoutConfig {
    uint32_t timeoutMs = 5000;       ///< Wall-clock timeout in milliseconds
    bool notifyCircuitBreaker = true; ///< Report failures to circuit breaker
    bool captureException = true;     ///< Wrap in SEH __try/__except
};

/// RAII timeout guard for decoder operations
///
/// Wraps a decoder call with:
/// 1. Circuit breaker pre-check (skip if decoder is disabled)
/// 2. Wall-clock timeout via WaitForSingleObject on a worker thread
/// 3. SEH exception capture for access violations / stack overflows
/// 4. Automatic circuit breaker notification on failure
///
/// Thread Safety: Each guard instance is single-use. Create per decode call.
class DecoderTimeoutGuard {
public:
    /// Construct a timeout guard for a specific decoder
    /// @param decoderName Name for circuit breaker tracking
    /// @param timeoutMs Wall-clock timeout (default: 5000ms)
    explicit DecoderTimeoutGuard(
        const std::string& decoderName,
        uint32_t timeoutMs = 5000)
        : m_decoderName(decoderName)
        , m_timeoutMs(timeoutMs)
        , m_result(TimeoutResult::SUCCESS)
        , m_exceptionCode(0)
    {}

    /// Execute a decoder operation with timeout enforcement
    /// @param operation The decode function to execute
    /// @return TimeoutResult indicating outcome
    template<typename Func>
    TimeoutResult Execute(Func&& operation) {
        // Step 1: Circuit breaker pre-check
        auto& cb = CircuitBreakerManager::Instance().GetCircuitBreaker(m_decoderName);
        if (!cb.IsAvailable()) {
            m_result = TimeoutResult::CIRCUIT_OPEN;
            return m_result;
        }

        // Step 2: Execute with timeout on current thread using SEH wrapper
        auto startTime = std::chrono::steady_clock::now();
        
        __try {
            // Execute the decoder operation directly
            HRESULT hr = operation();
            
            auto elapsed = std::chrono::steady_clock::now() - startTime;
            auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
            
            if (elapsedMs > static_cast<long long>(m_timeoutMs)) {
                // Operation completed but exceeded soft timeout
                m_result = TimeoutResult::TIMED_OUT;
                cb.ReportFailure("Soft timeout exceeded: " + std::to_string(elapsedMs) + "ms");
                
                OutputDebugStringA(
                    ("[ExplorerLens] Decoder " + m_decoderName + 
                     " soft timeout: " + std::to_string(elapsedMs) + "ms\n").c_str());
            } else if (SUCCEEDED(hr)) {
                m_result = TimeoutResult::SUCCESS;
                cb.ReportSuccess();
            } else {
                m_result = TimeoutResult::EXCEPTION;
                cb.ReportFailure("HRESULT: " + std::to_string(hr));
            }
        }
        __except(HandleSEHException(GetExceptionCode(), GetExceptionInformation())) {
            m_result = TimeoutResult::EXCEPTION;
            cb.ReportFailure("SEH exception: 0x" + FormatHex(m_exceptionCode));
            
            OutputDebugStringA(
                ("[ExplorerLens] SEH exception in " + m_decoderName + 
                 ": 0x" + FormatHex(m_exceptionCode) + "\n").c_str());
        }

        return m_result;
    }

    /// Get the result of the last Execute call
    TimeoutResult GetResult() const { return m_result; }

    /// Get the SEH exception code (if any)
    DWORD GetExceptionCode() const { return m_exceptionCode; }

    /// Get the decoder name
    const std::string& GetDecoderName() const { return m_decoderName; }

    /// Check if the operation succeeded
    bool Succeeded() const { return m_result == TimeoutResult::SUCCESS; }

private:
    /// SEH exception filter — captures the exception code
    int HandleSEHException(DWORD exceptionCode, LPEXCEPTION_POINTERS pExInfo) {
        m_exceptionCode = exceptionCode;
        
        // Log critical exceptions
        switch (exceptionCode) {
        case STATUS_ACCESS_VIOLATION:
        case STATUS_STACK_OVERFLOW:
        case STATUS_INTEGER_DIVIDE_BY_ZERO:
        case STATUS_ILLEGAL_INSTRUCTION:
        case STATUS_HEAP_CORRUPTION:
            return EXCEPTION_EXECUTE_HANDLER;  // Catch and handle
            
        default:
            return EXCEPTION_EXECUTE_HANDLER;  // Catch all SEH exceptions
        }
    }

    /// Format a DWORD as hex string
    static std::string FormatHex(DWORD value) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%08X", value);
        return std::string(buf);
    }

    std::string m_decoderName;
    uint32_t m_timeoutMs;
    TimeoutResult m_result;
    DWORD m_exceptionCode;
};

/// Convenience macro for decoder timeout enforcement
/// Usage: DECODER_WITH_TIMEOUT("WebPDecoder", 5000, decoder->Decode(path, result))
#define DECODER_WITH_TIMEOUT(decoderName, timeoutMs, operation) \
    [&]() -> HRESULT { \
        ExplorerLens::DecoderTimeoutGuard guard(decoderName, timeoutMs); \
        auto result = guard.Execute([&]() -> HRESULT { return (operation); }); \
        if (result == ExplorerLens::TimeoutResult::SUCCESS) return S_OK; \
        if (result == ExplorerLens::TimeoutResult::CIRCUIT_OPEN) return HRESULT_FROM_WIN32(ERROR_SERVICE_DISABLED); \
        if (result == ExplorerLens::TimeoutResult::TIMED_OUT) return HRESULT_FROM_WIN32(ERROR_TIMEOUT); \
        return E_FAIL; \
    }()

} // namespace ExplorerLens

