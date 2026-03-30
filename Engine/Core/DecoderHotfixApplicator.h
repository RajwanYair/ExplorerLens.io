// DecoderHotfixApplicator.h — Live Decoder Hotfix Applicator
// Copyright (c) 2026 ExplorerLens Project
//
// Applies runtime hotfixes to decoders without requiring full rebuilds. Supports
// rollback, manifest validation, and priority-based application ordering.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>

namespace ExplorerLens { namespace Engine {

enum class HotfixPriority : uint8_t {
    Low = 0, Normal = 1, High = 2, Critical = 3, Emergency = 4
};

struct HotfixEntry {
    uint32_t id = 0;
    std::string targetDecoder;
    HotfixPriority priority = HotfixPriority::Normal;
    std::string description;
    std::array<uint8_t, 32> patchHash = {};
    bool applied = false;
    uint64_t appliedTimestampMs = 0;
    std::string rollbackData;

    bool IsHighPriority() const {
        return priority >= HotfixPriority::Critical;
    }
};

struct HotfixManifest {
    std::string version;
    std::vector<HotfixEntry> entries;
    std::array<uint8_t, 64> signature = {};
    uint64_t createdTimestampMs = 0;
    std::string issuer;

    size_t GetPendingCount() const {
        size_t count = 0;
        for (const auto& e : entries)
            if (!e.applied) ++count;
        return count;
    }
};

using HotfixCallback = std::function<bool(const HotfixEntry& entry)>;

class DecoderHotfixApplicator {
public:
    DecoderHotfixApplicator()
        : m_maxRetries(3), m_dryRunMode(false), m_appliedCount(0) {}

    ~DecoderHotfixApplicator() = default;

    bool ApplyHotfix(HotfixEntry& entry) {
        if (entry.applied) return false;
        if (m_dryRunMode) return true;
        if (m_preApplyCallback && !m_preApplyCallback(entry)) return false;
        entry.applied = true;
        ++m_appliedCount;
        m_appliedIds.push_back(entry.id);
        m_rollbackStack.push_back(entry);
        return true;
    }

    bool RollbackHotfix(uint32_t hotfixId) {
        for (auto it = m_rollbackStack.rbegin(); it != m_rollbackStack.rend(); ++it) {
            if (it->id == hotfixId) {
                it->applied = false;
                if (m_appliedCount > 0) --m_appliedCount;
                m_appliedIds.erase(
                    std::remove(m_appliedIds.begin(), m_appliedIds.end(), hotfixId),
                    m_appliedIds.end());
                return true;
            }
        }
        return false;
    }

    size_t ApplyManifest(HotfixManifest& manifest) {
        auto sorted = manifest.entries;
        std::sort(sorted.begin(), sorted.end(),
            [](const HotfixEntry& a, const HotfixEntry& b) {
                return static_cast<uint8_t>(a.priority) > static_cast<uint8_t>(b.priority);
            });
        size_t applied = 0;
        for (auto& entry : sorted) {
            if (ApplyHotfix(entry)) ++applied;
        }
        manifest.entries = std::move(sorted);
        return applied;
    }

    std::vector<HotfixEntry> ListApplied() const { return m_rollbackStack; }

    bool ValidateManifest(const HotfixManifest& manifest) const {
        if (manifest.version.empty()) return false;
        if (manifest.entries.empty()) return false;
        for (const auto& e : manifest.entries)
            if (e.targetDecoder.empty() || e.id == 0) return false;
        return true;
    }

    size_t GetPendingCount(const HotfixManifest& manifest) const {
        return manifest.GetPendingCount();
    }

    void SetDryRun(bool dryRun) { m_dryRunMode = dryRun; }
    void SetPreApplyCallback(HotfixCallback cb) { m_preApplyCallback = std::move(cb); }
    void SetMaxRetries(uint32_t retries) { m_maxRetries = retries; }
    size_t GetAppliedCount() const { return m_appliedCount; }

private:
    uint32_t m_maxRetries;
    bool m_dryRunMode;
    size_t m_appliedCount;
    std::vector<uint32_t> m_appliedIds;
    std::vector<HotfixEntry> m_rollbackStack;
    HotfixCallback m_preApplyCallback;
};

}} // namespace ExplorerLens::Engine
