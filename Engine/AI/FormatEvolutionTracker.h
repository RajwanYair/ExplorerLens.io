// FormatEvolutionTracker.h — Format Evolution Tracker
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks version drift in binary format signatures over time — detecting when
// a format's magic bytes or structure shifts across generations of encoder tools.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

enum class DriftSeverity     { None, Minor, Moderate, Major, Breaking };

struct FormatSignatureVersion {
    std::string  formatId;
    uint32_t     schemaVersion  = 1;
    std::vector<uint8_t> magicBytes;
    std::string  encoderVersion;
    uint64_t     observedAtMs   = 0;
    float        driftScore     = 0.0f;
};

struct DriftEvent {
    std::string    formatId;
    DriftSeverity  severity     = DriftSeverity::None;
    uint32_t       fromVersion  = 0;
    uint32_t       toVersion    = 0;
    float          driftDelta   = 0.0f;
    std::string    description;
};

class FormatEvolutionTracker {
public:
    FormatEvolutionTracker() = default;

    void RecordObservation(const FormatSignatureVersion& sig) {
        m_history.push_back(sig);
        m_history.back().observedAtMs = ++m_clock;
    }

    DriftEvent AnalyseDrift(const std::string& formatId) const {
        DriftEvent evt;
        evt.formatId = formatId;
        std::vector<const FormatSignatureVersion*> matching;
        for (const auto& s : m_history)
            if (s.formatId == formatId) matching.push_back(&s);
        if (matching.size() < 2) { evt.severity = DriftSeverity::None; return evt; }
        float delta = matching.back()->driftScore - matching.front()->driftScore;
        evt.driftDelta  = delta < 0 ? -delta : delta;
        evt.fromVersion = matching.front()->schemaVersion;
        evt.toVersion   = matching.back()->schemaVersion;
        if      (evt.driftDelta > 0.75f) evt.severity = DriftSeverity::Breaking;
        else if (evt.driftDelta > 0.50f) evt.severity = DriftSeverity::Major;
        else if (evt.driftDelta > 0.25f) evt.severity = DriftSeverity::Moderate;
        else if (evt.driftDelta > 0.05f) evt.severity = DriftSeverity::Minor;
        else                             evt.severity = DriftSeverity::None;
        return evt;
    }

    std::vector<std::string>  TrackedFormats() const {
        std::vector<std::string> ids;
        for (const auto& s : m_history)
            if (std::find(ids.begin(), ids.end(), s.formatId) == ids.end())
                ids.push_back(s.formatId);
        return ids;
    }

    size_t  ObservationCount() const { return m_history.size(); }
    void    Clear()                  { m_history.clear(); m_clock = 0; }

private:
    std::vector<FormatSignatureVersion> m_history;
    uint64_t                            m_clock = 0;
};

} // namespace Engine
} // namespace ExplorerLens
