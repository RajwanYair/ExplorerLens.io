// ThumbnailBenchmarks.cpp — Google Benchmark stubs for decode latency targets
// Copyright (c) 2026 ExplorerLens Project
//
// Defines BM_ fixture stubs aligned to baseline.json targets. These stubs
// compile and register benchmarks without pulling in Engine headers; the
// actual decode calls are replaced with controlled-latency no-ops so the
// fixture infrastructure can be validated in CI without real image data.
//
// Performance targets (from data/benchmarks/baseline.json v39.9.0):
//   BM_SingleDecode_JPEG     p50 ≤ 4.2 ms
//   BM_SingleDecode_PNG      p50 ≤ 5.8 ms
//   BM_SingleDecode_WebP     p50 ≤ 6.3 ms
//   BM_SingleDecode_AVIF     p50 ≤ 9.1 ms
//   BM_SingleDecode_JXLOSSLESS p50 ≤ 11.2 ms
//   BM_BatchDecode_1000      throughput ≥ 235 img/sec
//   BM_CacheHit_L1           p50 < 0.5 ms
//   BM_CacheHit_L2_Disk      p50 < 5.0 ms
//
// To run against real Engine decode:
//   1. Replace stub implementations with actual Engine calls
//   2. Set EL_TEST_DATA_DIR environment variable to corpus directory
//   3. Build with: cmake -DBUILD_GBENCHMARKS=ON
//   4. Run:        build/bin/EngineGBenchmarks --benchmark_out=results.json
//
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <benchmark/benchmark.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <thread>

// ---------------------------------------------------------------------------
// Stub decode targets (replace with real Engine calls for production runs)
// ---------------------------------------------------------------------------

namespace ExplorerLens::Bench {

/// Simulates the decode pipeline for a given format and size.
/// In a real run, this would call Engine::DecodeThumbnail() and measure wall time.
inline void StubDecode(std::string_view format, int widthPx, int heightPx) {
    // Simulate pixel buffer fill — exercises memory bandwidth proportionally
    const size_t pixels = static_cast<size_t>(widthPx) * static_cast<size_t>(heightPx);
    const size_t byteCount = pixels * 4; // BGRA32
    // Volatile prevents the compiler from eliminating the work entirely
    volatile uint8_t dummy = 0;
    for (size_t i = 0; i < (byteCount >> 12); ++i) {
        dummy = static_cast<uint8_t>(i & 0xFF);
    }
    benchmark::DoNotOptimize(dummy);
    benchmark::DoNotOptimize(format.data());
}

/// Simulates a cache hit (L1 LRU in-memory path).
inline void StubCacheHit() {
    // Cache hit: memory lookup only — no decode work
    static std::array<uint8_t, 64> fakeEntry{};
    benchmark::DoNotOptimize(fakeEntry.data());
}

/// Simulates a cache hit from the L2 disk cache.
inline void StubCacheHitL2() {
    // L2 cache: minimal disk-read simulation
    volatile uint64_t sum = 0;
    for (int i = 0; i < 16; ++i) sum += static_cast<uint64_t>(i * 3);
    benchmark::DoNotOptimize(sum);
}

} // namespace ExplorerLens::Bench

// ---------------------------------------------------------------------------
// Single-decode benchmarks — aligned to baseline.json targets
// ---------------------------------------------------------------------------

// JPEG 6 MP (3264×1836) — target p50 ≤ 4.2 ms
static void BM_SingleDecode_JPEG(benchmark::State& state) {
    for (auto _ : state) {
        ExplorerLens::Bench::StubDecode("jpeg", 3264, 1836);
    }
    state.SetLabel("format=JPEG pixels=5992704");
    state.counters["target_p50_ms"] = 4.2;
}
BENCHMARK(BM_SingleDecode_JPEG)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(100)
    ->Name("BM_SingleDecode_JPEG");

// PNG 4K (3840×2160) — target p50 ≤ 5.8 ms
static void BM_SingleDecode_PNG(benchmark::State& state) {
    for (auto _ : state) {
        ExplorerLens::Bench::StubDecode("png", 3840, 2160);
    }
    state.SetLabel("format=PNG pixels=8294400");
    state.counters["target_p50_ms"] = 5.8;
}
BENCHMARK(BM_SingleDecode_PNG)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(100)
    ->Name("BM_SingleDecode_PNG");

// WebP lossy 2K (2560×1440) — target p50 ≤ 6.3 ms
static void BM_SingleDecode_WebP(benchmark::State& state) {
    for (auto _ : state) {
        ExplorerLens::Bench::StubDecode("webp", 2560, 1440);
    }
    state.SetLabel("format=WebP pixels=3686400");
    state.counters["target_p50_ms"] = 6.3;
}
BENCHMARK(BM_SingleDecode_WebP)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(100)
    ->Name("BM_SingleDecode_WebP");

// AVIF (dav1d) 2K (2560×1440) — target p50 ≤ 9.1 ms
static void BM_SingleDecode_AVIF(benchmark::State& state) {
    for (auto _ : state) {
        ExplorerLens::Bench::StubDecode("avif", 2560, 1440);
    }
    state.SetLabel("format=AVIF pixels=3686400");
    state.counters["target_p50_ms"] = 9.1;
}
BENCHMARK(BM_SingleDecode_AVIF)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(50)
    ->Name("BM_SingleDecode_AVIF");

// JPEG XL lossless (2560×1440) — target p50 ≤ 11.2 ms
static void BM_SingleDecode_JXLOSSLESS(benchmark::State& state) {
    for (auto _ : state) {
        ExplorerLens::Bench::StubDecode("jxl", 2560, 1440);
    }
    state.SetLabel("format=JPEG-XL pixels=3686400");
    state.counters["target_p50_ms"] = 11.2;
}
BENCHMARK(BM_SingleDecode_JXLOSSLESS)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(50)
    ->Name("BM_SingleDecode_JXLOSSLESS");

// Batch decode throughput — 1000 images at 320×240 — target ≥ 235 img/sec
static void BM_BatchDecode_1000(benchmark::State& state) {
    const int batchSize = static_cast<int>(state.range(0));
    for (auto _ : state) {
        for (int i = 0; i < batchSize; ++i) {
            ExplorerLens::Bench::StubDecode("jpeg", 320, 240);
        }
    }
    state.SetItemsProcessed(static_cast<int64_t>(state.iterations()) * batchSize);
    state.SetLabel("target_throughput_img_sec=235");
    state.counters["img_sec"] = benchmark::Counter(
        static_cast<double>(state.iterations()) * batchSize,
        benchmark::Counter::kIsRate);
}
BENCHMARK(BM_BatchDecode_1000)
    ->Unit(benchmark::kMillisecond)
    ->Arg(1000)
    ->Name("BM_BatchDecode_1000");

// L1 cache hit (in-memory LRU) — target p50 < 0.5 ms
static void BM_CacheHit_L1(benchmark::State& state) {
    for (auto _ : state) {
        ExplorerLens::Bench::StubCacheHit();
    }
    state.SetLabel("path=L1-LRU");
    state.counters["target_p50_ms"] = 0.5;
}
BENCHMARK(BM_CacheHit_L1)
    ->Unit(benchmark::kMicrosecond)
    ->Iterations(10000)
    ->Name("BM_CacheHit_L1");

// L2 disk cache hit — target p50 < 5.0 ms
static void BM_CacheHit_L2_Disk(benchmark::State& state) {
    for (auto _ : state) {
        ExplorerLens::Bench::StubCacheHitL2();
    }
    state.SetLabel("path=L2-disk");
    state.counters["target_p50_ms"] = 5.0;
}
BENCHMARK(BM_CacheHit_L2_Disk)
    ->Unit(benchmark::kMicrosecond)
    ->Iterations(1000)
    ->Name("BM_CacheHit_L2_Disk");

// ---------------------------------------------------------------------------
// Parameterised format decode sweep (used by perf regression gate)
// ---------------------------------------------------------------------------

static void BM_SingleDecode_FormatSweep(benchmark::State& state) {
    static constexpr std::array<std::pair<std::string_view, int>, 6> FORMATS = {{
        { "jpeg",  4 },  // rough decode cost rank (lower = faster)
        { "png",   5 },
        { "webp",  6 },
        { "avif",  9 },
        { "jxl",  11 },
        { "heic",  8 },
    }};
    const int idx = static_cast<int>(state.range(0));
    if (idx < 0 || idx >= static_cast<int>(FORMATS.size())) {
        state.SkipWithError("Invalid format index");
        return;
    }
    auto [fmt, cost] = FORMATS[static_cast<size_t>(idx)];
    for (auto _ : state) {
        ExplorerLens::Bench::StubDecode(fmt, 2560, 1440);
    }
    state.SetLabel(std::string("format=") + std::string(fmt));
}
BENCHMARK(BM_SingleDecode_FormatSweep)
    ->Unit(benchmark::kMillisecond)
    ->DenseRange(0, 5)
    ->Name("BM_SingleDecode_FormatSweep");

BENCHMARK_MAIN();
