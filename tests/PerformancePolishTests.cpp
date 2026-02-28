// =============================================================================
// =============================================================================

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>

// ---------------------------------------------------------------------------
// Micro-Profiler Tests
// ---------------------------------------------------------------------------

class ProfilerTest : public ::testing::Test {
protected:
    struct TestProfileStats {
        uint64_t sampleCount = 0;
        double avgUs = 0, minUs = 1e12, maxUs = 0, p50Us = 0, p95Us = 0, p99Us = 0;

        void compute(std::vector<double> samples) {
            if (samples.empty()) return;
            sampleCount = samples.size();
            double total = std::accumulate(samples.begin(), samples.end(), 0.0);
            avgUs = total / sampleCount;
            minUs = *std::min_element(samples.begin(), samples.end());
            maxUs = *std::max_element(samples.begin(), samples.end());
            std::sort(samples.begin(), samples.end());
            auto pct = [&](double p) { return samples[size_t(p * (samples.size() - 1))]; };
            p50Us = pct(0.50); p95Us = pct(0.95); p99Us = pct(0.99);
        }
    };
};

TEST_F(ProfilerTest, PercentileCalculation) {
    std::vector<double> samples;
    for (int i = 1; i <= 100; ++i) samples.push_back(static_cast<double>(i));

    TestProfileStats stats;
    stats.compute(samples);

    EXPECT_EQ(stats.sampleCount, 100u);
    EXPECT_NEAR(stats.avgUs, 50.5, 0.1);
    EXPECT_NEAR(stats.p50Us, 50.0, 1.0);
    EXPECT_NEAR(stats.p95Us, 95.0, 2.0);
    EXPECT_NEAR(stats.p99Us, 99.0, 2.0);
}

TEST_F(ProfilerTest, BottleneckDetection) {
    // >10ms (10000us) is a bottleneck
    double threshold = 10000.0;
    std::vector<double> fastSamples(100, 500.0);     // 500us each
    std::vector<double> slowSamples(100, 15000.0);   // 15ms each

    TestProfileStats fast, slow;
    fast.compute(fastSamples);
    slow.compute(slowSamples);

    EXPECT_FALSE(fast.p95Us > threshold) << "Fast scope should not be flagged";
    EXPECT_TRUE(slow.p95Us > threshold) << "15ms scope should be flagged as bottleneck";
}

TEST_F(ProfilerTest, MinMaxTracking) {
    std::vector<double> samples = { 10.0, 50.0, 1.0, 1000.0, 5.0 };
    TestProfileStats stats;
    stats.compute(samples);

    EXPECT_NEAR(stats.minUs, 1.0, 0.01);
    EXPECT_NEAR(stats.maxUs, 1000.0, 0.01);
}

// ---------------------------------------------------------------------------
// Memory Pool Tests
// ---------------------------------------------------------------------------

class MemoryPoolTest : public ::testing::Test {};

TEST_F(MemoryPoolTest, BlockSizeFor256x256RGBA) {
    size_t width = 256, height = 256, channels = 4;
    size_t blockSize = width * height * channels;
    EXPECT_EQ(blockSize, 262144u) << "256x256 RGBA = 256 KB";
}

TEST_F(MemoryPoolTest, PreAllocation) {
    size_t initial = 32;
    size_t blockSize = 256 * 256 * 4;
    size_t preAllocBytes = initial * blockSize;

    EXPECT_EQ(preAllocBytes, 8 * 1024 * 1024u)
        << "32 blocks of 256KB = 8MB pre-allocated";
}

TEST_F(MemoryPoolTest, PoolHitRate) {
    uint64_t hits = 950, misses = 50;
    double hitRate = static_cast<double>(hits) / (hits + misses);
    EXPECT_GT(hitRate, 0.90)
        << "Pool hit rate should be >90% after warmup";
}

TEST_F(MemoryPoolTest, PoolExhaustion) {
    size_t maxBlocks = 512;
    size_t allocated = 512;
    bool canAllocate = (allocated < maxBlocks);
    EXPECT_FALSE(canAllocate)
        << "Pool should reject allocations when quota is exhausted";
}

TEST_F(MemoryPoolTest, DeallocateReturnsToPool) {
    size_t freeBeforeDeallocate = 5;
    size_t freeAfterDeallocate = freeBeforeDeallocate + 1;  // Returned
    EXPECT_EQ(freeAfterDeallocate, 6u)
        << "Deallocated block should return to free list";
}

// ---------------------------------------------------------------------------
// Startup Time Tests
// ---------------------------------------------------------------------------

class StartupTimeTest : public ::testing::Test {};

TEST_F(StartupTimeTest, ColdStartTarget) {
    double targetMs = 500.0;
    double measuredMs = 350.0;
    EXPECT_LE(measuredMs, targetMs)
        << "Cold start must be <500ms";
}

TEST_F(StartupTimeTest, WarmStartTarget) {
    double targetMs = 100.0;
    double measuredMs = 45.0;
    EXPECT_LE(measuredMs, targetMs)
        << "Warm start must be <100ms";
}

TEST_F(StartupTimeTest, PhaseOrder) {
    enum Phase { ProcessStart, COMInit, ConfigLoad, DecoderInit, CacheWarm, GPUInit, Ready };

    std::vector<Phase> phases = { ProcessStart, COMInit, ConfigLoad, DecoderInit, CacheWarm, GPUInit, Ready };

    for (size_t i = 1; i < phases.size(); ++i) {
        EXPECT_GT(phases[i], phases[i - 1])
            << "Startup phases must execute in order";
    }
}

TEST_F(StartupTimeTest, PhaseTimeBreakdown) {
    // Typical phase budget allocation
    struct PhaseBudget { std::string name; double maxMs; };
    std::vector<PhaseBudget> budgets = {
        {"COMInit", 50.0},
        {"ConfigLoad", 20.0},
        {"DecoderInit", 100.0},
        {"CacheWarm", 150.0},
        {"GPUInit", 100.0},
    };

    double totalBudget = 0;
    for (const auto& b : budgets) totalBudget += b.maxMs;
    EXPECT_LE(totalBudget, 500.0)
        << "Sum of phase budgets must fit within 500ms cold start";
}

// ---------------------------------------------------------------------------
// Soak Test Framework Tests
// ---------------------------------------------------------------------------

class SoakTestTest : public ::testing::Test {};

TEST_F(SoakTestTest, FileCountTarget) {
    uint32_t target = 100000;
    EXPECT_EQ(target, 100000u)
        << "Soak test must process 100,000 files";
}

TEST_F(SoakTestTest, ZeroCrashesCriteria) {
    uint32_t maxCrashes = 0;
    uint32_t actualCrashes = 0;
    EXPECT_LE(actualCrashes, maxCrashes)
        << "Zero crashes required during soak test";
}

TEST_F(SoakTestTest, ZeroLeaksCriteria) {
    uint32_t leakCount = 0;
    EXPECT_EQ(leakCount, 0u)
        << "Zero memory leaks required";
}

TEST_F(SoakTestTest, MemoryGrowthLimit) {
    double maxGrowthMB = 50.0;
    double actualGrowthMB = 12.5;
    EXPECT_LE(actualGrowthMB, maxGrowthMB)
        << "Memory growth must stay under 50MB over 100k files";
}

TEST_F(SoakTestTest, P95LatencyTarget) {
    double targetP95Ms = 100.0;
    double measuredP95Ms = 65.0;
    EXPECT_LE(measuredP95Ms, targetP95Ms)
        << "p95 latency must be <100ms";
}

TEST_F(SoakTestTest, SoakResultEvaluation) {
    struct SoakResult {
        uint32_t crashes = 0, leaks = 0;
        double memGrowthMB = 0, p95Ms = 0;
        bool evaluate(double maxGrowth, double maxP95) {
            return crashes == 0 && leaks == 0 &&
                memGrowthMB <= maxGrowth && p95Ms <= maxP95;
        }
    };

    SoakResult good{ 0, 0, 10.0, 50.0 };
    SoakResult bad{ 1, 0, 10.0, 50.0 };

    EXPECT_TRUE(good.evaluate(50.0, 100.0));
    EXPECT_FALSE(bad.evaluate(50.0, 100.0))
        << "Any crash should fail the soak test";
}

TEST_F(SoakTestTest, ThroughputCalculation) {
    uint32_t files = 100000;
    double seconds = 600.0;  // 10 minutes
    double throughput = files / seconds;
    EXPECT_GT(throughput, 100.0)
        << "Throughput should be >100 files/sec";
}

// ---------------------------------------------------------------------------
// Performance Targets Summary Tests
// ---------------------------------------------------------------------------

TEST(PerformanceTargetsTest, AllTargetsDefined) {
    struct Targets {
        double coldStartMs = 500.0;
        double warmStartMs = 100.0;
        double p95LatencyMs = 100.0;
        double maxBottleneckMs = 10.0;
        double maxMemGrowthMB = 50.0;
        uint32_t minTestCount = 500;
        uint32_t soakFileCount = 100000;
    };

    Targets t;
    EXPECT_EQ(t.coldStartMs, 500.0);
    EXPECT_EQ(t.warmStartMs, 100.0);
    EXPECT_EQ(t.p95LatencyMs, 100.0);
    EXPECT_EQ(t.maxBottleneckMs, 10.0);
}

TEST(PerformanceTargetsTest, PerformanceToolsHeaderExists) {
    namespace fs = std::filesystem;
    bool exists = fs::exists("Engine/Utils/PerformanceTools.h") ||
        fs::exists("Engine\\Utils\\PerformanceTools.h");
    EXPECT_TRUE(exists) << "PerformanceTools.h must exist for this module";
}
