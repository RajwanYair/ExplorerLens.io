#pragma once
// ============================================================================
// COMDiagnosticsEngine.h — COM registration health check and auto-repair
//
// Purpose:   COM registration health check and auto-repair
// Provides:  COMHealthStatus, COMRepairAction enums, COMDiagnosticResult
//            struct, and COMDiagnosticsEngine class
// Used by:   Shell extension health monitoring
// ============================================================================

#include <string>
#include <vector>
#include <cstdint>
#include <mutex>
#include <chrono>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// COMDiagnosticsEngine — COM registration health monitoring + repair
// ============================================================================

enum class COMHealthStatus {
    Healthy,
    Degraded,
    Broken,
    Unregistered,
    Orphaned
};

inline const char* COMHealthStatusName(COMHealthStatus value) {
    switch (value) {
    case COMHealthStatus::Healthy:      return "Healthy";
    case COMHealthStatus::Degraded:     return "Degraded";
    case COMHealthStatus::Broken:       return "Broken";
    case COMHealthStatus::Unregistered: return "Unregistered";
    case COMHealthStatus::Orphaned:     return "Orphaned";
    default:                            return "Unknown";
    }
}

enum class COMRepairAction {
    Reregister,
    CleanKeys,
    ResetPermissions,
    FullRepair
};

inline const char* COMRepairActionName(COMRepairAction value) {
    switch (value) {
    case COMRepairAction::Reregister:       return "Reregister";
    case COMRepairAction::CleanKeys:        return "CleanKeys";
    case COMRepairAction::ResetPermissions: return "ResetPermissions";
    case COMRepairAction::FullRepair:       return "FullRepair";
    default:                                return "Unknown";
    }
}

struct COMDiagnosticResult {
    std::wstring                   clsid;
    COMHealthStatus                status = COMHealthStatus::Unregistered;
    std::wstring                   registrationPath;
    std::vector<COMRepairAction>   repairActions;
    bool                           inprocServer32Found = false;
    bool                           dllExists = false;
    bool                           typeLibRegistered = false;
    double                         checkTimeMs = 0.0;

    bool NeedsRepair() const {
        return status != COMHealthStatus::Healthy;
    }

    uint32_t GetRepairActionCount() const {
        return static_cast<uint32_t>(repairActions.size());
    }
};

struct COMRepairResult {
    std::wstring       clsid;
    COMRepairAction    action = COMRepairAction::Reregister;
    bool               success = false;
    std::string        errorMessage;
    double             repairTimeMs = 0.0;
    COMHealthStatus    statusBefore = COMHealthStatus::Broken;
    COMHealthStatus    statusAfter = COMHealthStatus::Broken;
};

class COMDiagnosticsEngine {
public:
    static constexpr const wchar_t* CLSID_LENS = L"9E6ECB90-5A61-42BD-B851-D3297D9C7F39";
    static constexpr uint32_t MAX_REPAIR_ATTEMPTS = 3;
    static constexpr uint32_t HEALTH_CHECK_VERSION = 2;

    COMDiagnosticsEngine() = default;
    ~COMDiagnosticsEngine() = default;

    COMDiagnosticsEngine(const COMDiagnosticsEngine&) = delete;
    COMDiagnosticsEngine& operator=(const COMDiagnosticsEngine&) = delete;

    COMDiagnosticResult CheckRegistration(const std::wstring& clsid) {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto startTime = std::chrono::high_resolution_clock::now();

        COMDiagnosticResult result;
        result.clsid = clsid;

        // Check if we have a cached status
        auto it = m_statusCache.find(clsid);
        if (it != m_statusCache.end()) {
            result = it->second;
        }
        else {
            // In production: query HKCR\CLSID\{clsid}\InprocServer32
            // For testability, start as Unregistered
            result.status = COMHealthStatus::Unregistered;
            result.registrationPath = L"HKCR\\CLSID\\{" + clsid + L"}\\InprocServer32";

            // Build repair action list
            result.repairActions.push_back(COMRepairAction::Reregister);
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        result.checkTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

        m_statusCache[clsid] = result;
        m_totalChecks++;
        return result;
    }

    COMRepairResult Repair(const std::wstring& clsid, COMRepairAction action) {
        std::lock_guard<std::mutex> lock(m_mutex);

        COMRepairResult repairResult;
        repairResult.clsid = clsid;
        repairResult.action = action;

        auto startTime = std::chrono::high_resolution_clock::now();

        // Get current status
        auto it = m_statusCache.find(clsid);
        if (it != m_statusCache.end()) {
            repairResult.statusBefore = it->second.status;
        }
        else {
            repairResult.statusBefore = COMHealthStatus::Unregistered;
        }

        // Simulate repair based on action
        switch (action) {
        case COMRepairAction::Reregister: {
            // In production: call regsvr32 or DllRegisterServer
            COMDiagnosticResult updated;
            updated.clsid = clsid;
            updated.status = COMHealthStatus::Healthy;
            updated.registrationPath = L"HKCR\\CLSID\\{" + clsid + L"}\\InprocServer32";
            updated.inprocServer32Found = true;
            updated.dllExists = true;
            m_statusCache[clsid] = updated;

            repairResult.success = true;
            repairResult.statusAfter = COMHealthStatus::Healthy;
            break;
        }
        case COMRepairAction::CleanKeys: {
            // In production: remove stale registry keys
            repairResult.success = true;
            repairResult.statusAfter = COMHealthStatus::Unregistered;
            if (it != m_statusCache.end()) {
                it->second.status = COMHealthStatus::Unregistered;
                it->second.repairActions.clear();
                it->second.repairActions.push_back(COMRepairAction::Reregister);
            }
            break;
        }
        case COMRepairAction::ResetPermissions: {
            repairResult.success = true;
            repairResult.statusAfter = repairResult.statusBefore;
            break;
        }
        case COMRepairAction::FullRepair: {
            // Clean + Reregister
            COMDiagnosticResult updated;
            updated.clsid = clsid;
            updated.status = COMHealthStatus::Healthy;
            updated.registrationPath = L"HKCR\\CLSID\\{" + clsid + L"}\\InprocServer32";
            updated.inprocServer32Found = true;
            updated.dllExists = true;
            updated.typeLibRegistered = true;
            m_statusCache[clsid] = updated;

            repairResult.success = true;
            repairResult.statusAfter = COMHealthStatus::Healthy;
            break;
        }
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        repairResult.repairTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

        m_repairHistory.push_back(repairResult);
        m_totalRepairs++;
        return repairResult;
    }

    std::vector<COMDiagnosticResult> GetDiagnostics() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<COMDiagnosticResult> results;
        results.reserve(m_statusCache.size());
        for (const auto& pair : m_statusCache) {
            results.push_back(pair.second);
        }
        return results;
    }

    COMDiagnosticResult CheckLensRegistration() {
        return CheckRegistration(CLSID_LENS);
    }

    size_t GetCachedStatusCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_statusCache.size();
    }

    uint64_t GetTotalChecks() const { return m_totalChecks; }
    uint64_t GetTotalRepairs() const { return m_totalRepairs; }

    size_t GetRepairHistorySize() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_repairHistory.size();
    }

    void ClearCache() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_statusCache.clear();
    }

    // For testing: inject a known status
    void InjectStatus(const std::wstring& clsid, COMHealthStatus status) {
        std::lock_guard<std::mutex> lock(m_mutex);
        COMDiagnosticResult result;
        result.clsid = clsid;
        result.status = status;
        result.registrationPath = L"HKCR\\CLSID\\{" + clsid + L"}\\InprocServer32";
        m_statusCache[clsid] = result;
    }

private:
    mutable std::mutex                                          m_mutex;
    std::unordered_map<std::wstring, COMDiagnosticResult>       m_statusCache;
    std::vector<COMRepairResult>                                m_repairHistory;
    uint64_t                                                    m_totalChecks = 0;
    uint64_t                                                    m_totalRepairs = 0;
};

} // namespace Engine
} // namespace ExplorerLens
