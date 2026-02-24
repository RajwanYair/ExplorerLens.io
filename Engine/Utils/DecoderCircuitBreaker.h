// ============================================================================
// DecoderCircuitBreaker.h
// Circuit breaker to disable failing decoders
// Prevents repeated failures from impacting system performance
// ============================================================================

#pragma once

#include <atomic>
#include <chrono>
#include <string>
#include <unordered_map>
#include <mutex>

namespace ExplorerLens {

// ============================================================================
// Circuit Breaker States
// ============================================================================
enum class CircuitState {
    CLOSED,      // Normal operation
    OPEN,        // Failing - decoder disabled
    HALF_OPEN    // Testing if decoder recovered
};

// ============================================================================
// Circuit Breaker for a Single Decoder
// ============================================================================
class DecoderCircuitBreaker {
public:
    explicit DecoderCircuitBreaker(const std::string& decoderName) 
        : m_decoderName(decoderName)
        , m_state(CircuitState::CLOSED)
        , m_failureCount(0)
        , m_successCount(0)
        , m_lastFailureTime(std::chrono::steady_clock::now())
        , m_failureThreshold(5)              // Open circuit after 5 consecutive failures
        , m_recoveryTimeout(std::chrono::minutes(5))  // Try recovery after 5 minutes
        , m_recoverySuccessThreshold(3)      // Need 3 successes to close circuit
    {}
    
    // Check if decoder is available for use
    bool IsAvailable() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        switch (m_state) {
        case CircuitState::CLOSED:
            return true;
            
        case CircuitState::OPEN:
            // Check if recovery timeout elapsed
            if (ShouldAttemptRecovery()) {
                m_state = CircuitState::HALF_OPEN;
                m_successCount = 0;
                return true;  // Allow one test attempt
            }
            return false;  // Still disabled
            
        case CircuitState::HALF_OPEN:
            return true;  // Allow testing
            
        default:
            return true;
        }
    }
    
    // Report successful decode
    void ReportSuccess() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        switch (m_state) {
        case CircuitState::CLOSED:
            m_failureCount = 0;  // Reset failure counter
            break;
            
        case CircuitState::HALF_OPEN:
            m_successCount++;
            if (m_successCount >= m_recoverySuccessThreshold) {
                // Decoder recovered!
                m_state = CircuitState::CLOSED;
                m_failureCount = 0;
                m_successCount = 0;
                
                OutputDebugStringA(
                    ("[ExplorerLens] Circuit breaker CLOSED for decoder: " + 
                     m_decoderName + " (recovered)\n").c_str());
            }
            break;
            
        case CircuitState::OPEN:
            // Should not happen, but reset just in case
            m_failureCount = 0;
            break;
        }
    }
    
    // Report failed decode
    void ReportFailure(const std::string& reason = "") {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        m_lastFailureTime = std::chrono::steady_clock::now();
        
        switch (m_state) {
        case CircuitState::CLOSED:
            m_failureCount++;
            if (m_failureCount >= m_failureThreshold) {
                // Too many failures - open circuit
                m_state = CircuitState::OPEN;
                
                std::string msg = "[ExplorerLens] Circuit breaker OPENED for decoder: " + 
                                  m_decoderName + 
                                  " (threshold: " + std::to_string(m_failureThreshold) + 
                                  " failures)";
                if (!reason.empty()) {
                    msg += " - Last error: " + reason;
                }
                msg += "\n";
                
                OutputDebugStringA(msg.c_str());
            }
            break;
            
        case CircuitState::HALF_OPEN:
            // Failed during recovery test - reopen circuit
            m_state = CircuitState::OPEN;
            m_successCount = 0;
            m_failureCount++;
            
            OutputDebugStringA(
                ("[ExplorerLens] Circuit breaker reopened for decoder: " + 
                 m_decoderName + " (recovery test failed)\n").c_str());
            break;
            
        case CircuitState::OPEN:
            m_failureCount++;
            break;
        }
    }
    
    // Get current state
    CircuitState GetState() const { return m_state; }
    
    // Get failure count
    int GetFailureCount() const { return m_failureCount; }
    
    // Manually reset circuit breaker
    void Reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_state = CircuitState::CLOSED;
        m_failureCount = 0;
        m_successCount = 0;
        
        OutputDebugStringA(
            ("[ExplorerLens] Circuit breaker RESET for decoder: " + 
             m_decoderName + "\n").c_str());
    }
    
private:
    bool ShouldAttemptRecovery() const {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(
            now - m_lastFailureTime);
        return elapsed >= m_recoveryTimeout;
    }
    
    std::string m_decoderName;
    CircuitState m_state;
    int m_failureCount;
    int m_successCount;
    std::chrono::steady_clock::time_point m_lastFailureTime;
    
    // Thresholds (configurable)
    int m_failureThreshold;
    std::chrono::minutes m_recoveryTimeout;
    int m_recoverySuccessThreshold;
    
    mutable std::mutex m_mutex;
};

// ============================================================================
// Global Circuit Breaker Manager
// ============================================================================
class CircuitBreakerManager {
public:
    static CircuitBreakerManager& Instance() {
        static CircuitBreakerManager instance;
        return instance;
    }
    
    // Get or create circuit breaker for a decoder
    DecoderCircuitBreaker& GetCircuitBreaker(const std::string& decoderName) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_breakers.find(decoderName);
        if (it == m_breakers.end()) {
            it = m_breakers.emplace(
                decoderName, 
                std::make_unique<DecoderCircuitBreaker>(decoderName)
            ).first;
        }
        
        return *it->second;
    }
    
    // Reset all circuit breakers
    void ResetAll() {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& pair : m_breakers) {
            pair.second->Reset();
        }
    }
    
    // Get statistics
    struct Stats {
        int totalBreakers = 0;
        int closedBreakers = 0;
        int openBreakers = 0;
        int halfOpenBreakers = 0;
    };
    
    Stats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        Stats stats;
        stats.totalBreakers = static_cast<int>(m_breakers.size());
        
        for (const auto& pair : m_breakers) {
            switch (pair.second->GetState()) {
            case CircuitState::CLOSED:
                stats.closedBreakers++;
                break;
            case CircuitState::OPEN:
                stats.openBreakers++;
                break;
            case CircuitState::HALF_OPEN:
                stats.halfOpenBreakers++;
                break;
            }
        }
        
        return stats;
    }
    
private:
    CircuitBreakerManager() = default;
    ~CircuitBreakerManager() = default;
    
    // No copy/move
    CircuitBreakerManager(const CircuitBreakerManager&) = delete;
    CircuitBreakerManager& operator=(const CircuitBreakerManager&) = delete;
    
    std::unordered_map<std::string, std::unique_ptr<DecoderCircuitBreaker>> m_breakers;
    mutable std::mutex m_mutex;
};

// ============================================================================
// Convenience Macros
// ============================================================================

#define DECODER_CIRCUIT_CHECK(decoderName) \
    auto& __circuitBreaker = ExplorerLens::CircuitBreakerManager::Instance().GetCircuitBreaker(decoderName); \
    if (!__circuitBreaker.IsAvailable()) { \
        return E_FAIL; /* Decoder disabled by circuit breaker */ \
    }

#define DECODER_CIRCUIT_SUCCESS(decoderName) \
    ExplorerLens::CircuitBreakerManager::Instance().GetCircuitBreaker(decoderName).ReportSuccess()

#define DECODER_CIRCUIT_FAILURE(decoderName, reason) \
    ExplorerLens::CircuitBreakerManager::Instance().GetCircuitBreaker(decoderName).ReportFailure(reason)

} // namespace ExplorerLens

