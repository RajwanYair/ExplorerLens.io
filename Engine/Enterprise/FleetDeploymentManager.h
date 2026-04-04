// FleetDeploymentManager.h — Fleet Deployment via MDM/WinGet/SCCM
// Copyright (c) 2026 ExplorerLens Project
//
// Manages staged fleet deployments through MDM, WinGet, SCCM, Ansible, and Manual
// channels with canary/ring rollout support and real-time job tracking.
//
#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace ExplorerLens::Engine {

enum class FleetDeployMethod : uint8_t {
    WinGet = 0,
    MDM_Intune = 1,
    SCCM = 2,
    Ansible = 3,
    Manual = 4
};

enum class RolloutStage : uint8_t {
    Canary = 0,
    Ring1 = 1,
    Ring2 = 2,
    Full = 3
};

enum class JobState : uint8_t {
    Pending = 0,
    Running = 1,
    Completed = 2,
    Failed = 3,
    Cancelled = 4
};

struct DeploymentJob
{
    std::string jobId;
    FleetDeployMethod method{FleetDeployMethod::WinGet};
    std::string version;
    RolloutStage stage{RolloutStage::Canary};
    uint32_t targetCount{0};
    uint32_t completedCount{0};
    uint32_t failedCount{0};
    JobState state{JobState::Pending};
    std::chrono::system_clock::time_point scheduledAt;
};

struct InventoryEntry
{
    std::string endpointId;
    std::string installedVersion;
    bool managedByFleet{false};
    std::chrono::system_clock::time_point lastSeen;
};

class FleetDeploymentManager
{
  public:
    FleetDeploymentManager() = default;
    ~FleetDeploymentManager() = default;

    FleetDeploymentManager(const FleetDeploymentManager&) = delete;
    FleetDeploymentManager& operator=(const FleetDeploymentManager&) = delete;

    // Job scheduling
    std::string ScheduleDeployment(const std::string& version, FleetDeployMethod method,
                                   RolloutStage initialStage = RolloutStage::Canary);
    bool AdvanceStage(const std::string& jobId);
    bool CancelJob(const std::string& jobId);

    // Job status
    std::optional<DeploymentJob> GetJobStatus(const std::string& jobId) const;
    std::vector<DeploymentJob> ListJobs() const;
    std::vector<DeploymentJob> GetActiveJobs() const;

    // Fleet inventory
    std::vector<InventoryEntry> GetFleetInventory() const;
    uint32_t GetManagedEndpointCount() const noexcept;
    bool RefreshInventory();

    // Method availability check
    bool IsMethodAvailable(FleetDeployMethod method) const;

    // Progress callback
    using ProgressCallback = std::function<void(const DeploymentJob&)>;
    void SetProgressCallback(ProgressCallback cb);

  private:
    std::vector<DeploymentJob> m_jobs;
    std::vector<InventoryEntry> m_inventory;
    ProgressCallback m_progressCallback;

    std::string GenerateJobId() const;
    bool ExecuteDeployStep(DeploymentJob& job);
    void UpdateJobState(const std::string& jobId, JobState state);
    RolloutStage AdvancedStage(RolloutStage current) const noexcept;
};

}  // namespace ExplorerLens::Engine
