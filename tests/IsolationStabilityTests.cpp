//==============================================================================
// ExplorerLens — Worker/Isolation Stabilization
// Tests SEH fuzzing engine, circuit breaker hardening, decoder timeout
// enforcement, memory leak regression, and worker isolation monitoring.
//==============================================================================

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
#include <functional>

// Headers under test
#include "../Engine/Utils/FuzzingEngine.h"
#include "../Engine/Utils/DecoderCircuitBreaker.h"

using namespace ExplorerLens::Engine::Isolation;
using namespace ExplorerLens;

//==============================================================================
// Corruption Strategy Tests
//==============================================================================

TEST(CorruptionStrategy, StrategyNamesValid) {
    for (uint32_t i = 0; i < static_cast<uint32_t>(CorruptionStrategy::MaxEnum); ++i) {
        auto s = static_cast<CorruptionStrategy>(i);
        const char* name = CorruptionStrategyName(s);
        ASSERT_NE(name, nullptr);
        EXPECT_STRNE(name, "Unknown");
    }
}

TEST(CorruptionStrategy, UnknownReturnsUnknown) {
    auto name = CorruptionStrategyName(static_cast<CorruptionStrategy>(99));
    EXPECT_STREQ(name, "Unknown");
}

TEST(CorruptionStrategy, StrategyCount) {
    EXPECT_EQ(static_cast<uint32_t>(CorruptionStrategy::MaxEnum), 8u);
}

//==============================================================================
// Magic Bytes Tests
//==============================================================================

TEST(FormatMagic, KnownMagicBytesNotEmpty) {
    auto magics = GetKnownMagicBytes();
    EXPECT_GE(magics.size(), 15u);
}

TEST(FormatMagic, AllFormatsHaveMagicBytes) {
    for (const auto& m : GetKnownMagicBytes()) {
        EXPECT_NE(m.extension, nullptr);
        EXPECT_GT(m.extension[0], '\0') << "Empty extension";
        EXPECT_FALSE(m.magic.empty()) << "Missing magic for " << m.extension;
        EXPECT_GT(m.minSize, 0u) << "Zero minSize for " << m.extension;
    }
}

TEST(FormatMagic, ZipMagicCorrect) {
    auto magics = GetKnownMagicBytes();
    bool found = false;
    for (auto& m : magics) {
        if (std::string(m.extension) == ".zip") {
            EXPECT_EQ(m.magic[0], 0x50);
            EXPECT_EQ(m.magic[1], 0x4B);
            found = true;
        }
    }
    EXPECT_TRUE(found) << ".zip not in magic bytes table";
}

TEST(FormatMagic, PngMagicCorrect) {
    auto magics = GetKnownMagicBytes();
    bool found = false;
    for (auto& m : magics) {
        if (std::string(m.extension) == ".png") {
            EXPECT_EQ(m.magic.size(), 8u);
            EXPECT_EQ(m.magic[0], 0x89);
            EXPECT_EQ(m.magic[1], 0x50);
            found = true;
        }
    }
    EXPECT_TRUE(found);
}

//==============================================================================
// Corrupt Payload Generator Tests
//==============================================================================

TEST(CorruptPayload, GenerateZeroFill) {
    CorruptPayloadGenerator gen(42);
    auto payload = gen.Generate(CorruptionStrategy::ZeroFill, ".zip", 256);
    EXPECT_EQ(payload.size(), 256u);
    for (auto b : payload) EXPECT_EQ(b, 0x00);
}

TEST(CorruptPayload, GenerateRandomBytes) {
    CorruptPayloadGenerator gen(42);
    auto p1 = gen.Generate(CorruptionStrategy::RandomBytes, ".png", 512);
    EXPECT_EQ(p1.size(), 512u);
    bool hasNonZero = false;
    for (auto b : p1) if (b != 0) hasNonZero = true;
    EXPECT_TRUE(hasNonZero);
}

TEST(CorruptPayload, GenerateValidHeaderGarbage) {
    CorruptPayloadGenerator gen(42);
    auto payload = gen.Generate(CorruptionStrategy::ValidHeaderGarbage, ".zip", 1024);
    EXPECT_EQ(payload.size(), 1024u);
    EXPECT_EQ(payload[0], 0x50);
    EXPECT_EQ(payload[1], 0x4B);
    EXPECT_EQ(payload[2], 0x03);
    EXPECT_EQ(payload[3], 0x04);
}

TEST(CorruptPayload, GenerateTruncated) {
    CorruptPayloadGenerator gen(42);
    auto payload = gen.Generate(CorruptionStrategy::Truncated, ".png", 1024);
    EXPECT_LT(payload.size(), 1024u);
    EXPECT_GT(payload.size(), 0u);
}

TEST(CorruptPayload, GenerateBitFlip) {
    CorruptPayloadGenerator gen(42);
    auto payload = gen.Generate(CorruptionStrategy::BitFlip, ".jpg", 512);
    EXPECT_EQ(payload.size(), 512u);
}

TEST(CorruptPayload, GenerateOversizeField) {
    CorruptPayloadGenerator gen(42);
    auto payload = gen.Generate(CorruptionStrategy::OversizeField, ".bmp", 512);
    EXPECT_EQ(payload.size(), 512u);
    bool hasOversize = false;
    for (size_t i = 0; i + 3 < payload.size(); i += 4) {
        if (payload[i] == 0xFF && payload[i + 1] == 0xFF &&
            payload[i + 2] == 0xFF && payload[i + 3] == 0xFF) {
            hasOversize = true;
            break;
        }
    }
    EXPECT_TRUE(hasOversize);
}

TEST(CorruptPayload, GenerateRepeatingPattern) {
    CorruptPayloadGenerator gen(42);
    auto payload = gen.Generate(CorruptionStrategy::RepeatingPattern, ".tga", 256);
    EXPECT_EQ(payload.size(), 256u);
    uint32_t pattern = 0;
    std::memcpy(&pattern, payload.data(), 4);
    EXPECT_EQ(pattern, 0xDEADBEEFu);
}

TEST(CorruptPayload, RandomStrategyInRange) {
    CorruptPayloadGenerator gen(42);
    for (int i = 0; i < 100; ++i) {
        auto s = gen.RandomStrategy();
        EXPECT_LT(static_cast<uint32_t>(s),
            static_cast<uint32_t>(CorruptionStrategy::MaxEnum));
    }
}

TEST(CorruptPayload, RandomPayloadSizeInRange) {
    CorruptPayloadGenerator gen(42);
    for (int i = 0; i < 100; ++i) {
        auto sz = gen.RandomPayloadSize(128, 4096);
        EXPECT_GE(sz, 128u);
        EXPECT_LE(sz, 4096u);
    }
}

//==============================================================================
// SEH Isolation Wrapper Tests
//==============================================================================

TEST(SEHIsolation, SuccessfulExecution) {
    auto result = SEHIsolationWrapper::Execute([]() { return true; });
    EXPECT_TRUE(result.completed);
    EXPECT_FALSE(result.sehCaught);
    EXPECT_EQ(result.exceptionCode, 0u);
    EXPECT_GT(result.elapsedMs, 0.0);
}

TEST(SEHIsolation, FailedExecution) {
    auto result = SEHIsolationWrapper::Execute([]() { return false; });
    EXPECT_FALSE(result.completed);
    EXPECT_FALSE(result.sehCaught);
}

TEST(SEHIsolation, TimeMeasurement) {
    auto result = SEHIsolationWrapper::Execute([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return true;
        });
    EXPECT_GE(result.elapsedMs, 5.0);
}

//==============================================================================
// Worker Isolation Monitor Tests
//==============================================================================

TEST(WorkerMonitor, RegisterWorker) {
    WorkerIsolationMonitor monitor;
    monitor.RegisterWorker(1);
    monitor.RegisterWorker(2);
    EXPECT_EQ(monitor.WorkerCount(), 2u);
}

TEST(WorkerMonitor, RecordSuccess) {
    WorkerIsolationMonitor monitor;
    monitor.RegisterWorker(1);
    monitor.RecordSuccess(1);
    monitor.RecordSuccess(1);
    monitor.RecordSuccess(1);

    auto workers = monitor.GetAllWorkers();
    ASSERT_EQ(workers.size(), 1u);
    EXPECT_EQ(workers[0].totalRequests, 3u);
    EXPECT_EQ(workers[0].successfulDecodes, 3u);
    EXPECT_EQ(workers[0].sehExceptions, 0u);
    EXPECT_FALSE(workers[0].markedForRecycle);
}

TEST(WorkerMonitor, RecycleTriggerByCount) {
    WorkerIsolationMonitor monitor(5, 100.0);
    monitor.RegisterWorker(1);
    for (int i = 0; i < 5; ++i) monitor.RecordSEHException(1);
    EXPECT_TRUE(monitor.ShouldRecycle(1));
    EXPECT_EQ(monitor.RecycleCandidateCount(), 1u);
}

TEST(WorkerMonitor, RecycleTriggerByRate) {
    WorkerIsolationMonitor monitor(1000, 10.0);
    monitor.RegisterWorker(1);
    for (int i = 0; i < 10; ++i) monitor.RecordSuccess(1);
    for (int i = 0; i < 2; ++i) monitor.RecordSEHException(1);
    EXPECT_TRUE(monitor.ShouldRecycle(1));
}

TEST(WorkerMonitor, HealthyWorkerNotRecycled) {
    WorkerIsolationMonitor monitor(10, 5.0);
    monitor.RegisterWorker(1);
    for (int i = 0; i < 100; ++i) monitor.RecordSuccess(1);
    monitor.RecordSEHException(1);
    EXPECT_FALSE(monitor.ShouldRecycle(1));
}

TEST(WorkerMonitor, MultipleWorkerIndependence) {
    WorkerIsolationMonitor monitor(3, 100.0);
    monitor.RegisterWorker(1);
    monitor.RegisterWorker(2);
    for (int i = 0; i < 50; ++i) monitor.RecordSuccess(1);
    for (int i = 0; i < 3; ++i) monitor.RecordSEHException(2);
    EXPECT_FALSE(monitor.ShouldRecycle(1));
    EXPECT_TRUE(monitor.ShouldRecycle(2));
}

TEST(WorkerMonitor, TimeoutCountsAsFailure) {
    WorkerIsolationMonitor monitor(3, 100.0);
    monitor.RegisterWorker(1);
    for (int i = 0; i < 3; ++i) monitor.RecordTimeout(1);
    EXPECT_TRUE(monitor.ShouldRecycle(1));
}

//==============================================================================
// Fuzz Campaign Runner Tests
//==============================================================================

TEST(FuzzCampaign, RunWithNoDecoder) {
    FuzzCampaignRunner runner(42);
    auto report = runner.Run(10, { ".zip", ".png", ".jpg" });
    EXPECT_EQ(report.totalIterations, 30u);
    EXPECT_EQ(report.crashes, 0u);
    EXPECT_TRUE(report.OverallPass());
}

TEST(FuzzCampaign, RunWithSuccessDecoder) {
    FuzzCampaignRunner runner(42);
    runner.SetDecoderCallback([](const uint8_t*, size_t, const std::string&) {
        return true;
        });
    auto report = runner.Run(50, { ".webp" });
    EXPECT_EQ(report.totalIterations, 50u);
    EXPECT_EQ(report.crashes, 0u);
    EXPECT_TRUE(report.OverallPass());
}

TEST(FuzzCampaign, RunWithRejectDecoder) {
    FuzzCampaignRunner runner(42);
    runner.SetDecoderCallback([](const uint8_t*, size_t, const std::string&) {
        return false;
        });
    auto report = runner.Run(100, { ".heif" });
    EXPECT_EQ(report.totalIterations, 100u);
    EXPECT_EQ(report.crashes, 0u);
    EXPECT_EQ(report.gracefulRejects, 100u);
    EXPECT_DOUBLE_EQ(report.GracefulRate(), 100.0);
    EXPECT_TRUE(report.OverallPass());
}

TEST(FuzzCampaign, QuickSmokeTest) {
    FuzzCampaignRunner runner(42);
    runner.SetDecoderCallback([](const uint8_t*, size_t, const std::string&) {
        return false;
        });
    auto report = runner.QuickSmoke(500);
    EXPECT_EQ(report.totalIterations, 500u);
    EXPECT_EQ(report.crashes, 0u);
    EXPECT_GT(report.totalElapsedMs, 0.0);
    EXPECT_GT(report.avgIterationMs, 0.0);
    EXPECT_TRUE(report.OverallPass());
}

TEST(FuzzCampaign, SEHRateCalculation) {
    FuzzCampaignReport report;
    report.totalIterations = 1000;
    report.sehExceptions = 50;
    EXPECT_DOUBLE_EQ(report.SEHRate(), 5.0);
}

TEST(FuzzCampaign, SupportedFormatCount) {
    FuzzCampaignRunner runner;
    EXPECT_GE(runner.SupportedFormatCount(), 15u);
}

//==============================================================================
// FuzzResult Tests
//==============================================================================

TEST(FuzzResult, DefaultPass) {
    FuzzResult res{};
    EXPECT_TRUE(res.IsPass());
}

TEST(FuzzResult, CrashedFails) {
    FuzzResult res{};
    res.crashed = true;
    EXPECT_FALSE(res.IsPass());
}

//==============================================================================
// Circuit Breaker Integration Tests
//==============================================================================

TEST(CircuitBreakerS6, InitiallyAvailable) {
    DecoderCircuitBreaker cb("TestDecoder");
    EXPECT_TRUE(cb.IsAvailable());
}

TEST(CircuitBreakerS6, OpensAfterConsecutiveFailures) {
    DecoderCircuitBreaker cb("TestDecoder");
    for (int i = 0; i < 5; ++i) cb.RecordFailure();
    EXPECT_FALSE(cb.IsAvailable());
}

TEST(CircuitBreakerS6, SuccessResetsFailureCount) {
    DecoderCircuitBreaker cb("TestDecoder");
    cb.RecordFailure();
    cb.RecordFailure();
    cb.RecordSuccess();
    cb.RecordFailure();
    cb.RecordFailure();
    EXPECT_TRUE(cb.IsAvailable());
}

TEST(CircuitBreakerS6, StressTest5000) {
    DecoderCircuitBreaker cb("StressDecoder");
    int openCount = 0;
    for (int i = 0; i < 5000; ++i) {
        if (i % 100 == 0) {
            cb.RecordFailure();
        }
        else {
            cb.RecordSuccess();
        }
        if (!cb.IsAvailable()) openCount++;
    }
    EXPECT_LT(openCount, 100) << "Circuit stayed open too often under mixed load";
}

//==============================================================================
// Memory Leak Regression Tests
//==============================================================================

TEST(MemoryLeak, CaptureSnapshot) {
    auto snap = MemoryLeakRegressionHarness::CaptureSnapshot();
    EXPECT_GT(snap.workingSetBytes, 0u);
    EXPECT_GT(snap.privateBytes, 0u);
}

TEST(MemoryLeak, NoLeakInTightLoop) {
    auto result = MemoryLeakRegressionHarness::RunLeakTest([]() {
        volatile int x = 42;
        (void)x;
        }, 100);
    EXPECT_EQ(result.iterations, 100u);
    EXPECT_TRUE(result.passedLeakCheck)
        << "Growth per iteration: " << result.growthPerIterKB << " KB";
}

TEST(MemoryLeak, ThresholdConstant) {
    EXPECT_EQ(MemoryLeakRegressionHarness::LeakTestResult::MAX_GROWTH_PER_ITER_KB,
        1.0);
}

//==============================================================================
// Campaign Report Tests
//==============================================================================

TEST(CampaignReport, OverallPassWhenZeroCrashes) {
    FuzzCampaignReport report;
    report.totalIterations = 10000;
    report.crashes = 0;
    EXPECT_TRUE(report.OverallPass());
}

TEST(CampaignReport, OverallFailWhenCrashes) {
    FuzzCampaignReport report;
    report.totalIterations = 10000;
    report.crashes = 1;
    EXPECT_FALSE(report.OverallPass());
}

TEST(CampaignReport, ZeroIterationsNoDiv) {
    FuzzCampaignReport report;
    report.totalIterations = 0;
    EXPECT_DOUBLE_EQ(report.SEHRate(), 0.0);
    EXPECT_DOUBLE_EQ(report.GracefulRate(), 0.0);
}
