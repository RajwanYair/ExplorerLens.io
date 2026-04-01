// DecoderIncidentReporter.h — Decoder Health Incident Reporting
// Copyright (c) 2026 ExplorerLens Project
//
// Converts a DecoderHealthSnapshot into a structured IncidentReport suitable
// for logging, alerting, and dashboard display. Severity is computed from
// error rate, crash count, and quarantine status.
//
#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <random>
#include <iomanip>

namespace ExplorerLens { namespace Engine {

enum class IncidentSeverity : uint8_t {
    Info     = 0,
    Warning  = 1,
    Error    = 2,
    Critical = 3,
};

struct DecoderHealthSnapshot {
    std::string decoderName;
    double      errorRatePct  = 0.0;
    int         crashCount    = 0;
    int         totalDecodes  = 0;
    bool        isQuarantined = false;
};

struct IncidentReport {
    std::string      id;
    std::string      decoderName;
    IncidentSeverity severity = IncidentSeverity::Info;
    std::string      summary;
};

class DecoderIncidentReporter {
public:
    IncidentReport CreateReport(const DecoderHealthSnapshot& snap) const {
        IncidentReport r;
        r.id          = GenerateId();
        r.decoderName = snap.decoderName;
        r.severity    = ComputeSeverity(snap);
        r.summary     = BuildSummary(snap);
        return r;
    }

    std::vector<IncidentReport> CreateBatchReport(
        const std::vector<DecoderHealthSnapshot>& snaps) const
    {
        std::vector<IncidentReport> result;
        result.reserve(snaps.size());
        for (const auto& s : snaps) result.push_back(CreateReport(s));
        return result;
    }

    int CriticalCount(const std::vector<IncidentReport>& reports) const noexcept {
        int n = 0;
        for (const auto& r : reports)
            if (r.severity == IncidentSeverity::Critical) ++n;
        return n;
    }

private:
    static IncidentSeverity ComputeSeverity(const DecoderHealthSnapshot& s) noexcept {
        if (s.isQuarantined || s.crashCount >= 10) return IncidentSeverity::Critical;
        if (s.crashCount >= 5 || s.errorRatePct >= 20.0) return IncidentSeverity::Error;
        if (s.crashCount >= 1 || s.errorRatePct >= 5.0)  return IncidentSeverity::Warning;
        return IncidentSeverity::Info;
    }

    static std::string BuildSummary(const DecoderHealthSnapshot& s) {
        std::ostringstream ss;
        ss << s.decoderName << ": crashes=" << s.crashCount
           << " err=" << s.errorRatePct << "% total=" << s.totalDecodes;
        return ss.str();
    }

    static std::string GenerateId() {
        static uint64_t seq = 0;
        std::ostringstream ss;
        ss << "INC-" << std::hex << std::uppercase << ++seq;
        return ss.str();
    }
};

}} // namespace ExplorerLens::Engine
