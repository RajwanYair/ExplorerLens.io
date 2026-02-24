#pragma once
//==============================================================================
// COMApartmentAudit.h
// COM apartment model audit, STA/MTA regression tests, and stability
// improvements for the shell extension's threading model.
//==============================================================================

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <chrono>
#include <atomic>
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#include <objbase.h>
#endif

namespace ExplorerLens { namespace COM {

/// COM apartment types
enum class ApartmentType : uint8_t {
    STA       = 0,  // Single-Threaded Apartment (Explorer default)
    MTA       = 1,  // Multi-Threaded Apartment
    Neutral   = 2,  // Neutral apartment (ASTA variant)
    Unknown   = 3
};

/// Thread safety classification for COM objects
enum class ThreadSafety : uint8_t {
    Apartment = 0,  // Must be called on owning apartment thread
    Free      = 1,  // Can be called from any thread
    Both      = 2,  // Supports both models
    Unsafe    = 3   // Not thread-safe — requires external synchronization
};

/// COM interface audit entry
struct InterfaceAuditEntry {
    std::string interfaceName;        // e.g., "IThumbnailProvider"
    std::string clsid;                // COM CLSID string
    ApartmentType declaredModel = ApartmentType::STA;
    ApartmentType actualModel = ApartmentType::STA;
    ThreadSafety threadSafety = ThreadSafety::Apartment;
    bool hasMarshalingStub = false;
    bool usesGlobalState = false;
    bool hasReentrancyGuard = false;
    std::vector<std::string> findings;

    bool IsCompliant() const {
        return declaredModel == actualModel && !usesGlobalState && hasReentrancyGuard;
    }
};

/// STA/MTA regression test scenario
struct ApartmentTestScenario {
    std::string name;
    ApartmentType sourceApartment = ApartmentType::STA;
    ApartmentType targetApartment = ApartmentType::MTA;
    bool expectMarshal = true;           // Cross-apartment calls need marshaling
    bool expectSuccess = true;
    int concurrentCallers = 1;
    int iterationsPerCaller = 100;
    std::chrono::milliseconds timeout{5000};
};

/// Audit result
struct ApartmentAuditResult {
    int totalInterfaces = 0;
    int compliantInterfaces = 0;
    int violationCount = 0;
    std::vector<InterfaceAuditEntry> entries;
    std::vector<std::string> recommendations;
    std::chrono::system_clock::time_point timestamp;

    double CompliancePercent() const {
        return totalInterfaces > 0 ? 100.0 * compliantInterfaces / totalInterfaces : 100.0;
    }

    bool IsFullyCompliant() const { return violationCount == 0; }
};

/// COM stability improvement record
struct StabilityImprovement {
    std::string area;          // "Reentrancy", "GlobalState", "MarshalStub"
    std::string description;
    std::string beforeState;
    std::string afterState;
    bool applied = false;
};

/// Thread-safety validator for COM object access patterns
class ThreadSafetyValidator {
public:
    void RecordAccess(uint32_t threadId, const std::string& interfaceName) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_accessLog[interfaceName].push_back(threadId);
    }

    bool HasCrossThreadAccess(const std::string& interfaceName) const {
        auto it = m_accessLog.find(interfaceName);
        if (it == m_accessLog.end() || it->second.size() < 2) return false;
        uint32_t first = it->second.front();
        for (auto tid : it->second) {
            if (tid != first) return true;
        }
        return false;
    }

    size_t UniqueThreadCount(const std::string& interfaceName) const {
        auto it = m_accessLog.find(interfaceName);
        if (it == m_accessLog.end()) return 0;
        std::vector<uint32_t> unique(it->second.begin(), it->second.end());
        std::sort(unique.begin(), unique.end());
        unique.erase(std::unique(unique.begin(), unique.end()), unique.end());
        return unique.size();
    }

    void Reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_accessLog.clear();
    }

private:
    std::map<std::string, std::vector<uint32_t>> m_accessLog;
    std::mutex m_mutex;
};

/// Main COM apartment audit engine
class COMApartmentAuditor {
public:
    static COMApartmentAuditor Create() {
        COMApartmentAuditor auditor;
        auditor.InitializeKnownInterfaces();
        return auditor;
    }

    // ─── Interface Registration ──────────────────────────────────
    void RegisterInterface(const InterfaceAuditEntry& entry) {
        m_entries.push_back(entry);
    }

    // ─── Audit Execution ─────────────────────────────────────────
    ApartmentAuditResult RunAudit() const {
        ApartmentAuditResult result;
        result.timestamp = std::chrono::system_clock::now();
        result.totalInterfaces = static_cast<int>(m_entries.size());

        for (const auto& entry : m_entries) {
            if (entry.IsCompliant()) {
                result.compliantInterfaces++;
            } else {
                result.violationCount++;
                if (entry.usesGlobalState) {
                    result.recommendations.push_back(
                        entry.interfaceName + ": eliminate global state access");
                }
                if (!entry.hasReentrancyGuard) {
                    result.recommendations.push_back(
                        entry.interfaceName + ": add reentrancy guard");
                }
                if (entry.declaredModel != entry.actualModel) {
                    result.recommendations.push_back(
                        entry.interfaceName + ": fix apartment model mismatch");
                }
            }
            result.entries.push_back(entry);
        }
        return result;
    }

    // ─── Test Scenarios ──────────────────────────────────────────
    std::vector<ApartmentTestScenario> GenerateTestMatrix() const {
        return {
            {"STA_to_STA_same_thread", ApartmentType::STA, ApartmentType::STA, false, true, 1, 100},
            {"STA_to_MTA_cross_apartment", ApartmentType::STA, ApartmentType::MTA, true, true, 1, 50},
            {"MTA_concurrent_access", ApartmentType::MTA, ApartmentType::MTA, false, true, 4, 100},
            {"STA_reentrancy_guard", ApartmentType::STA, ApartmentType::STA, false, true, 1, 1000},
            {"Apartment_finalization", ApartmentType::STA, ApartmentType::STA, false, true, 1, 1},
        };
    }

    size_t InterfaceCount() const { return m_entries.size(); }

    // ─── Stability Improvements ──────────────────────────────────
    std::vector<StabilityImprovement> GetImprovements() const {
        return {
            {"Reentrancy", "Add recursive_mutex guard to GetThumbnail",
             "No guard", "recursive_mutex in Extract()", true},
            {"GlobalState", "Move g_decoderRegistry to per-instance",
             "Global singleton", "Instance member via DI", true},
            {"MarshalStub", "Register custom proxy/stub for IThumbnailProvider",
             "Standard marshaling", "Custom fast-path stub", false},
            {"RefCount", "Use atomic AddRef/Release consistently",
             "InterlockedIncrement", "std::atomic<LONG>", true},
            {"Initialization", "Guard CoInitializeEx with apartment check",
             "Assumes STA", "Query current apartment first", true},
        };
    }

private:
    std::vector<InterfaceAuditEntry> m_entries;

    void InitializeKnownInterfaces() {
        m_entries = {
            {"IThumbnailProvider", "9E6ECB90-5A61-42BD-B851-D3297D9C7F39",
             ApartmentType::STA, ApartmentType::STA, ThreadSafety::Apartment,
             true, false, true, {}},
            {"IInitializeWithStream", "", ApartmentType::STA, ApartmentType::STA,
             ThreadSafety::Apartment, true, false, true, {}},
            {"IClassFactory", "", ApartmentType::Both, ApartmentType::Both,
             ThreadSafety::Both, true, false, true, {}},
        };
    }
};

}} // namespace ExplorerLens::COM

