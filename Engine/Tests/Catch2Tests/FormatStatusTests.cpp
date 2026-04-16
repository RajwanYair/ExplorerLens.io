// FormatStatusTests.cpp — Catch2 tests for FormatStatusProvider
// Copyright (c) 2026 ExplorerLens Project
//
// Validates the singleton health-tracking counters and derived health states.
//
#include <catch2/catch_test_macros.hpp>

#include "../../Core/FormatStatusProvider.h"

#include <string>
#include <algorithm>

using namespace ExplorerLens::Engine;

// Reset the singleton state between test cases using a helper.
// FormatStatusProvider exposes Reset() for testing; if not present,
// use a fresh format name per test to avoid cross-test contamination.

static std::string UniqueFormat(const char* base) {
    static int counter = 0;
    return std::string(base) + "_" + std::to_string(++counter);
}

// ---------------------------------------------------------------------------
// RecordSuccess / RecordFailure basics
// ---------------------------------------------------------------------------

TEST_CASE("FormatStatusProvider: HEALTHY when 0 failures", "[fsp][health]") {
    auto fmt = UniqueFormat("JPEG");
    auto& fsp = FormatStatusProvider::Instance();
    fsp.RecordSuccess(fmt, 10000);
    fsp.RecordSuccess(fmt, 12000);
    fsp.RecordSuccess(fmt, 8000);
    CHECK(fsp.GetHealth(fmt) == DecoderHealth::HEALTHY);
}

TEST_CASE("FormatStatusProvider: FAILING when failure rate > 10%", "[fsp][health]") {
    auto fmt = UniqueFormat("TIFF");
    auto& fsp = FormatStatusProvider::Instance();
    for (int i = 0; i < 10; ++i) fsp.RecordFailure(fmt);
    for (int i = 0; i < 5; ++i)  fsp.RecordSuccess(fmt, 5000);
    // 10 fail / 15 total = 66% failure rate → FAILING
    CHECK(fsp.GetHealth(fmt) == DecoderHealth::FAILING);
}

TEST_CASE("FormatStatusProvider: DEGRADED when failure rate 1-10%", "[fsp][health]") {
    auto fmt = UniqueFormat("PNG");
    auto& fsp = FormatStatusProvider::Instance();
    for (int i = 0; i < 1; ++i)  fsp.RecordFailure(fmt);
    for (int i = 0; i < 19; ++i) fsp.RecordSuccess(fmt, 7000);
    // 1 fail / 20 total = 5% → DEGRADED
    CHECK(fsp.GetHealth(fmt) == DecoderHealth::DEGRADED);
}

TEST_CASE("FormatStatusProvider: UNAVAILABLE after MarkUnavailable", "[fsp][health]") {
    auto fmt = UniqueFormat("AVIF");
    auto& fsp = FormatStatusProvider::Instance();
    fsp.MarkUnavailable(fmt);
    CHECK(fsp.GetHealth(fmt) == DecoderHealth::UNAVAILABLE);
}

// ---------------------------------------------------------------------------
// RecordFallback
// ---------------------------------------------------------------------------

TEST_CASE("FormatStatusProvider: RecordFallback increments fallback counter", "[fsp][fallback]") {
    auto fmt = UniqueFormat("HEIC");
    auto& fsp = FormatStatusProvider::Instance();
    fsp.RecordFallback(fmt);
    fsp.RecordFallback(fmt);
    auto snaps = fsp.GetAllSnapshots();
    auto it = std::find_if(snaps.begin(), snaps.end(),
                            [&](const FormatStatusSnapshot& s) { return s.formatId == fmt; });
    REQUIRE(it != snaps.end());
    CHECK(it->fallbacks >= 2);
}

// ---------------------------------------------------------------------------
// GetAllSnapshots
// ---------------------------------------------------------------------------

TEST_CASE("FormatStatusProvider: GetAllSnapshots returns all recorded formats", "[fsp][snapshots]") {
    auto fmt1 = UniqueFormat("WEBP");
    auto fmt2 = UniqueFormat("JXL");
    auto& fsp = FormatStatusProvider::Instance();
    fsp.RecordSuccess(fmt1, 5000);
    fsp.RecordSuccess(fmt2, 8000);
    auto snaps = fsp.GetAllSnapshots();
    auto find = [&](const std::string& id) {
        return std::any_of(snaps.begin(), snaps.end(),
                           [&](const FormatStatusSnapshot& s) { return s.formatId == id; });
    };
    CHECK(find(fmt1));
    CHECK(find(fmt2));
}

TEST_CASE("FormatStatusProvider: GetAllSnapshots snapshots are sorted by formatId", "[fsp][snapshots]") {
    auto& fsp = FormatStatusProvider::Instance();
    auto snaps = fsp.GetAllSnapshots();
    for (size_t i = 1; i < snaps.size(); ++i) {
        CHECK(snaps[i-1].formatId <= snaps[i].formatId);
    }
}

// ---------------------------------------------------------------------------
// HasFailingDecoders
// ---------------------------------------------------------------------------

TEST_CASE("FormatStatusProvider: HasFailingDecoders returns true when FAILING decoder exists", "[fsp][gate]") {
    auto fmt = UniqueFormat("DDS");
    auto& fsp = FormatStatusProvider::Instance();
    for (int i = 0; i < 20; ++i) fsp.RecordFailure(fmt);
    CHECK(fsp.HasFailingDecoders());
}

TEST_CASE("FormatStatusProvider: UNKNOWN health for unseen format", "[fsp][health]") {
    auto& fsp = FormatStatusProvider::Instance();
    CHECK(fsp.GetHealth("FORMAT_THAT_DOES_NOT_EXIST_XYZ") == DecoderHealth::UNKNOWN);
}

// ---------------------------------------------------------------------------
// Average decode time tracking
// ---------------------------------------------------------------------------

TEST_CASE("FormatStatusProvider: average decode time is computed correctly", "[fsp][perf]") {
    auto fmt = UniqueFormat("PDF");
    auto& fsp = FormatStatusProvider::Instance();
    fsp.RecordSuccess(fmt, 10000);  // 10ms
    fsp.RecordSuccess(fmt, 20000);  // 20ms
    fsp.RecordSuccess(fmt, 30000);  // 30ms
    auto snaps = fsp.GetAllSnapshots();
    auto it = std::find_if(snaps.begin(), snaps.end(),
                            [&](const FormatStatusSnapshot& s) { return s.formatId == fmt; });
    REQUIRE(it != snaps.end());
    // Average should be ~20ms (20000 µs)
    CHECK(it->avgDecodeUs >= 15000);
    CHECK(it->avgDecodeUs <= 25000);
}
