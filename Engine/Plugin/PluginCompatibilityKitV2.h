#pragma once
// Plugin Compatibility Kit v2.0
// ABI stability tests, perf/memory policy gates, crash injection, conformance report.

#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens::Plugin {

// ─── ABI version ──────────────────────────────────────────────────────────────

struct ABIVersion {
    uint32_t major  { 1 };
    uint32_t minor  { 0 };
    uint32_t patch  { 0 };

    bool IsCompatible(const ABIVersion& target) const {
        return major == target.major;  // major-version compatibility only
    }

    std::string ToString() const {
        return std::to_string(major) + "." +
               std::to_string(minor) + "." +
               std::to_string(patch);
    }

    static ABIVersion V1()   { return { 1, 0, 0 }; }
    static ABIVersion V1_1() { return { 1, 1, 0 }; }
    static ABIVersion V2()   { return { 2, 0, 0 }; }

    bool operator==(const ABIVersion& o) const {
        return major == o.major && minor == o.minor && patch == o.patch;
    }
    bool operator!=(const ABIVersion& o) const { return !(*this == o); }
};

// ─── Exported symbol set ───────────────────────────────────────────────────────

struct ExportedSymbol {
    std::string name;
    std::string signature;  // simplified type signature
    bool        isRequired { true };
};

struct ABIStableSurface {
    ABIVersion              version;
    std::vector<ExportedSymbol> symbols;

    static ABIStableSurface V1Baseline() {
        ABIStableSurface s;
        s.version = ABIVersion::V1();
        s.symbols = {
            { "DT_PluginGetVersion",  "uint32_t()",             true  },
            { "DT_PluginInit",        "bool(const char*)",      true  },
            { "DT_PluginDecode",      "bool(const uint8_t*,size_t,void**)", true },
            { "DT_PluginGetCapabilities", "uint32_t()",         true  },
            { "DT_PluginShutdown",    "void()",                 true  },
        };
        return s;
    }
};

// ─── Conformance checks ────────────────────────────────────────────────────────

enum class ConformanceCheckId : uint32_t {
    ABIVersionMatch     = 1,
    RequiredExports     = 2,
    PerfGate            = 3,
    MemoryGate          = 4,
    CrashIsolation      = 5,
    ABIDriftDetection   = 6,
};

inline std::string ToString(ConformanceCheckId id) {
    switch (id) {
        case ConformanceCheckId::ABIVersionMatch:    return "ABIVersionMatch";
        case ConformanceCheckId::RequiredExports:    return "RequiredExports";
        case ConformanceCheckId::PerfGate:           return "PerfGate";
        case ConformanceCheckId::MemoryGate:         return "MemoryGate";
        case ConformanceCheckId::CrashIsolation:     return "CrashIsolation";
        case ConformanceCheckId::ABIDriftDetection:  return "ABIDriftDetection";
        default: return "Unknown";
    }
}

struct ConformanceCheckResult {
    ConformanceCheckId  id;
    bool                passed      { false };
    std::string         message;
    std::string         details;

    static ConformanceCheckResult Pass(ConformanceCheckId id, std::string msg = "") {
        return { id, true, std::move(msg), "" };
    }
    static ConformanceCheckResult Fail(ConformanceCheckId id, std::string msg, std::string det = "") {
        return { id, false, std::move(msg), std::move(det) };
    }
};

// ─── Performance gate ─────────────────────────────────────────────────────────

struct PluginPerfGate {
    double  maxDecodeMs     { 100.0 };  // p95 decode must be under 100ms
    double  maxInitMs       { 500.0 };  // init must complete under 500ms
    double  maxShutdownMs   { 200.0 };  // shutdown must complete under 200ms

    ConformanceCheckResult Evaluate(double decodeMs, double initMs, double shutdownMs) const {
        if (decodeMs > maxDecodeMs)
            return ConformanceCheckResult::Fail(ConformanceCheckId::PerfGate,
                "decode too slow", "got " + std::to_string(decodeMs) + "ms, limit " + std::to_string(maxDecodeMs) + "ms");
        if (initMs > maxInitMs)
            return ConformanceCheckResult::Fail(ConformanceCheckId::PerfGate,
                "init too slow", "got " + std::to_string(initMs) + "ms");
        if (shutdownMs > maxShutdownMs)
            return ConformanceCheckResult::Fail(ConformanceCheckId::PerfGate,
                "shutdown too slow", "got " + std::to_string(shutdownMs) + "ms");
        return ConformanceCheckResult::Pass(ConformanceCheckId::PerfGate, "all timing gates pass");
    }
};

// ─── Memory gate ─────────────────────────────────────────────────────────────

struct PluginMemoryGate {
    uint64_t    maxPeakBytes    { 50ULL * 1024 * 1024 };  // 50 MB peak per decode

    ConformanceCheckResult Evaluate(uint64_t peakBytes) const {
        if (peakBytes > maxPeakBytes)
            return ConformanceCheckResult::Fail(ConformanceCheckId::MemoryGate,
                "peak memory exceeded",
                "got " + std::to_string(peakBytes / (1024*1024)) + "MB, limit 50MB");
        return ConformanceCheckResult::Pass(ConformanceCheckId::MemoryGate,
            "memory within budget");
    }
};

// ─── Conformance report ───────────────────────────────────────────────────────

struct PluginConformanceReport {
    std::string                         pluginId;
    ABIVersion                          reportedVersion;
    std::vector<ConformanceCheckResult> checks;
    uint32_t                            passCount   { 0 };
    uint32_t                            failCount   { 0 };

    bool AllPassed() const { return failCount == 0; }

    std::string ToJSON() const {
        std::string json = "{\n  \"pluginId\": \"" + pluginId + "\",\n";
        json += "  \"abiVersion\": \"" + reportedVersion.ToString() + "\",\n";
        json += "  \"pass\": " + std::to_string(passCount) + ",\n";
        json += "  \"fail\": " + std::to_string(failCount) + ",\n";
        json += "  \"verdict\": \"" + std::string(AllPassed() ? "PASS" : "FAIL") + "\"\n}";
        return json;
    }

    std::string Summary() const {
        return "Conformance " + pluginId + ": " +
               std::to_string(passCount) + "P/" + std::to_string(failCount) + "F — " +
               (AllPassed() ? "PASS" : "FAIL");
    }
};

// ─── ABI drift detector ───────────────────────────────────────────────────────

class ABIDriftDetector {
public:
    ABIDriftDetector(const ABIStableSurface& baseline, const ABIStableSurface& current)
        : m_baseline(baseline), m_current(current) {}

    ConformanceCheckResult Detect() const {
        if (!m_baseline.version.IsCompatible(m_current.version)) {
            return ConformanceCheckResult::Fail(ConformanceCheckId::ABIDriftDetection,
                "ABI version mismatch",
                "baseline=" + m_baseline.version.ToString() +
                " current=" + m_current.version.ToString());
        }

        std::vector<std::string> missing;
        for (const auto& sym : m_baseline.symbols) {
            if (!sym.isRequired) continue;
            bool found = false;
            for (const auto& cur : m_current.symbols) {
                if (cur.name == sym.name) { found = true; break; }
            }
            if (!found) missing.push_back(sym.name);
        }

        if (!missing.empty()) {
            std::string detail = "missing exports:";
            for (const auto& m : missing) detail += " " + m;
            return ConformanceCheckResult::Fail(ConformanceCheckId::ABIDriftDetection,
                "required exports missing", detail);
        }

        return ConformanceCheckResult::Pass(ConformanceCheckId::ABIDriftDetection,
            "no ABI drift detected");
    }

private:
    ABIStableSurface m_baseline;
    ABIStableSurface m_current;
};

} // namespace ExplorerLens::Plugin

