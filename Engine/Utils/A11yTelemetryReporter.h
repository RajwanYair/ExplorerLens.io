// A11yTelemetryReporter.h — Accessibility Telemetry Reporter
// Copyright (c) 2026 ExplorerLens Project
//
// Collects and reports accessibility usage telemetry: screen reader activations,
// high contrast usage, keyboard navigation frequency, and WCAG audit pass rates.
// All data is aggregated — no personally identifiable information is collected.
//
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

struct A11yTelemetryRecord {
    uint64_t screenReaderSessions  = 0;
    uint64_t highContrastSessions  = 0;
    uint64_t keyboardNavEvents     = 0;
    uint64_t altTextGenerated      = 0;
    uint64_t wcagAuditsPassed      = 0;
    uint64_t wcagAuditsFailed      = 0;
    float    avgCaptionQuality     = 0.0f;
    float    avgContrastRatio      = 0.0f;
    float    a11yScoreAvg          = 0.0f;
};

struct A11yTelemetryReport {
    A11yTelemetryRecord aggregate;
    std::string          generatedAt;
    std::string          reportFormat = "json";
    std::string          payload;
};

class A11yTelemetryReporter {
public:
    A11yTelemetryReporter() = default;

    bool Initialize() { m_ready = true; return true; }
    bool IsReady() const { return m_ready; }

    void RecordScreenReaderSession()    { ++m_record.screenReaderSessions; }
    void RecordHighContrastSession()    { ++m_record.highContrastSessions; }
    void RecordKeyboardNavEvent()       { ++m_record.keyboardNavEvents; }
    void RecordAltTextGenerated()       { ++m_record.altTextGenerated; }
    void RecordWCAGAuditPassed()        { ++m_record.wcagAuditsPassed; }
    void RecordWCAGAuditFailed()        { ++m_record.wcagAuditsFailed; }

    void RecordCaptionQuality(float score) {
        uint64_t n = m_record.wcagAuditsPassed + m_record.wcagAuditsFailed + 1;
        m_record.avgCaptionQuality =
            (m_record.avgCaptionQuality * (n - 1) + score) / static_cast<float>(n);
    }

    void RecordContrastRatio(float ratio) {
        uint64_t n = m_record.screenReaderSessions + m_record.highContrastSessions + 1;
        m_record.avgContrastRatio =
            (m_record.avgContrastRatio * (n - 1) + ratio) / static_cast<float>(n);
        m_record.a11yScoreAvg = (m_record.avgCaptionQuality + m_record.avgContrastRatio) / 2.0f;
    }

    A11yTelemetryReport GenerateReport(const std::string& format = "json") const {
        A11yTelemetryReport rpt;
        rpt.aggregate    = m_record;
        rpt.reportFormat = format;
        rpt.payload = "{\"screenReader\":" + std::to_string(m_record.screenReaderSessions)
                    + ",\"highContrast\":" + std::to_string(m_record.highContrastSessions)
                    + ",\"keyboardNav\":" + std::to_string(m_record.keyboardNavEvents)
                    + ",\"altTextGen\":" + std::to_string(m_record.altTextGenerated)
                    + ",\"wcagPassRate\":"
                    + std::to_string(m_record.wcagAuditsPassed + m_record.wcagAuditsFailed > 0
                        ? 100 * m_record.wcagAuditsPassed /
                          (m_record.wcagAuditsPassed + m_record.wcagAuditsFailed)
                        : 0)
                    + "}";
        return rpt;
    }

    void Reset() { m_record = {}; }

    void Shutdown() { m_ready = false; }

private:
    bool                 m_ready  = false;
    A11yTelemetryRecord  m_record;
};

}} // namespace ExplorerLens::Engine
