#pragma once
// Sprint 138 — Win11 Compatibility Matrix Execution
// Full OS/GPU/DPI matrix validation framework for Windows 10/11 builds.
// Defines test scenarios, pass criteria, and compatibility reporting.

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <array>
#include <algorithm>

namespace DarkThumbs::Utils {

// ─── Windows build identifiers ───────────────────────────────────
enum class WindowsBuild : uint16_t {
    Win10_21H2   = 19044,
    Win10_22H2   = 19045,
    Win11_21H2   = 22000,
    Win11_22H2   = 22621,
    Win11_23H2   = 22631,
    Win11_24H2   = 26100,
    Unknown      = 0
};

inline const char* WindowsBuildName(WindowsBuild b) {
    switch (b) {
        case WindowsBuild::Win10_21H2: return "Windows 10 21H2 (19044)";
        case WindowsBuild::Win10_22H2: return "Windows 10 22H2 (19045)";
        case WindowsBuild::Win11_21H2: return "Windows 11 21H2 (22000)";
        case WindowsBuild::Win11_22H2: return "Windows 11 22H2 (22621)";
        case WindowsBuild::Win11_23H2: return "Windows 11 23H2 (22631)";
        case WindowsBuild::Win11_24H2: return "Windows 11 24H2 (26100)";
        default: return "Unknown";
    }
}

inline bool IsWindows11(WindowsBuild b) {
    return static_cast<uint16_t>(b) >= 22000;
}

// ─── GPU vendor ──────────────────────────────────────────────────
enum class GPUVendor : uint8_t {
    NVIDIA   = 0,
    AMD      = 1,
    Intel    = 2,
    Software = 3,   // WARP / software renderer
    Unknown  = 255
};

inline const char* GPUVendorName(GPUVendor v) {
    switch (v) {
        case GPUVendor::NVIDIA:   return "NVIDIA";
        case GPUVendor::AMD:      return "AMD";
        case GPUVendor::Intel:    return "Intel";
        case GPUVendor::Software: return "Software (WARP)";
        default: return "Unknown";
    }
}

// ─── DPI scaling factors ────────────────────────────────────────
enum class DPIScale : uint8_t {
    Scale_100 = 100,   // 96 DPI
    Scale_125 = 125,   // 120 DPI
    Scale_150 = 150,   // 144 DPI
    Scale_175 = 175,   // 168 DPI
    Scale_200 = 200,   // 192 DPI
    Scale_250 = 250,   // 240 DPI
    Scale_300 = 0      // 288 DPI (overflow in uint8_t, use 0 as sentinel)
};

inline uint32_t DPIFromScale(DPIScale s) {
    if (s == DPIScale::Scale_300) return 288;
    return static_cast<uint32_t>(s) * 96 / 100;
}

// ─── Compatibility test scenario ─────────────────────────────────
struct CompatTestScenario {
    std::string   name;
    WindowsBuild  osBuild = WindowsBuild::Unknown;
    GPUVendor     gpu = GPUVendor::Unknown;
    DPIScale      dpi = DPIScale::Scale_100;
    bool          darkMode = false;
    bool          multiMonitor = false;

    std::string Description() const {
        return name + " [" + WindowsBuildName(osBuild) + ", " +
               GPUVendorName(gpu) + ", DPI=" +
               std::to_string(DPIFromScale(dpi)) + "]";
    }
};

// ─── Test result ────────────────────────────────────────────────
enum class CompatResult : uint8_t {
    Pass       = 0,
    Fail       = 1,
    Warning    = 2,   // Works but with visual artifacts
    Skipped    = 3,   // Environment not available
    NotTested  = 4
};

inline const char* CompatResultName(CompatResult r) {
    switch (r) {
        case CompatResult::Pass:      return "PASS";
        case CompatResult::Fail:      return "FAIL";
        case CompatResult::Warning:   return "WARNING";
        case CompatResult::Skipped:   return "SKIPPED";
        case CompatResult::NotTested: return "NOT TESTED";
        default: return "Unknown";
    }
}

// ─── Test execution record ──────────────────────────────────────
struct CompatTestExecution {
    CompatTestScenario scenario;
    CompatResult       result = CompatResult::NotTested;
    std::string        notes;
    double             durationMs = 0.0;

    // Sub-checks
    bool comRegistrationOk = false;
    bool thumbnailRenderOk = false;
    bool darkModeOk = false;
    bool dpiScalingOk = false;
    bool gpuAccelOk = false;
    bool shellIntegrationOk = false;

    bool IsFullPass() const {
        return result == CompatResult::Pass &&
               comRegistrationOk && thumbnailRenderOk &&
               dpiScalingOk && gpuAccelOk && shellIntegrationOk;
    }
};

// ─── Compatibility matrix ───────────────────────────────────────
struct CompatMatrixStats {
    uint32_t totalScenarios = 0;
    uint32_t passed = 0;
    uint32_t failed = 0;
    uint32_t warnings = 0;
    uint32_t skipped = 0;
    uint32_t notTested = 0;

    double PassRate() const {
        uint32_t tested = passed + failed + warnings;
        return tested > 0 ? static_cast<double>(passed) / tested : 0.0;
    }

    bool MeetsMinimum(double threshold = 0.95) const {
        return PassRate() >= threshold;
    }
};

class CompatibilityMatrix {
public:
    CompatibilityMatrix() { BuildDefaultMatrix(); }

    void AddScenario(const CompatTestScenario& scenario) {
        CompatTestExecution exec;
        exec.scenario = scenario;
        m_executions.push_back(exec);
    }

    void RecordResult(size_t index, CompatResult result) {
        if (index < m_executions.size()) {
            m_executions[index].result = result;
        }
    }

    void RecordExecution(const CompatTestExecution& exec) {
        m_executions.push_back(exec);
    }

    CompatMatrixStats GetStats() const {
        CompatMatrixStats stats;
        stats.totalScenarios = static_cast<uint32_t>(m_executions.size());
        for (auto& e : m_executions) {
            switch (e.result) {
                case CompatResult::Pass:      stats.passed++; break;
                case CompatResult::Fail:      stats.failed++; break;
                case CompatResult::Warning:   stats.warnings++; break;
                case CompatResult::Skipped:   stats.skipped++; break;
                case CompatResult::NotTested: stats.notTested++; break;
            }
        }
        return stats;
    }

    size_t ScenarioCount() const { return m_executions.size(); }

    const std::vector<CompatTestExecution>& Executions() const { return m_executions; }

    static CompatibilityMatrix Create() { return CompatibilityMatrix(); }

private:
    void BuildDefaultMatrix() {
        // Core OS builds × GPU vendors
        static const WindowsBuild builds[] = {
            WindowsBuild::Win10_22H2,
            WindowsBuild::Win11_22H2,
            WindowsBuild::Win11_23H2,
            WindowsBuild::Win11_24H2
        };
        static const GPUVendor gpus[] = {
            GPUVendor::NVIDIA, GPUVendor::AMD, GPUVendor::Intel
        };
        static const DPIScale dpis[] = {
            DPIScale::Scale_100, DPIScale::Scale_150, DPIScale::Scale_200
        };

        int idx = 0;
        for (auto build : builds) {
            for (auto gpu : gpus) {
                for (auto dpi : dpis) {
                    CompatTestScenario s;
                    s.name = "Matrix_" + std::to_string(idx++);
                    s.osBuild = build;
                    s.gpu = gpu;
                    s.dpi = dpi;
                    s.darkMode = (idx % 2 == 0);
                    AddScenario(s);
                }
            }
        }
    }

    std::vector<CompatTestExecution> m_executions;
};

} // namespace DarkThumbs::Utils
