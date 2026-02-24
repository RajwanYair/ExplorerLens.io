#include "PerformanceBenchmarkV2.h"
#include <algorithm>
#include <numeric>
#include <cmath>

namespace ExplorerLens { namespace Engine {

PerformanceBenchmarkV2::PerformanceBenchmarkV2() = default;

const wchar_t* PerformanceBenchmarkV2::GetBenchmarkTypeName(BenchmarkType type) {
    switch (type) {
        case BenchmarkType::SingleDecode:  return L"Single Decode";
        case BenchmarkType::BatchDecode:   return L"Batch Decode";
        case BenchmarkType::CacheHit:      return L"Cache Hit";
        case BenchmarkType::GPURender:     return L"GPU Render";
        case BenchmarkType::FormatConvert: return L"Format Convert";
        case BenchmarkType::EndToEnd:      return L"End to End";
        default:                           return L"Unknown";
    }
}

BenchmarkResult PerformanceBenchmarkV2::ComputeStats(const std::wstring& label,
                                                      BenchmarkType type,
                                                      const std::vector<double>& samplesMs) {
    BenchmarkResult result;
    result.label = label;
    result.type = type;
    result.iterations = static_cast<uint32_t>(samplesMs.size());

    if (samplesMs.empty()) return result;

    auto sorted = samplesMs;
    std::sort(sorted.begin(), sorted.end());

    result.minMs = sorted.front();
    result.maxMs = sorted.back();

    double sum = std::accumulate(sorted.begin(), sorted.end(), 0.0);
    result.meanMs = sum / sorted.size();

    size_t mid = sorted.size() / 2;
    result.medianMs = (sorted.size() % 2 == 0)
        ? (sorted[mid - 1] + sorted[mid]) / 2.0
        : sorted[mid];

    size_t p95Idx = static_cast<size_t>(sorted.size() * 0.95);
    if (p95Idx >= sorted.size()) p95Idx = sorted.size() - 1;
    result.p95Ms = sorted[p95Idx];

    size_t p99Idx = static_cast<size_t>(sorted.size() * 0.99);
    if (p99Idx >= sorted.size()) p99Idx = sorted.size() - 1;
    result.p99Ms = sorted[p99Idx];

    double variance = 0.0;
    for (auto v : sorted) {
        double diff = v - result.meanMs;
        variance += diff * diff;
    }
    result.stdDevMs = std::sqrt(variance / sorted.size());

    return result;
}

void PerformanceBenchmarkV2::AddResult(const BenchmarkResult& result) {
    m_results.push_back(result);
}

void PerformanceBenchmarkV2::Reset() {
    m_results.clear();
}

bool PerformanceBenchmarkV2::MeetsTarget(const BenchmarkResult& result, double targetP95Ms) {
    return result.p95Ms <= targetP95Ms;
}

}} // namespace ExplorerLens::Engine

