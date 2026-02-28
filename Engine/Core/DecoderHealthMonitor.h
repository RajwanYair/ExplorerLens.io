//==============================================================================
// ExplorerLens Engine - Decoder Health Monitor
// Copyright (c) 2026 - ExplorerLens Project
// Task A22: Decoder availability and health tracking system
//==============================================================================

#pragma once

#include <string>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <chrono>

namespace ExplorerLens {
namespace Engine {

 /// <summary>
 /// Tracks decoder health, success rates, and availability
 /// </summary>
 class DecoderHealthMonitor {
 public:
 struct DecoderStats {
 std::atomic<uint64_t> successCount{0};
 std::atomic<uint64_t> failureCount{0};
 std::atomic<uint64_t> timeoutCount{0};
 std::chrono::steady_clock::time_point lastSuccess;
 std::chrono::steady_clock::time_point lastFailure;
 bool isAvailable{true};
 
 double GetSuccessRate() const {
 uint64_t total = successCount + failureCount;
 return total > 0 ? static_cast<double>(successCount) / total : 0.0;
 }
 
 bool IsHealthy() const {
 return isAvailable && GetSuccessRate() >= 0.8;
 }
 };
 
 static DecoderHealthMonitor& GetInstance() {
 static DecoderHealthMonitor instance;
 return instance;
 }
 
 void RecordSuccess(const std::wstring& decoderName) {
 std::lock_guard<std::mutex> lock(m_mutex);
 auto& stats = m_stats[decoderName];
 stats.successCount++;
 stats.lastSuccess = std::chrono::steady_clock::now();
 stats.isAvailable = true;
 }
 
 void RecordFailure(const std::wstring& decoderName) {
 std::lock_guard<std::mutex> lock(m_mutex);
 auto& stats = m_stats[decoderName];
 stats.failureCount++;
 stats.lastFailure = std::chrono::steady_clock::now();
 
 // Mark as unavailable if failure rate exceeds 90%
 if (stats.GetSuccessRate() < 0.1 && stats.failureCount > 10) {
 stats.isAvailable = false;
 }
 }
 
 void RecordTimeout(const std::wstring& decoderName) {
 std::lock_guard<std::mutex> lock(m_mutex);
 auto& stats = m_stats[decoderName];
 stats.timeoutCount++;
 stats.failureCount++; // Timeout counts as failure
 stats.lastFailure = std::chrono::steady_clock::now();
 }
 
 bool IsDecoderAvailable(const std::wstring& decoderName) const {
 std::lock_guard<std::mutex> lock(m_mutex);
 auto it = m_stats.find(decoderName);
 if (it == m_stats.end()) return true; // Assume available if no history
 return it->second.isAvailable;
 }
 
 DecoderStats GetStats(const std::wstring& decoderName) const {
 std::lock_guard<std::mutex> lock(m_mutex);
 auto it = m_stats.find(decoderName);
 if (it != m_stats.end()) {
 return it->second;
 }
 return DecoderStats{};
 }
 
 void Reset(const std::wstring& decoderName) {
 std::lock_guard<std::mutex> lock(m_mutex);
 m_stats.erase(decoderName);
 }
 
 void ResetAll() {
 std::lock_guard<std::mutex> lock(m_mutex);
 m_stats.clear();
 }
 
 private:
 DecoderHealthMonitor() = default;
 ~DecoderHealthMonitor() = default;
 DecoderHealthMonitor(const DecoderHealthMonitor&) = delete;
 DecoderHealthMonitor& operator=(const DecoderHealthMonitor&) = delete;
 
 mutable std::mutex m_mutex;
 std::unordered_map<std::wstring, DecoderStats> m_stats;
 };

} // namespace Engine
} // namespace ExplorerLens

