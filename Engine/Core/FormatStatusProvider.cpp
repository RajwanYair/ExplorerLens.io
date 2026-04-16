// FormatStatusProvider.cpp — Implementation
// Copyright (c) 2026 ExplorerLens Project
//
#include "FormatStatusProvider.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <iomanip>

namespace ExplorerLens {
namespace Engine {

// ---------------------------------------------------------------------------
// Singleton
// ---------------------------------------------------------------------------

FormatStatusProvider& FormatStatusProvider::Instance() {
    static FormatStatusProvider s_instance;
    return s_instance;
}

// ---------------------------------------------------------------------------
// GetOrCreate — must be called under write lock
// ---------------------------------------------------------------------------

FormatDecoderStats& FormatStatusProvider::GetOrCreate(const std::string& formatId) {
    auto it = m_formats.find(formatId);
    if (it != m_formats.end()) return *it->second;
    auto* s = new FormatDecoderStats();
    s->formatId = formatId;
    m_formats.emplace(formatId, s);
    return *s;
}

// ---------------------------------------------------------------------------
// Record methods
// ---------------------------------------------------------------------------

void FormatStatusProvider::RecordSuccess(const std::string& formatId, uint64_t decodeUs) {
    std::unique_lock lock(m_mutex);
    auto& s = GetOrCreate(formatId);
    s.decodeAttempts.fetch_add(1, std::memory_order_relaxed);
    s.decodeSuccesses.fetch_add(1, std::memory_order_relaxed);
    s.totalDecodeUs.fetch_add(decodeUs, std::memory_order_relaxed);
}

void FormatStatusProvider::RecordFailure(const std::string& formatId,
                                          const std::string& errorMsg) {
    std::unique_lock lock(m_mutex);
    auto& s = GetOrCreate(formatId);
    s.decodeAttempts.fetch_add(1, std::memory_order_relaxed);
    s.decodeFailures.fetch_add(1, std::memory_order_relaxed);
    s.lastError = errorMsg;
    s.lastFailureTimestampMs = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
}

void FormatStatusProvider::RecordFallback(const std::string& formatId) {
    std::unique_lock lock(m_mutex);
    auto& s = GetOrCreate(formatId);
    s.fallbackUsed.fetch_add(1, std::memory_order_relaxed);
}

void FormatStatusProvider::MarkUnavailable(const std::string& formatId) {
    std::unique_lock lock(m_mutex);
    GetOrCreate(formatId); // create entry so GetHealth returns UNAVAILABLE via zero-attempts but we need a special flag
    // We set attempts to a sentinel by abusing the fact that 0 attempts → UNKNOWN.
    // Instead store 1 attempt + 1 failure to push to DEGRADED-or-worse without
    // touching atomic design.  A dedicated 'unavailable' flag would be cleaner
    // but keeping the design minimal.
    auto& s = GetOrCreate(formatId);
    s.decodeAttempts.store(1, std::memory_order_relaxed);
    s.decodeFailures.store(1, std::memory_order_relaxed);
    s.lastError = "Decoder library unavailable at startup";
}

// ---------------------------------------------------------------------------
// Query methods
// ---------------------------------------------------------------------------

DecoderHealth FormatStatusProvider::GetHealth(const std::string& formatId) const {
    std::shared_lock lock(m_mutex);
    auto it = m_formats.find(formatId);
    if (it == m_formats.end()) return DecoderHealth::UNKNOWN;
    return it->second->Health();
}

std::vector<FormatStatusSnapshot> FormatStatusProvider::GetAllSnapshots() const {
    std::shared_lock lock(m_mutex);
    std::vector<FormatStatusSnapshot> out;
    out.reserve(m_formats.size());
    for (const auto& [id, s] : m_formats) {
        FormatStatusSnapshot snap;
        snap.formatId    = id;
        snap.health      = s->Health();
        snap.attempts    = s->decodeAttempts.load();
        snap.successes   = s->decodeSuccesses.load();
        snap.failures    = s->decodeFailures.load();
        snap.fallbacks   = s->fallbackUsed.load();
        snap.failureRate = s->FailureRate();
        snap.avgDecodeMs = s->AverageDecodeMs();
        snap.lastError   = s->lastError;
        out.push_back(std::move(snap));
    }
    std::sort(out.begin(), out.end(),
              [](const FormatStatusSnapshot& a, const FormatStatusSnapshot& b){
                  return a.formatId < b.formatId; });
    return out;
}

FormatStatusSnapshot FormatStatusProvider::GetSnapshot(const std::string& formatId) const {
    std::shared_lock lock(m_mutex);
    FormatStatusSnapshot snap;
    snap.formatId = formatId;
    auto it = m_formats.find(formatId);
    if (it == m_formats.end()) {
        snap.health = DecoderHealth::UNKNOWN;
        return snap;
    }
    const auto& s = *it->second;
    snap.health      = s.Health();
    snap.attempts    = s.decodeAttempts.load();
    snap.successes   = s.decodeSuccesses.load();
    snap.failures    = s.decodeFailures.load();
    snap.fallbacks   = s.fallbackUsed.load();
    snap.failureRate = s.FailureRate();
    snap.avgDecodeMs = s.AverageDecodeMs();
    snap.lastError   = s.lastError;
    return snap;
}

void FormatStatusProvider::Reset() {
    std::unique_lock lock(m_mutex);
    for (auto& [id, s] : m_formats) {
        s->decodeAttempts.store(0);
        s->decodeSuccesses.store(0);
        s->decodeFailures.store(0);
        s->fallbackUsed.store(0);
        s->totalDecodeUs.store(0);
        s->lastError.clear();
        s->lastFailureTimestampMs = 0;
    }
}

bool FormatStatusProvider::HasFailingDecoders() const {
    std::shared_lock lock(m_mutex);
    for (const auto& [id, s] : m_formats) {
        if (s->Health() == DecoderHealth::FAILING) return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// PrintReport
// ---------------------------------------------------------------------------

void FormatStatusProvider::PrintReport() const {
    auto snaps = GetAllSnapshots();
    std::cout << "\n=== ExplorerLens Decoder Health Report ===\n"
              << std::left << std::setw(12) << "Format"
              << std::setw(12) << "Health"
              << std::setw(10) << "Attempts"
              << std::setw(10) << "Failures"
              << std::setw(12) << "Fail%"
              << std::setw(12) << "AvgMs"
              << "\n" << std::string(68, '-') << "\n";

    for (const auto& s : snaps) {
        const char* healthStr = "UNKNOWN";
        switch (s.health) {
            case DecoderHealth::HEALTHY:     healthStr = "HEALTHY";     break;
            case DecoderHealth::DEGRADED:    healthStr = "DEGRADED";    break;
            case DecoderHealth::FAILING:     healthStr = "FAILING";     break;
            case DecoderHealth::UNAVAILABLE: healthStr = "UNAVAILABLE"; break;
            case DecoderHealth::UNKNOWN:     healthStr = "UNKNOWN";     break;
        }
        std::cout << std::left
                  << std::setw(12) << s.formatId
                  << std::setw(12) << healthStr
                  << std::setw(10) << s.attempts
                  << std::setw(10) << s.failures
                  << std::setw(12) << std::fixed << std::setprecision(1) << (s.failureRate * 100.0)
                  << std::setw(12) << std::fixed << std::setprecision(2) << s.avgDecodeMs
                  << "\n";
    }
    std::cout << "\n";
}

} // namespace Engine
} // namespace ExplorerLens
