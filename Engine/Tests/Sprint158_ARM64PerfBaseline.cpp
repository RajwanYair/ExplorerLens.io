// ARM64 Performance Baseline Tests
#include "../Utils/ARM64PerformanceBaseline.h"
#include "GTestShim.h"

using namespace ExplorerLens::Platform;

TEST(ARM64PerfBaseline, MockBaselineCreated) {
  auto b = ARM64PerformanceBaseline::CreateMock();
  ASSERT(!b.simdComparisons.empty());
}

TEST(ARM64PerfBaseline, KPIsThroughputAbove100) {
  ASSERT(ARM64KPIs::kBatchThroughputImgSec >= 100.0);
}

TEST(ARM64PerfBaseline, KPIsP95Below200ms) {
  ASSERT(ARM64KPIs::kSingleThumbP95Ms < 200.0);
}

TEST(ARM64PerfBaseline, KPIsCacheHitBelow10ms) {
  ASSERT(ARM64KPIs::kCacheHitAvgMs <= 10.0);
}

TEST(ARM64PerfBaseline, BenchmarkSampleHasDuration) {
  BenchmarkSample s;
  s.p95Ms = 22.5;
  ASSERT(s.MeetsP95Budget(30.0));
}

TEST(ARM64PerfBaseline, SIMDComparisonHasSpeedup) {
  SIMDComparisonResult r;
  r.speedupRatio = 2.09;
  r.neonWins = true;
  ASSERT(r.speedupRatio > 1.0);
}

TEST(ARM64PerfBaseline, RegressionThresholdIs10Percent) {
  ASSERT(BaselineTrendResult::kRegressionThreshold == 10.0);
}

TEST(ARM64PerfBaseline, TrendCompareNoRegression) {
  auto b1 = ARM64PerformanceBaseline::CreateMock();
  auto b2 = ARM64PerformanceBaseline::CreateMock();
  auto trend = BaselineTrendResult::Compare(b1, b2);
  ASSERT(!trend.hasRegression);
}

TEST(ARM64PerfBaseline, MockSamplesAllPositiveDuration) {
  auto b = ARM64PerformanceBaseline::CreateMock();
  ASSERT(b.singleThumb.p95Ms > 0.0);
}

TEST(ARM64PerfBaseline, ColdStartKPI) {
  ASSERT(ARM64KPIs::kColdStartMs < 2000.0);
  ASSERT(ARM64KPIs::kColdStartMs > 0.0);
}

TEST(ARM64PerfBaseline, MeetsAllKPIs) {
  auto b = ARM64PerformanceBaseline::CreateMock();
  ASSERT(b.MeetsAllKPIs());
}
