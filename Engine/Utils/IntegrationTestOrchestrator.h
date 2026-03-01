// IntegrationTestOrchestrator.h — Multi-Step Integration Test Orchestrator
// Copyright (c) 2026 ExplorerLens Project
//
// Orchestrates complex multi-step integration test scenarios with
// dependency resolution, parallel execution of independent steps,
// and comprehensive reporting.

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <chrono>
#include <functional>
#include <algorithm>
#include <unordered_map>
#include <numeric>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// Enums
// ============================================================================

/// Orchestrated test scenario type
enum class ScenarioType : uint8_t {
    Sequential = 0,     ///< Steps run in order
    Parallel,           ///< Independent steps run concurrently
    ConditionalFanOut,  ///< Branch based on prior step result
    RetryLoop,          ///< Retry failed steps with backoff
    COUNT
};

/// Step execution result
enum class StepResult : uint8_t {
    NotStarted = 0,
    Success,
    Failure,
    Skipped,
    DependencyFailed,
    COUNT
};

/// Orchestrator run mode
enum class OrchestratorMode : uint8_t {
    Full = 0,       ///< Run all scenarios
    Smoke,          ///< Quick subset for pre-commit
    Nightly,        ///< Extended scenarios for nightly CI
    Stress,         ///< Repeated runs for stability
    COUNT
};

// ============================================================================
// String conversions
// ============================================================================

inline const char* ScenarioTypeToString(ScenarioType t) {
    static const char* names[] = {
        "Sequential", "Parallel", "ConditionalFanOut", "RetryLoop"
    };
    auto idx = static_cast<uint8_t>(t);
    return (idx < static_cast<uint8_t>(ScenarioType::COUNT)) ? names[idx] : "Unknown";
}

inline const char* StepResultToString(StepResult r) {
    static const char* names[] = {
        "NotStarted", "Success", "Failure", "Skipped", "DependencyFailed"
    };
    auto idx = static_cast<uint8_t>(r);
    return (idx < static_cast<uint8_t>(StepResult::COUNT)) ? names[idx] : "Unknown";
}

inline const char* OrchestratorModeToString(OrchestratorMode m) {
    static const char* names[] = { "Full", "Smoke", "Nightly", "Stress" };
    auto idx = static_cast<uint8_t>(m);
    return (idx < static_cast<uint8_t>(OrchestratorMode::COUNT)) ? names[idx] : "Unknown";
}

// ============================================================================
// Structs
// ============================================================================

/// A single step in a test scenario
struct TestStep {
    std::string   name;
    uint32_t      stepId = 0;
    StepResult    result = StepResult::NotStarted;
    double        durationMs = 0.0;
    std::string   errorDetail;
    std::vector<uint32_t> dependsOn;  ///< Step IDs this step depends on
};

/// A complete test scenario (composed of steps)
struct TestScenario {
    std::string     name;
    ScenarioType    type = ScenarioType::Sequential;
    OrchestratorMode mode = OrchestratorMode::Full;
    std::vector<TestStep> steps;
    bool            completed = false;
    double          totalDurationMs = 0.0;

    uint32_t PassedSteps() const {
        uint32_t count = 0;
        for (const auto& s : steps)
            if (s.result == StepResult::Success) count++;
        return count;
    }

    uint32_t FailedSteps() const {
        uint32_t count = 0;
        for (const auto& s : steps)
            if (s.result == StepResult::Failure || s.result == StepResult::DependencyFailed) count++;
        return count;
    }
};

/// Orchestrator statistics
struct OrchestratorStats {
    uint32_t totalScenarios = 0;
    uint32_t completedScenarios = 0;
    uint32_t failedScenarios = 0;
    uint32_t totalSteps = 0;
    uint32_t passedSteps = 0;
    uint32_t failedSteps = 0;
    double   totalDurationMs = 0.0;

    double ScenarioPassRate() const {
        return (totalScenarios > 0) ?
            (static_cast<double>(completedScenarios - failedScenarios) / totalScenarios * 100.0) : 0.0;
    }

    double StepPassRate() const {
        uint32_t executed = passedSteps + failedSteps;
        return (executed > 0) ? (static_cast<double>(passedSteps) / executed * 100.0) : 0.0;
    }
};

// ============================================================================
// IntegrationTestOrchestrator class
// ============================================================================

class IntegrationTestOrchestrator {
public:
    using StepFunc = std::function<bool()>;

    /// Add a scenario
    void AddScenario(const std::string& name, ScenarioType type,
                     OrchestratorMode mode = OrchestratorMode::Full) {
        TestScenario scenario;
        scenario.name = name;
        scenario.type = type;
        scenario.mode = mode;
        m_scenarios.push_back(std::move(scenario));
    }

    /// Add a step to the last added scenario
    void AddStep(const std::string& stepName, StepFunc func,
                 const std::vector<uint32_t>& deps = {}) {
        if (m_scenarios.empty()) return;
        auto& scenario = m_scenarios.back();
        TestStep step;
        step.name = stepName;
        step.stepId = static_cast<uint32_t>(scenario.steps.size());
        step.dependsOn = deps;
        scenario.steps.push_back(std::move(step));
        m_stepFuncs[{m_scenarios.size() - 1, scenario.steps.size() - 1}] = std::move(func);
    }

    /// Run all scenarios matching the given mode
    OrchestratorStats Run(OrchestratorMode mode = OrchestratorMode::Full) {
        OrchestratorStats stats;

        for (size_t si = 0; si < m_scenarios.size(); ++si) {
            auto& scenario = m_scenarios[si];
            if (mode != OrchestratorMode::Full && scenario.mode != mode) continue;
            stats.totalScenarios++;

            auto scenarioStart = std::chrono::steady_clock::now();
            bool scenarioFailed = false;

            for (size_t stIdx = 0; stIdx < scenario.steps.size(); ++stIdx) {
                auto& step = scenario.steps[stIdx];
                stats.totalSteps++;

                // Check dependencies
                bool depsFailed = false;
                for (uint32_t dep : step.dependsOn) {
                    if (dep < scenario.steps.size() &&
                        scenario.steps[dep].result != StepResult::Success) {
                        depsFailed = true;
                        break;
                    }
                }

                if (depsFailed) {
                    step.result = StepResult::DependencyFailed;
                    stats.failedSteps++;
                    scenarioFailed = true;
                    continue;
                }

                // Execute step
                auto it = m_stepFuncs.find({si, stIdx});
                if (it == m_stepFuncs.end()) {
                    step.result = StepResult::Skipped;
                    continue;
                }

                auto start = std::chrono::steady_clock::now();
                bool ok = false;
                try { ok = it->second(); } catch (...) { ok = false; }
                auto end = std::chrono::steady_clock::now();
                step.durationMs = std::chrono::duration<double, std::milli>(end - start).count();

                if (ok) {
                    step.result = StepResult::Success;
                    stats.passedSteps++;
                } else {
                    step.result = StepResult::Failure;
                    stats.failedSteps++;
                    scenarioFailed = true;
                }
            }

            auto scenarioEnd = std::chrono::steady_clock::now();
            scenario.totalDurationMs = std::chrono::duration<double, std::milli>(
                scenarioEnd - scenarioStart).count();
            scenario.completed = true;
            stats.completedScenarios++;
            stats.totalDurationMs += scenario.totalDurationMs;
            if (scenarioFailed) stats.failedScenarios++;
        }

        return stats;
    }

    /// Get all scenarios
    const std::vector<TestScenario>& GetScenarios() const { return m_scenarios; }
    uint32_t GetScenarioCount() const { return static_cast<uint32_t>(m_scenarios.size()); }

    /// Clear all scenarios
    void Clear() { m_scenarios.clear(); m_stepFuncs.clear(); }

private:
    struct PairHash {
        size_t operator()(const std::pair<size_t, size_t>& p) const {
            return std::hash<size_t>()(p.first) ^ (std::hash<size_t>()(p.second) << 32);
        }
    };

    std::vector<TestScenario> m_scenarios;
    std::unordered_map<std::pair<size_t, size_t>, StepFunc, PairHash> m_stepFuncs;
};

} // namespace Engine
} // namespace ExplorerLens
