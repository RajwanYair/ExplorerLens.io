// PluginABTestFramework.h — Plugin A/B Test Framework
// Copyright (c) 2026 ExplorerLens Project
//
// Assigns users to cohorts A/B/control and tracks feature exposure metrics for plugin feature experiments.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

enum class TestCohort { Control, VariantA, VariantB };
struct ABExperiment { std::string name; double controlWeight; double variantAWeight; double variantBWeight; };
class PluginABTestFramework {
public:
    void      RegisterExperiment(ABExperiment exp) { m_experiments[exp.name] = exp; }
    TestCohort AssignCohort(const std::string& expName, uint64_t userId) const {
        (void)expName; return userId % 3 == 0 ? TestCohort::VariantA : userId % 3 == 1 ? TestCohort::VariantB : TestCohort::Control;
    }
    bool      IsExperimentActive(const std::string& name) const { return m_experiments.count(name) > 0; }
    size_t    ExperimentCount() const { return m_experiments.size(); }
private:
    std::unordered_map<std::string, ABExperiment> m_experiments;
};

} // namespace Engine
} // namespace ExplorerLens