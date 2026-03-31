// WorkflowAutomationEngine.h — Rule-Based & ML Workflow Automation
// Copyright (c) 2026 ExplorerLens Project
//
// Defines triggers, conditions, and actions for automating thumbnail workflows.
// Users can create rules like "When a new RAW file appears in X folder, generate
// thumbnails at 3 sizes and export to Y". Combines rule-based logic with ML-driven
// condition evaluation for intelligent automation.
//
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace ExplorerLens {
namespace Engine {

enum class WorkflowTrigger : uint8_t {
    FileCreated = 0,
    FileModified,
    FolderOpened,
    ScheduledTime,
    SystemIdle,
    ManualInvoke,
    BatchComplete
};

enum class WorkflowCondition : uint8_t {
    Always = 0,
    FileTypeMatch,
    FileSizeAbove,
    FileSizeBelow,
    FolderContainsMoreThan,
    TimeWindow,
    SystemLoadBelow
};

enum class WorkflowAction : uint8_t {
    GenerateThumbnail = 0,
    GenerateMultiSize,
    Categorize,
    PregenFolder,
    ExportBatch,
    NotifyUser,
    UpdateIndex
};

struct WorkflowRule {
    uint32_t id = 0;
    std::wstring name;
    WorkflowTrigger trigger = WorkflowTrigger::FileCreated;
    WorkflowCondition condition = WorkflowCondition::Always;
    WorkflowAction action = WorkflowAction::GenerateThumbnail;
    bool enabled = true;
    uint64_t executionCount = 0;
};

struct WorkflowStats {
    uint64_t totalRules = 0;
    uint64_t enabledRules = 0;
    uint64_t totalExecutions = 0;
    uint64_t failedExecutions = 0;
};

class WorkflowAutomationEngine {
public:
    static WorkflowAutomationEngine& Instance() {
        static WorkflowAutomationEngine instance;
        return instance;
    }

    bool Initialize() {
        m_initialized = true;
        return true;
    }

    uint32_t AddRule(const WorkflowRule& rule) {
        if (!m_initialized) return 0;
        WorkflowRule r = rule;
        r.id = m_nextId++;
        m_rules.push_back(r);
        m_stats.totalRules++;
        m_stats.enabledRules += r.enabled ? 1 : 0;
        return r.id;
    }

    bool RemoveRule(uint32_t ruleId) {
        for (auto it = m_rules.begin(); it != m_rules.end(); ++it) {
            if (it->id == ruleId) {
                if (it->enabled) m_stats.enabledRules--;
                m_stats.totalRules--;
                m_rules.erase(it);
                return true;
            }
        }
        return false;
    }

    bool EnableRule(uint32_t ruleId, bool enable) {
        for (auto& r : m_rules) {
            if (r.id == ruleId) {
                if (r.enabled != enable) {
                    m_stats.enabledRules += enable ? 1 : -1;
                }
                r.enabled = enable;
                return true;
            }
        }
        return false;
    }

    size_t GetRuleCount() const { return m_rules.size(); }
    WorkflowStats GetStats() const { return m_stats; }
    bool IsInitialized() const { return m_initialized; }

    void Shutdown() {
        m_rules.clear();
        m_initialized = false;
    }

private:
    WorkflowAutomationEngine() = default;
    bool m_initialized = false;
    uint32_t m_nextId = 1;
    std::vector<WorkflowRule> m_rules;
    WorkflowStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
