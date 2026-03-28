#include "DiagnosticDashboard.h"
#include <chrono>

namespace ExplorerLens { namespace Engine {

DiagnosticDashboard::DiagnosticDashboard() = default;

const wchar_t* DiagnosticDashboard::GetCategoryName(MetricCategory cat) {
 switch (cat) {
 case MetricCategory::CPU: return L"CPU";
 case MetricCategory::Memory: return L"Memory";
 case MetricCategory::GPU: return L"GPU";
 case MetricCategory::Disk: return L"Disk";
 case MetricCategory::Network: return L"Network";
 case MetricCategory::Decoder: return L"Decoder";
 case MetricCategory::Cache: return L"Cache";
 default: return L"Unknown";
 }
}

const wchar_t* DiagnosticDashboard::GetHealthName(DiagHealthLevel level) {
 switch (level) {
 case DiagHealthLevel::Healthy: return L"Healthy";
 case DiagHealthLevel::Warning: return L"Warning";
 case DiagHealthLevel::Degraded: return L"Degraded";
 case DiagHealthLevel::Critical: return L"Critical";
 case DiagHealthLevel::Unknown: return L"Unknown";
 default: return L"Unknown";
 }
}

DiagHealthLevel DiagnosticDashboard::EvaluateHealth(double value, double threshold) const {
 double ratio = (threshold > 0.0) ? (value / threshold) : 0.0;
 if (ratio < 0.6) return DiagHealthLevel::Healthy;
 if (ratio < 0.8) return DiagHealthLevel::Warning;
 if (ratio < 0.95) return DiagHealthLevel::Degraded;
 return DiagHealthLevel::Critical;
}

void DiagnosticDashboard::RecordMetric(const std::wstring& name, MetricCategory cat,
 double value, double threshold) {
 MetricPoint mp;
 mp.name = name;
 mp.category = cat;
 mp.value = value;
 mp.threshold = threshold;
 mp.health = EvaluateHealth(value, threshold);
 auto now = std::chrono::system_clock::now();
 mp.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
 now.time_since_epoch()).count();
 m_metrics.push_back(std::move(mp));
}

HealthSnapshot DiagnosticDashboard::GetSnapshot() const {
 HealthSnapshot snap;
 snap.metricCount = static_cast<uint32_t>(m_metrics.size());
 DiagHealthLevel worst = DiagHealthLevel::Healthy;
 for (const auto& m : m_metrics) {
 if (m.health == DiagHealthLevel::Warning) snap.warningCount++;
 if (m.health == DiagHealthLevel::Critical) snap.criticalCount++;
 if (static_cast<uint32_t>(m.health) > static_cast<uint32_t>(worst))
 worst = m.health;
 }
 snap.overall = worst;
 return snap;
}

void DiagnosticDashboard::Reset() {
 m_metrics.clear();
}

}} // namespace ExplorerLens::Engine
