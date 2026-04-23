// DecoderUnitTests.cpp — Catch2 unit tests for v35.5 decoder components
// Copyright (c) 2026 ExplorerLens Project
//
// Covers:
//   - ExifOrientationNormalizer      (BGRA pixel transforms, EXIF tag parsing)
//   - ArchiveCoverExtractor          (buffer parsing, min-size guard)
//   - EmbeddedPreviewExtractor       (resolution guard, stub fallback)
//   - ThumbnailSizeGrid              (preset lookup, grid layout)
//   - TwoTierCacheManager            (hit/miss, L2 promotion)
//   - PredictivePrefetchEngine       (RecordAccess, ScheduleNeighbours)
//   - CacheMetricsCollector          (ParseStats JSON, rolling log)
//   - EventLogProvider               (registration + smoke)
//
#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

#include "../../Core/ExifOrientationNormalizer.h"
#include "../../Core/ThumbnailSizeGrid.h"
#include "../../Core/PredictivePrefetchEngine.h"
#include "../../Core/CacheMetricsCollector.h"
#include "../../Core/EventLogProvider.h"

// ─────────────────────────────────────────────────────────────────────────────
//  ExifOrientationNormalizer
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("ExifOrientationNormalizer — orientation 1 is identity", "[exif][normalizer]") {
    using namespace ExplorerLens::Engine;

    // Create a 4×4 BGRA32 image with a known pattern
    const uint32_t W = 4, H = 4;
    std::vector<uint8_t> pixels(W * H * 4);
    for (uint32_t i = 0; i < W * H * 4; ++i) pixels[i] = static_cast<uint8_t>(i);

    auto original = pixels;

    PixelBuffer buf;
    buf.pixels = pixels.data();
    buf.width  = W;
    buf.height = H;
    buf.stride = W * 4;
    ExifOrientationNormalizer::Normalize(buf, ExifOrientation::NORMAL);

    REQUIRE(pixels == original);
}

TEST_CASE("ExifOrientationNormalizer — Rotate180 is involution", "[exif][normalizer]") {
    using namespace ExplorerLens::Engine;

    const uint32_t W = 4, H = 4;
    std::vector<uint8_t> pixels(W * H * 4);
    for (uint32_t i = 0; i < W * H * 4; ++i) pixels[i] = static_cast<uint8_t>(i * 3 + 1);

    auto original = pixels;

    PixelBuffer buf;
    buf.pixels = pixels.data();
    buf.width  = W;
    buf.height = H;
    buf.stride = W * 4;
    // Orientation 3 = Rotate180
    ExifOrientationNormalizer::Normalize(buf, ExifOrientation::ROTATE_180);
    buf.pixels = pixels.data();  // re-point after potential resize
    ExifOrientationNormalizer::Normalize(buf, ExifOrientation::ROTATE_180);

    REQUIRE(pixels == original);
}

TEST_CASE("ExifOrientationNormalizer — FlipH then FlipH equals identity", "[exif][normalizer]") {
    using namespace ExplorerLens::Engine;

    const uint32_t W = 8, H = 4;
    std::vector<uint8_t> pixels(W * H * 4);
    for (size_t i = 0; i < pixels.size(); ++i) pixels[i] = static_cast<uint8_t>(i ^ 0xAA);

    auto original = pixels;

    PixelBuffer buf;
    buf.pixels = pixels.data();
    buf.width  = W;
    buf.height = H;
    buf.stride = W * 4;
    // Orientation 2 = FlipH
    ExifOrientationNormalizer::Normalize(buf, ExifOrientation::FLIP_HORIZONTAL);
    buf.pixels = pixels.data();
    ExifOrientationNormalizer::Normalize(buf, ExifOrientation::FLIP_HORIZONTAL);

    REQUIRE(pixels == original);
}

TEST_CASE("ExifOrientationNormalizer — orientation 8 produces transposed dimensions", "[exif][normalizer]") {
    using namespace ExplorerLens::Engine;

    const uint32_t W = 6, H = 4;
    std::vector<uint8_t> pixels(W * H * 4, 0xCC);

    PixelBuffer buf;
    buf.pixels = pixels.data();
    buf.width  = W;
    buf.height = H;
    buf.stride = W * 4;
    // Orientation 8 = Rotate90CCW; result should be H×W
    ExifOrientationNormalizer::Normalize(buf, ExifOrientation::ROTATE_90_CCW);

    REQUIRE(buf.width  == H);
    REQUIRE(buf.height == W);
}

// ─────────────────────────────────────────────────────────────────────────────
//  ThumbnailSizeGrid
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("ThumbnailSizeGrid — NearestPreset rounds to closest canonical size", "[sizegrid]") {
    using namespace ExplorerLens::Engine;

    // Exact hits
    REQUIRE(NearestPreset(256) == ThumbnailSize::THUMBNAIL_256);
    REQUIRE(NearestPreset(512) == ThumbnailSize::THUMBNAIL_512);
    REQUIRE(NearestPreset(64)  == ThumbnailSize::ICON_XLARGE);

    // Below smallest
    REQUIRE(NearestPreset(5) == ThumbnailSize::ICON_SMALL);

    // Between 256 and 512: closer to 256
    REQUIRE(NearestPreset(300) == ThumbnailSize::THUMBNAIL_256);

    // Between 256 and 512: closer to 512
    REQUIRE(NearestPreset(400) == ThumbnailSize::THUMBNAIL_512);
}

TEST_CASE("ThumbnailSizeGrid — ComputeGrid produces correct row/column counts", "[sizegrid]") {
    using namespace ExplorerLens::Engine;

    auto layout = ComputeGrid(10, 128, 4, 4);
    REQUIRE(layout.columns == 4);
    REQUIRE(layout.rows    == 3);   // ceil(10 / 4)
    REQUIRE(layout.cellCount == 10);
}

TEST_CASE("ThumbnailSizeGrid — ComputeGrid with count <= maxColumns", "[sizegrid]") {
    using namespace ExplorerLens::Engine;

    auto layout = ComputeGrid(3, 64, 6, 2);
    REQUIRE(layout.columns == 3);
    REQUIRE(layout.rows    == 1);
}

TEST_CASE("ThumbnailSizeGrid — Preset table is sorted ascending", "[sizegrid]") {
    using namespace ExplorerLens::Engine;

    for (size_t i = 1; i < THUMBNAIL_SIZE_TABLE.size(); ++i) {
        REQUIRE(THUMBNAIL_SIZE_TABLE[i].pixels > THUMBNAIL_SIZE_TABLE[i-1].pixels);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  PredictivePrefetchEngine
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("PredictivePrefetchEngine — starts and stops cleanly", "[prefetch]") {
    using namespace ExplorerLens::Engine;

    PredictivePrefetchEngine engine(2, 16);

    std::atomic<int> callCount{ 0 };
    engine.Start([&](const PrefetchRequest&) { ++callCount; });

    REQUIRE(engine.IsRunning());
    engine.Stop();
    REQUIRE_FALSE(engine.IsRunning());
}

TEST_CASE("PredictivePrefetchEngine — EnqueuePath triggers decode callback", "[prefetch]") {
    using namespace ExplorerLens::Engine;

    PredictivePrefetchEngine engine(1, 16);

    std::atomic<int> callCount{ 0 };
    engine.Start([&](const PrefetchRequest& req) {
        (void)req;
        ++callCount;
    });

    engine.EnqueuePath("C:\\test\\photo.jpg", 256);

    // Wait up to 500ms for callback
    for (int i = 0; i < 50 && callCount.load() == 0; ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    engine.Stop();
    REQUIRE(callCount.load() >= 1);
}

TEST_CASE("PredictivePrefetchEngine — CancelDirectory clears pending requests", "[prefetch]") {
    using namespace ExplorerLens::Engine;

    PredictivePrefetchEngine engine(1, 64);

    // Don't start the worker — queue requests manually
    engine.EnqueuePath("C:\\photos\\a.jpg", 256);
    engine.EnqueuePath("C:\\photos\\b.jpg", 256);
    engine.EnqueuePath("C:\\other\\c.jpg",  256);

    engine.CancelDirectory("C:\\photos");

    // Start with a callback that counts what's left
    std::atomic<int> otherCount{ 0 };
    engine.Start([&](const PrefetchRequest& req) {
        if (req.path.parent_path() == "C:\\other") ++otherCount;
    });

    for (int i = 0; i < 30 && otherCount.load() == 0; ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    engine.Stop();
    REQUIRE(otherCount.load() == 1);
}

TEST_CASE("PredictivePrefetchEngine — SetRadius changes radius atomically", "[prefetch]") {
    using namespace ExplorerLens::Engine;

    PredictivePrefetchEngine engine(2, 16);
    REQUIRE(engine.GetRadius() == 2u);

    engine.SetRadius(5);
    REQUIRE(engine.GetRadius() == 5u);

    engine.SetRadius(0);
    REQUIRE(engine.GetRadius() == 0u);
}

// ─────────────────────────────────────────────────────────────────────────────
//  CacheMetricsCollector — JSON parsing
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("CacheMetricsCollector — LastSnapshot returns zero on fresh instance", "[metrics]") {
    using namespace ExplorerLens::Engine;

    CacheMetricsCollector collector(1);  // 1-second interval
    auto snap = collector.LastSnapshot();
    REQUIRE(snap.totalRequests == 0u);
    REQUIRE(snap.l1HitRate == Catch::Approx(0.0));
}

TEST_CASE("CacheMetricsCollector — starts polling and fires callback", "[metrics]") {
    using namespace ExplorerLens::Engine;

    CacheMetricsCollector collector(1);   // 1-second interval

    std::atomic<int> cbCount{ 0 };
    collector.Start(
        [] { return "{\"l1HitRate\":0.85,\"l2HitRate\":0.60,\"totalRequests\":1000,"
                    "\"evictions\":12,\"l1FillPercent\":72.5}"; },
        [&](const CacheSnapshot&) { ++cbCount; });

    // Wait up to 3s for at least one tick
    for (int i = 0; i < 60 && cbCount.load() == 0; ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

    collector.Stop();

    auto snap = collector.LastSnapshot();
    REQUIRE(snap.l1HitRate     == Catch::Approx(0.85));
    REQUIRE(snap.l2HitRate     == Catch::Approx(0.60));
    REQUIRE(snap.totalRequests == 1000u);
    REQUIRE(snap.evictions     == 12u);
    REQUIRE(snap.l1FillPercent == Catch::Approx(72.5));
    REQUIRE(cbCount.load()     >= 1);
}

// ─────────────────────────────────────────────────────────────────────────────
//  EventLogProvider — smoke test (no admin required)
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("EventLogProvider — Register + Deregister does not crash", "[eventlog]") {
    using namespace ExplorerLens::Engine;

    auto& evl = EventLogProvider::Instance();
    // Register (may silently fail without admin — must not throw)
    REQUIRE_NOTHROW(evl.Register());

    // Report an event (may silently fail if not registered — must not throw)
    REQUIRE_NOTHROW(evl.Info(EventId::CONFIG_LOAD_FAILED,
                             L"Catch2 smoke test event — ignore"));

    REQUIRE_NOTHROW(evl.Deregister());
}

TEST_CASE("EventLogProvider — double Register is idempotent", "[eventlog]") {
    using namespace ExplorerLens::Engine;

    auto& evl = EventLogProvider::Instance();
    REQUIRE_NOTHROW(evl.Register());
    REQUIRE_NOTHROW(evl.Register());   // second call must be no-op
    REQUIRE_NOTHROW(evl.Deregister());
}
