// PipelineIntegrationTests.cpp — Catch2 integration tests for the decode pipeline
// Copyright (c) 2026 ExplorerLens Project
//
// ROADMAP: §7.1 (DecodePipeline probe → route → decode-at-size → transform → output),
//          §10.2 (Integration layer: ~50 tests), §10.4 (Catch2 migration)
//
// PURPOSE
// ───────
// Exercises the end-to-end decode pipeline state machine using self-contained
// stub decoders so we validate *behavior*, not individual library linkage.
// Tests cover: probe → route → decode → transform → result sequencing,
// partial decode fallback, cancellation via stop_token, and error propagation.
//
// Self-contained: no production Engine headers — uses local mock types.
// Thread-safe: all fixtures are stack-allocated; no globals mutated.

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <optional>
#include <span>
#include <stop_token>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace ExplorerLens::Engine::PipelineIntegration {

// ============================================================================
// Self-contained pipeline model (mirrors §7.4 IStreamingDecoder contract)
// ============================================================================

enum class PipelineStage : uint8_t {
    IDLE,
    PROBE,
    ROUTE,
    DECODE,
    TRANSFORM,
    OUTPUT,
    FAILED,
    CANCELLED
};

enum class PartialDecodeState : uint8_t {
    COMPLETE,
    PARTIAL,
    HEADER_ONLY,
    FAILED
};

enum class PipelineError : uint8_t {
    NONE,
    UNKNOWN_FORMAT,
    HEADER_INVALID,
    DECODE_FAILED,
    CANCELLED,
    OUT_OF_MEMORY
};

struct PipelineResult {
    PipelineStage finalStage{PipelineStage::IDLE};
    PartialDecodeState partial{PartialDecodeState::FAILED};
    PipelineError error{PipelineError::NONE};
    uint32_t width{0};
    uint32_t height{0};
    std::vector<PipelineStage> transitions;
};

// Self-contained stub "decoder" — deterministic behavior for tests.
struct MockDecoder {
    std::string_view name{"mock"};
    bool supportsPartial{true};
    bool failProbe{false};
    bool failDecode{false};
    uint32_t outWidth{256};
    uint32_t outHeight{256};

    bool ProbeHeader(std::span<const uint8_t> bytes) const noexcept {
        if (failProbe) return false;
        // 8-byte minimum header for all mocks
        return bytes.size() >= 8;
    }

    bool Decode(std::stop_token stop) const noexcept {
        if (failDecode) return false;
        for (int i = 0; i < 4; ++i) {
            if (stop.stop_requested()) return false;
        }
        return true;
    }
};

// The state machine under test
PipelineResult RunPipeline(const MockDecoder& dec,
                            std::span<const uint8_t> bytes,
                            std::stop_token stop = {}) noexcept {
    PipelineResult r;
    r.transitions.reserve(6);

    r.transitions.push_back(PipelineStage::PROBE);
    if (!dec.ProbeHeader(bytes)) {
        r.finalStage = PipelineStage::FAILED;
        r.error = bytes.size() < 8 ? PipelineError::HEADER_INVALID
                                    : PipelineError::UNKNOWN_FORMAT;
        return r;
    }

    r.transitions.push_back(PipelineStage::ROUTE);
    if (stop.stop_requested()) {
        r.finalStage = PipelineStage::CANCELLED;
        r.error = PipelineError::CANCELLED;
        return r;
    }

    r.transitions.push_back(PipelineStage::DECODE);
    if (!dec.Decode(stop)) {
        if (stop.stop_requested()) {
            r.finalStage = PipelineStage::CANCELLED;
            r.error = PipelineError::CANCELLED;
            r.partial = PartialDecodeState::PARTIAL;
            return r;
        }
        if (dec.supportsPartial) {
            r.finalStage = PipelineStage::OUTPUT;
            r.partial = PartialDecodeState::HEADER_ONLY;
            r.width = dec.outWidth;
            r.height = dec.outHeight;
            return r;
        }
        r.finalStage = PipelineStage::FAILED;
        r.error = PipelineError::DECODE_FAILED;
        return r;
    }

    r.transitions.push_back(PipelineStage::TRANSFORM);
    r.transitions.push_back(PipelineStage::OUTPUT);
    r.finalStage = PipelineStage::OUTPUT;
    r.partial = PartialDecodeState::COMPLETE;
    r.width = dec.outWidth;
    r.height = dec.outHeight;
    return r;
}

// ============================================================================
// Tests
// ============================================================================

TEST_CASE("Pipeline happy path: probe → route → decode → transform → output",
          "[Pipeline][Integration]") {
    MockDecoder dec;
    std::vector<uint8_t> bytes(16, 0xAB);

    auto r = RunPipeline(dec, bytes);

    REQUIRE(r.finalStage == PipelineStage::OUTPUT);
    REQUIRE(r.partial == PartialDecodeState::COMPLETE);
    REQUIRE(r.error == PipelineError::NONE);
    REQUIRE(r.width == 256);
    REQUIRE(r.height == 256);
    REQUIRE(r.transitions.size() == 5);
    REQUIRE(r.transitions[0] == PipelineStage::PROBE);
    REQUIRE(r.transitions[4] == PipelineStage::OUTPUT);
}

TEST_CASE("Pipeline short header is rejected at probe stage",
          "[Pipeline][Integration][HeaderValidation]") {
    MockDecoder dec;
    std::vector<uint8_t> bytes(4, 0xFF);  // below 8-byte minimum

    auto r = RunPipeline(dec, bytes);

    REQUIRE(r.finalStage == PipelineStage::FAILED);
    REQUIRE(r.error == PipelineError::HEADER_INVALID);
    REQUIRE(r.transitions.size() == 1);
    REQUIRE(r.transitions[0] == PipelineStage::PROBE);
}

TEST_CASE("Pipeline probe failure returns UNKNOWN_FORMAT with sufficient bytes",
          "[Pipeline][Integration]") {
    MockDecoder dec{.failProbe = true};
    std::vector<uint8_t> bytes(64, 0x01);

    auto r = RunPipeline(dec, bytes);

    REQUIRE(r.finalStage == PipelineStage::FAILED);
    REQUIRE(r.error == PipelineError::UNKNOWN_FORMAT);
}

TEST_CASE("Pipeline decoder with partial support returns HEADER_ONLY on failure",
          "[Pipeline][Integration][PartialDecode]") {
    MockDecoder dec{.supportsPartial = true, .failDecode = true};
    std::vector<uint8_t> bytes(32, 0x00);

    auto r = RunPipeline(dec, bytes);

    REQUIRE(r.finalStage == PipelineStage::OUTPUT);
    REQUIRE(r.partial == PartialDecodeState::HEADER_ONLY);
    REQUIRE(r.error == PipelineError::NONE);
    REQUIRE(r.width == 256);
}

TEST_CASE("Pipeline decoder without partial support fails cleanly",
          "[Pipeline][Integration][PartialDecode]") {
    MockDecoder dec{.supportsPartial = false, .failDecode = true};
    std::vector<uint8_t> bytes(32, 0x00);

    auto r = RunPipeline(dec, bytes);

    REQUIRE(r.finalStage == PipelineStage::FAILED);
    REQUIRE(r.error == PipelineError::DECODE_FAILED);
    REQUIRE(r.partial == PartialDecodeState::FAILED);
    REQUIRE(r.width == 0);
    REQUIRE(r.height == 0);
}

TEST_CASE("Pipeline cancellation via stop_token before route",
          "[Pipeline][Integration][Cancellation]") {
    MockDecoder dec;
    std::vector<uint8_t> bytes(16, 0x42);

    std::stop_source src;
    src.request_stop();

    auto r = RunPipeline(dec, bytes, src.get_token());

    REQUIRE(r.finalStage == PipelineStage::CANCELLED);
    REQUIRE(r.error == PipelineError::CANCELLED);
    REQUIRE(r.transitions.size() >= 2);  // at least PROBE + ROUTE
}

TEST_CASE("Pipeline dimension output matches decoder hint",
          "[Pipeline][Integration][Dimensions]") {
    auto w = GENERATE(as<uint32_t>{}, 16u, 64u, 256u, 1024u, 4096u);
    auto h = GENERATE(as<uint32_t>{}, 16u, 64u, 256u, 1024u, 4096u);

    MockDecoder dec{.outWidth = w, .outHeight = h};
    std::vector<uint8_t> bytes(16, 0xCC);

    auto r = RunPipeline(dec, bytes);

    REQUIRE(r.finalStage == PipelineStage::OUTPUT);
    REQUIRE(r.width == w);
    REQUIRE(r.height == h);
}

TEST_CASE("Pipeline transitions always start with PROBE",
          "[Pipeline][Integration][StateMachine]") {
    MockDecoder dec;
    std::vector<uint8_t> bytes(16, 0x00);

    auto r = RunPipeline(dec, bytes);

    REQUIRE(!r.transitions.empty());
    REQUIRE(r.transitions.front() == PipelineStage::PROBE);
}

TEST_CASE("Pipeline transitions are monotonic (no back-edges)",
          "[Pipeline][Integration][StateMachine]") {
    MockDecoder dec;
    std::vector<uint8_t> bytes(16, 0x00);

    auto r = RunPipeline(dec, bytes);

    // Ordinal ordering of enum values mirrors forward progression
    for (size_t i = 1; i < r.transitions.size(); ++i) {
        auto prev = static_cast<uint8_t>(r.transitions[i - 1]);
        auto curr = static_cast<uint8_t>(r.transitions[i]);
        REQUIRE(curr >= prev);
    }
}

TEST_CASE("Pipeline is thread-safe (concurrent independent runs)",
          "[Pipeline][Integration][Concurrency]") {
    std::atomic<int> successes{0};
    std::atomic<int> failures{0};
    std::vector<std::thread> threads;
    threads.reserve(8);

    for (int i = 0; i < 8; ++i) {
        threads.emplace_back([&successes, &failures, i]() {
            MockDecoder dec{.outWidth = 128u + static_cast<uint32_t>(i),
                            .outHeight = 128u};
            std::vector<uint8_t> bytes(16, static_cast<uint8_t>(i));
            auto r = RunPipeline(dec, bytes);
            if (r.finalStage == PipelineStage::OUTPUT) {
                successes.fetch_add(1, std::memory_order_relaxed);
            } else {
                failures.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    for (auto& t : threads) t.join();

    REQUIRE(successes.load() == 8);
    REQUIRE(failures.load() == 0);
}

TEST_CASE("Pipeline produces zero-dimension output on failure",
          "[Pipeline][Integration][Invariants]") {
    MockDecoder dec{.supportsPartial = false, .failDecode = true};
    std::vector<uint8_t> bytes(32, 0x00);

    auto r = RunPipeline(dec, bytes);

    REQUIRE(r.width == 0);
    REQUIRE(r.height == 0);
    REQUIRE(r.finalStage == PipelineStage::FAILED);
}

TEST_CASE("Pipeline empty byte span fails at probe",
          "[Pipeline][Integration]") {
    MockDecoder dec;
    std::span<const uint8_t> empty;

    auto r = RunPipeline(dec, empty);

    REQUIRE(r.finalStage == PipelineStage::FAILED);
    REQUIRE(r.error == PipelineError::HEADER_INVALID);
}

}  // namespace ExplorerLens::Engine::PipelineIntegration
