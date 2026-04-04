#pragma once
// ============================================================================
// PerfRegressionGate.h
// CI-enforceable performance regression gate with KPI thresholds and trends
// ============================================================================

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <map>
#include <numeric>
#include <string>
#include <vector>

namespace ExplorerLens {

// ── KPI identifiers ────────────────────────────────────────────────────────

enum class PerfKPI {
    SingleThumbnailMs,      // p50 single thumbnail latency
    SingleThumbnailP95Ms,   // p95 single thumbnail latency
    BatchThroughputImgSec,  // images/sec batch throughput
    CacheHitMs,             // cache hit latency
    ColdStartMs,            // first-thumbnail-after-boot latency
    MemoryPeakMB,           // peak working set during batch
    GpuFrameMs,             // GPU render frame time
    DecoderInitMs           // decoder initialization time
};

inline const char* KpiName(PerfKPI kpi)
{
    switch (kpi) {
        case PerfKPI::SingleThumbnailMs:
            return "SingleThumbnailMs";
        case PerfKPI::SingleThumbnailP95Ms:
            return "SingleThumbnailP95Ms";
        case PerfKPI::BatchThroughputImgSec:
            return "BatchThroughputImgSec";
        case PerfKPI::CacheHitMs:
            return "CacheHitMs";
        case PerfKPI::ColdStartMs:
            return "ColdStartMs";
        case PerfKPI::MemoryPeakMB:
            return "MemoryPeakMB";
        case PerfKPI::GpuFrameMs:
            return "GpuFrameMs";
        case PerfKPI::DecoderInitMs:
            return "DecoderInitMs";
    }
    return "Unknown";
}

// ── KPI threshold ──────────────────────────────────────────────────────────

enum class ThresholdDirection {
    LowerIsBetter,  // latency — value should be below threshold
    HigherIsBetter  // throughput — value should be above threshold
};

struct KpiThreshold
{
    PerfKPI kpi;
    double warnThreshold = 0.0;
    double failThreshold = 0.0;
    ThresholdDirection direction = ThresholdDirection::LowerIsBetter;
    double regressionPct = 10.0;  // % regression vs baseline → warn
    double criticalPct = 25.0;    // % regression vs baseline → fail
};

// ── Measurement sample ─────────────────────────────────────────────────────

struct PerfSample
{
    PerfKPI kpi;
    double value = 0.0;
    std::string label;  // e.g. "CBZ-100pages", "WEBP-4K"
    std::string commitHash;
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();
};

// ── Gate verdict ───────────────────────────────────────────────────────────

enum class GateVerdict {
    Pass,
    Warn,
    Fail
};

struct KpiResult
{
    PerfKPI kpi;
    double measured = 0.0;
    double baseline = 0.0;
    double threshold = 0.0;
    double regressionPct = 0.0;
    GateVerdict verdict = GateVerdict::Pass;
    std::string detail;
};

struct GateResult
{
    GateVerdict overall = GateVerdict::Pass;
    int passCount = 0;
    int warnCount = 0;
    int failCount = 0;
    std::vector<KpiResult> results;
    std::chrono::milliseconds evalDuration{0};

    bool Passed() const
    {
        return overall != GateVerdict::Fail;
    }
};

// ── Trend statistics ───────────────────────────────────────────────────────

struct TrendStats
{
    PerfKPI kpi;
    double mean = 0.0;
    double stddev = 0.0;
    double min = 0.0;
    double max = 0.0;
    double latest = 0.0;
    int sampleCount = 0;
    double trendSlope = 0.0;  // positive = getting worse for LowerIsBetter

    bool IsDegrading(ThresholdDirection dir) const
    {
        if (dir == ThresholdDirection::LowerIsBetter)
            return trendSlope > 0;
        return trendSlope < 0;
    }
};

// ── Performance Regression Gate ────────────────────────────────────────────

class PerfRegressionGate
{
  public:
    PerfRegressionGate()
    {
        SetDefaultThresholds();
    }

    // Configure thresholds
    void SetThreshold(KpiThreshold threshold)
    {
        m_thresholds[threshold.kpi] = std::move(threshold);
    }

    void SetDefaultThresholds()
    {
        m_thresholds[PerfKPI::SingleThumbnailMs] = {PerfKPI::SingleThumbnailMs, 15.0, 25.0,
                                                    ThresholdDirection::LowerIsBetter};
        m_thresholds[PerfKPI::SingleThumbnailP95Ms] = {PerfKPI::SingleThumbnailP95Ms, 25.0, 40.0,
                                                       ThresholdDirection::LowerIsBetter};
        m_thresholds[PerfKPI::BatchThroughputImgSec] = {PerfKPI::BatchThroughputImgSec, 200.0, 150.0,
                                                        ThresholdDirection::HigherIsBetter};
        m_thresholds[PerfKPI::CacheHitMs] = {PerfKPI::CacheHitMs, 3.0, 8.0, ThresholdDirection::LowerIsBetter};
        m_thresholds[PerfKPI::ColdStartMs] = {PerfKPI::ColdStartMs, 100.0, 200.0, ThresholdDirection::LowerIsBetter};
        m_thresholds[PerfKPI::MemoryPeakMB] = {PerfKPI::MemoryPeakMB, 150.0, 250.0, ThresholdDirection::LowerIsBetter};
        m_thresholds[PerfKPI::GpuFrameMs] = {PerfKPI::GpuFrameMs, 8.0, 16.0, ThresholdDirection::LowerIsBetter};
        m_thresholds[PerfKPI::DecoderInitMs] = {PerfKPI::DecoderInitMs, 10.0, 20.0, ThresholdDirection::LowerIsBetter};
    }

    // Record a measurement
    void RecordSample(PerfSample sample)
    {
        m_history[sample.kpi].push_back(std::move(sample));
    }

    // Set baseline values (e.g. from previous release)
    void SetBaseline(PerfKPI kpi, double value)
    {
        m_baselines[kpi] = value;
    }

    // Evaluate current measurements against thresholds
    GateResult Evaluate(const std::map<PerfKPI, double>& current) const
    {
        auto start = std::chrono::steady_clock::now();
        GateResult gate;

        for (auto& [kpi, value] : current) {
            KpiResult r;
            r.kpi = kpi;
            r.measured = value;

            auto thIt = m_thresholds.find(kpi);
            if (thIt == m_thresholds.end()) {
                r.verdict = GateVerdict::Pass;
                r.detail = "No threshold configured";
                gate.results.push_back(r);
                gate.passCount++;
                continue;
            }

            auto& th = thIt->second;
            r.threshold = th.failThreshold;

            // Check baseline regression
            auto blIt = m_baselines.find(kpi);
            if (blIt != m_baselines.end()) {
                r.baseline = blIt->second;
                if (r.baseline > 0) {
                    if (th.direction == ThresholdDirection::LowerIsBetter)
                        r.regressionPct = ((value - r.baseline) / r.baseline) * 100.0;
                    else
                        r.regressionPct = ((r.baseline - value) / r.baseline) * 100.0;
                }
            }

            // Classify verdict
            r.verdict = ClassifyVerdict(value, th, r.regressionPct);

            switch (r.verdict) {
                case GateVerdict::Pass:
                    gate.passCount++;
                    r.detail = "Within limits";
                    break;
                case GateVerdict::Warn:
                    gate.warnCount++;
                    r.detail = "Near threshold";
                    break;
                case GateVerdict::Fail:
                    gate.failCount++;
                    r.detail = "Threshold exceeded";
                    break;
            }
            gate.results.push_back(r);
        }

        gate.overall = gate.failCount > 0   ? GateVerdict::Fail
                       : gate.warnCount > 0 ? GateVerdict::Warn
                                            : GateVerdict::Pass;

        auto end = std::chrono::steady_clock::now();
        gate.evalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        return gate;
    }

    // Compute trend statistics for a KPI
    TrendStats ComputeTrend(PerfKPI kpi) const
    {
        TrendStats ts;
        ts.kpi = kpi;

        auto it = m_history.find(kpi);
        if (it == m_history.end() || it->second.empty())
            return ts;

        auto& samples = it->second;
        ts.sampleCount = static_cast<int>(samples.size());

        std::vector<double> vals;
        vals.reserve(samples.size());
        for (auto& s : samples)
            vals.push_back(s.value);

        ts.min = *std::min_element(vals.begin(), vals.end());
        ts.max = *std::max_element(vals.begin(), vals.end());
        ts.latest = vals.back();
        ts.mean = std::accumulate(vals.begin(), vals.end(), 0.0) / vals.size();

        if (vals.size() > 1) {
            double sq_sum = 0;
            for (auto v : vals)
                sq_sum += (v - ts.mean) * (v - ts.mean);
            ts.stddev = std::sqrt(sq_sum / (vals.size() - 1));

            // Simple linear trend slope (index-based)
            double xMean = (vals.size() - 1) / 2.0;
            double num = 0, den = 0;
            for (size_t i = 0; i < vals.size(); ++i) {
                num += (i - xMean) * (vals[i] - ts.mean);
                den += (i - xMean) * (i - xMean);
            }
            if (den > 0)
                ts.trendSlope = num / den;
        }

        return ts;
    }

    // Generate CI report
    static std::string FormatReport(const GateResult& gate)
    {
        std::string rpt;
        rpt += "=== Performance Regression Gate ===\n";
        rpt += "Overall: " + VerdictLabel(gate.overall) + "\n";
        rpt += "Pass: " + std::to_string(gate.passCount) + " Warn: " + std::to_string(gate.warnCount)
               + " Fail: " + std::to_string(gate.failCount) + "\n\n";

        for (auto& r : gate.results) {
            rpt += "[" + VerdictLabel(r.verdict) + "] " + KpiName(r.kpi) + ": " + FormatDouble(r.measured);
            if (r.baseline > 0)
                rpt += " (baseline=" + FormatDouble(r.baseline) + ", delta=" + FormatDouble(r.regressionPct) + "%)";
            rpt += " — " + r.detail + "\n";
        }
        return rpt;
    }

    const std::map<PerfKPI, KpiThreshold>& Thresholds() const
    {
        return m_thresholds;
    }

  private:
    static GateVerdict ClassifyVerdict(double value, const KpiThreshold& th, double regressionPct)
    {
        bool thresholdViolation = false;
        bool warnViolation = false;

        if (th.direction == ThresholdDirection::LowerIsBetter) {
            thresholdViolation = value > th.failThreshold;
            warnViolation = value > th.warnThreshold;
        } else {
            thresholdViolation = value < th.failThreshold;
            warnViolation = value < th.warnThreshold;
        }

        if (thresholdViolation || regressionPct > th.criticalPct)
            return GateVerdict::Fail;
        if (warnViolation || regressionPct > th.regressionPct)
            return GateVerdict::Warn;
        return GateVerdict::Pass;
    }

    static std::string VerdictLabel(GateVerdict v)
    {
        switch (v) {
            case GateVerdict::Pass:
                return "PASS";
            case GateVerdict::Warn:
                return "WARN";
            case GateVerdict::Fail:
                return "FAIL";
        }
        return "?";
    }

    static std::string FormatDouble(double v)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.2f", v);
        return buf;
    }

    std::map<PerfKPI, KpiThreshold> m_thresholds;
    std::map<PerfKPI, double> m_baselines;
    std::map<PerfKPI, std::vector<PerfSample>> m_history;
};

}  // namespace ExplorerLens
