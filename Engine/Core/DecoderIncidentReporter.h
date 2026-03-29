// DecoderIncidentReporter.h — Decoder Health Snapshot & Automated Incident Reporting
// Copyright (c) 2026 ExplorerLens Project
//
// Captures point-in-time decoder health snapshots (error rate, avg latency, crash count)
// and generates structured incident reports suitable for telemetry upload and console display.
//
#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace ExplorerLens {
namespace Engine {

enum class IncidentSeverity { Info, Warning, Error, Critical };

struct DecoderHealthSnapshot {
    std::string decoderName;
    double      avgLatencyMs    = 0.0;
    double      errorRatePct    = 0.0;
    int         crashCount      = 0;
    int         totalDecodes    = 0;
    bool        isQuarantined   = false;
    std::chrono::system_clock::time_point captureTime = std::chrono::system_clock::now();
};

struct IncidentReport {
    std::string       id;
    IncidentSeverity  severity     = IncidentSeverity::Info;
    std::string       decoderName;
    std::string       summary;
    std::string       detail;
    std::vector<DecoderHealthSnapshot> snapshots;

    std::string SeverityName() const noexcept {
        switch (severity) {
        case IncidentSeverity::Info:     return "Info";
        case IncidentSeverity::Warning:  return "Warning";
        case IncidentSeverity::Error:    return "Error";
        case IncidentSeverity::Critical: return "Critical";
        }
        return "Unknown";
    }
};

class DecoderIncidentReporter {
public:
    explicit DecoderIncidentReporter() = default;

    IncidentReport CreateReport(const DecoderHealthSnapshot& snap) const {
        IncidentReport r;
        r.id          = GenerateId(snap.decoderName);
        r.decoderName = snap.decoderName;
        r.snapshots   = { snap };

        if (snap.crashCount >= 10 || snap.isQuarantined) {
            r.severity = IncidentSeverity::Critical;
            r.summary  = snap.decoderName + ": quarantined after " + std::to_string(snap.crashCount) + " crashes";
        } else if (snap.errorRatePct >= 50.0 || snap.crashCount >= 5) {
            r.severity = IncidentSeverity::Error;
            r.summary  = snap.decoderName + ": high error rate " + FormatPct(snap.errorRatePct);
        } else if (snap.errorRatePct >= 20.0 || snap.avgLatencyMs > 3000.0) {
            r.severity = IncidentSeverity::Warning;
            r.summary  = snap.decoderName + ": degraded performance";
        } else {
            r.severity = IncidentSeverity::Info;
            r.summary  = snap.decoderName + ": healthy";
        }

        std::ostringstream oss;
        oss << "Decoder=" << snap.decoderName
            << " AvgLatency=" << std::fixed << std::setprecision(1) << snap.avgLatencyMs << "ms"
            << " ErrorRate=" << snap.errorRatePct << "%"
            << " Crashes=" << snap.crashCount
            << " TotalDecodes=" << snap.totalDecodes;
        r.detail = oss.str();
        return r;
    }

    std::vector<IncidentReport> CreateBatchReport(const std::vector<DecoderHealthSnapshot>& snaps) const {
        std::vector<IncidentReport> reports;
        reports.reserve(snaps.size());
        for (const auto& s : snaps) reports.push_back(CreateReport(s));
        return reports;
    }

    int CriticalCount(const std::vector<IncidentReport>& reports) const noexcept {
        int n = 0;
        for (const auto& r : reports) if (r.severity == IncidentSeverity::Critical) n++;
        return n;
    }

private:
    static std::string GenerateId(const std::string& decoder) {
        return "INC-" + decoder + "-" + std::to_string(
            std::chrono::system_clock::now().time_since_epoch().count() % 100000);
    }
    static std::string FormatPct(double v) {
        std::ostringstream oss; oss << std::fixed << std::setprecision(1) << v << "%";
        return oss.str();
    }
};

} // namespace Engine
} // namespace ExplorerLens
