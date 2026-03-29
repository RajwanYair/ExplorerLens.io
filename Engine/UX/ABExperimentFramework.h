// ABExperimentFramework.h — A/B Experiment Framework
// Copyright (c) 2026 ExplorerLens Project
//
// Lightweight A/B experiment assignment, cohort management, and conversion
// tracking. Uses deterministic hashing for consistent user-cohort assignment.
//
#pragma once
#include <string>
#include <unordered_map>
#include <cstdint>
#include <functional>

namespace ExplorerLens { namespace Engine {

enum class Cohort { Control = 0, VariantA = 1, VariantB = 2 };

struct ExperimentStats {
    uint64_t controlUsers    = 0;
    uint64_t variantAUsers   = 0;
    uint64_t variantBUsers   = 0;
    uint64_t conversions[3]  = {0, 0, 0};
    float    conversionRate(Cohort c) const {
        uint64_t users = (c == Cohort::Control) ? controlUsers
                       : (c == Cohort::VariantA) ? variantAUsers : variantBUsers;
        uint64_t cvt = conversions[static_cast<int>(c)];
        return users > 0 ? static_cast<float>(cvt) / static_cast<float>(users) : 0.0f;
    }
};

class ABExperimentFramework {
public:
    ABExperimentFramework() = default;

    bool Initialize(const std::string& experimentId, uint32_t seed = 42) {
        m_experimentId = experimentId;
        m_seed         = seed;
        m_ready        = true;
        return true;
    }
    bool IsReady() const { return m_ready; }

    Cohort AssignCohort(const std::string& userId) {
        uint64_t h = std::hash<std::string>{}(m_experimentId + userId + std::to_string(m_seed));
        Cohort c = static_cast<Cohort>(h % 3);
        auto& cnt = m_stats;
        if (c == Cohort::Control)  cnt.controlUsers++;
        else if (c == Cohort::VariantA) cnt.variantAUsers++;
        else                           cnt.variantBUsers++;
        return c;
    }

    void RecordConversion(const std::string& userId) {
        Cohort c = AssignCohort(userId);
        m_stats.conversions[static_cast<int>(c)]++;
    }

    ExperimentStats GetStats() const { return m_stats; }

    void Reset() {
        m_stats = {};
    }

    void Shutdown() { m_ready = false; }

private:
    bool           m_ready = false;
    std::string    m_experimentId;
    uint32_t       m_seed = 42;
    ExperimentStats m_stats;
};

}} // namespace ExplorerLens::Engine
