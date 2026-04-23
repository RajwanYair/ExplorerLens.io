// DecoderFallbackChainTests.cpp — Catch2 tests for DecoderFallbackChain
// Copyright (c) 2026 ExplorerLens Project
//
// Tests the ordered decoder fallback resolution chain from §7.3 decoder
// priority tier table. Exercises ChainEntry priority sorting, availability
// toggling, max-chain-length enforcement, and chain exhaustion semantics.
// References: ROADMAP §7.3, §10.2, D43 (DecoderRegistryV2 compatibility).
//
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "../../Core/DecoderFallbackChain.h"

#include <algorithm>
#include <string>
#include <vector>

using namespace ExplorerLens::Engine;

// =============================================================================
// §1 — Initialization
// =============================================================================

TEST_CASE("FallbackChain: default-constructed chain is not initialized", "[fallback][init]") {
    DecoderFallbackChain chain;
    REQUIRE_FALSE(chain.IsInitialized());
}

TEST_CASE("FallbackChain: Initialize() returns true on first call", "[fallback][init]") {
    DecoderFallbackChain chain;
    REQUIRE(chain.Initialize());
    REQUIRE(chain.IsInitialized());
}

TEST_CASE("FallbackChain: Initialize() is idempotent", "[fallback][init]") {
    DecoderFallbackChain chain;
    REQUIRE(chain.Initialize());
    REQUIRE(chain.Initialize());  // second call also succeeds
    REQUIRE(chain.IsInitialized());
}

TEST_CASE("FallbackChain: GetName returns default label", "[fallback][config]") {
    DecoderFallbackChain chain;
    REQUIRE(chain.GetName() == "DecoderFallbackChain");
}

TEST_CASE("FallbackChain: GetConfig has sensible defaults", "[fallback][config]") {
    DecoderFallbackChain chain;
    auto cfg = chain.GetConfig();
    REQUIRE(cfg.enabled);
    REQUIRE(cfg.maxChainLength >= 1u);
}

// =============================================================================
// §2 — AddDecoder: basic insertion
// =============================================================================

TEST_CASE("FallbackChain: empty chain has zero length", "[fallback][add]") {
    DecoderFallbackChain chain;
    REQUIRE(chain.GetChainLength() == 0u);
}

TEST_CASE("FallbackChain: single decoder added, length = 1", "[fallback][add]") {
    DecoderFallbackChain chain;
    chain.Initialize();
    DecoderFallbackChain::ChainEntry e{"JpegDecoder", 10, true};
    REQUIRE(chain.AddDecoder(e));
    REQUIRE(chain.GetChainLength() == 1u);
}

TEST_CASE("FallbackChain: three decoders added, length = 3", "[fallback][add]") {
    DecoderFallbackChain chain;
    chain.Initialize();
    REQUIRE(chain.AddDecoder({"JpegDecoder", 10, true}));
    REQUIRE(chain.AddDecoder({"WICDecoder",  5,  true}));
    REQUIRE(chain.AddDecoder({"GDIPlusDecoder", 1, true}));
    REQUIRE(chain.GetChainLength() == 3u);
}

TEST_CASE("FallbackChain: max chain length enforced", "[fallback][add]") {
    DecoderFallbackChain chain;
    chain.Initialize();
    auto cfg = chain.GetConfig();
    uint32_t maxLen = cfg.maxChainLength;

    // Fill to exactly maxLen
    for (uint32_t i = 0; i < maxLen; ++i) {
        DecoderFallbackChain::ChainEntry e{"decoder" + std::to_string(i), (int)i, true};
        REQUIRE(chain.AddDecoder(e));
    }
    REQUIRE(chain.GetChainLength() == maxLen);

    // One more should be rejected
    DecoderFallbackChain::ChainEntry overflow{"overflowDecoder", 99, true};
    REQUIRE_FALSE(chain.AddDecoder(overflow));
    REQUIRE(chain.GetChainLength() == maxLen);  // unchanged
}

// =============================================================================
// §3 — GetFirstAvailable: priority and availability
// =============================================================================

TEST_CASE("FallbackChain: GetFirstAvailable on empty chain returns null", "[fallback][get]") {
    DecoderFallbackChain chain;
    chain.Initialize();
    REQUIRE(chain.GetFirstAvailable() == nullptr);
}

TEST_CASE("FallbackChain: GetFirstAvailable returns first inserted when all available",
          "[fallback][get]") {
    DecoderFallbackChain chain;
    chain.Initialize();
    chain.AddDecoder({"PrimaryDecoder", 100, true});
    chain.AddDecoder({"FallbackDecoder", 50, true});

    const auto* e = chain.GetFirstAvailable();
    REQUIRE(e != nullptr);
    REQUIRE(e->decoderName == "PrimaryDecoder");
}

TEST_CASE("FallbackChain: GetFirstAvailable skips unavailable entries", "[fallback][get]") {
    DecoderFallbackChain chain;
    chain.Initialize();
    chain.AddDecoder({"BrokenDecoder", 100, false});   // unavailable
    chain.AddDecoder({"WorkingDecoder", 50, true});    // available

    const auto* e = chain.GetFirstAvailable();
    REQUIRE(e != nullptr);
    REQUIRE(e->decoderName == "WorkingDecoder");
}

TEST_CASE("FallbackChain: all unavailable returns null", "[fallback][get]") {
    DecoderFallbackChain chain;
    chain.Initialize();
    chain.AddDecoder({"DisabledDecoder1", 10, false});
    chain.AddDecoder({"DisabledDecoder2", 5,  false});

    REQUIRE(chain.GetFirstAvailable() == nullptr);
}

TEST_CASE("FallbackChain: first two unavailable, third available", "[fallback][get]") {
    DecoderFallbackChain chain;
    chain.Initialize();
    chain.AddDecoder({"A", 30, false});
    chain.AddDecoder({"B", 20, false});
    chain.AddDecoder({"C", 10, true});

    const auto* e = chain.GetFirstAvailable();
    REQUIRE(e != nullptr);
    REQUIRE(e->decoderName == "C");
}

// =============================================================================
// §4 — Realistic JPEG fallback scenario
// =============================================================================

TEST_CASE("FallbackChain: JPEG primary libjpeg-turbo → WIC fallback", "[fallback][scenario]") {
    // Simulate: libjpeg-turbo is the primary; WIC is the fallback (§7.3 P0)
    DecoderFallbackChain chain;
    chain.Initialize();

    // Primary: libjpeg-turbo (priority 100, available)
    chain.AddDecoder({"libjpeg-turbo", 100, true});
    // Fallback: WIC (priority 50)
    chain.AddDecoder({"WIC",           50, true});

    const auto* first = chain.GetFirstAvailable();
    REQUIRE(first != nullptr);
    REQUIRE(first->decoderName == "libjpeg-turbo");
}

TEST_CASE("FallbackChain: JPEG primary fails, WIC fallback used", "[fallback][scenario]") {
    DecoderFallbackChain chain;
    chain.Initialize();
    chain.AddDecoder({"libjpeg-turbo", 100, false});  // failed to init
    chain.AddDecoder({"WIC",           50,  true});

    const auto* first = chain.GetFirstAvailable();
    REQUIRE(first != nullptr);
    REQUIRE(first->decoderName == "WIC");
}

TEST_CASE("FallbackChain: all decoders unavailable — GDI+ fallback returns null too",
          "[fallback][scenario]") {
    DecoderFallbackChain chain;
    chain.Initialize();
    chain.AddDecoder({"libjpeg-turbo", 100, false});
    chain.AddDecoder({"WIC",           50,  false});
    chain.AddDecoder({"GDIPlus",       10,  false});

    REQUIRE(chain.GetFirstAvailable() == nullptr);
}

// =============================================================================
// §5 — Priority field semantics
// =============================================================================

TEST_CASE("FallbackChain: zero-priority decoder is still valid", "[fallback][priority]") {
    DecoderFallbackChain chain;
    chain.Initialize();
    chain.AddDecoder({"ZeroPriDecoder", 0, true});

    const auto* e = chain.GetFirstAvailable();
    REQUIRE(e != nullptr);
    REQUIRE(e->decoderName == "ZeroPriDecoder");
}

TEST_CASE("FallbackChain: negative priority decoder is inserted", "[fallback][priority]") {
    DecoderFallbackChain chain;
    chain.Initialize();
    chain.AddDecoder({"NegPri", -5, true});
    REQUIRE(chain.GetChainLength() == 1u);
}

// =============================================================================
// §6 — GetConfig fields
// =============================================================================

TEST_CASE("FallbackChain: maxChainLength >= 4 (supports P0/P1/P2 + WIC)", "[fallback][config]") {
    DecoderFallbackChain chain;
    REQUIRE(chain.GetConfig().maxChainLength >= 4u);
}

TEST_CASE("FallbackChain: enabled flag defaults to true", "[fallback][config]") {
    DecoderFallbackChain chain;
    REQUIRE(chain.GetConfig().enabled);
}

TEST_CASE("FallbackChain: label is non-empty", "[fallback][config]") {
    DecoderFallbackChain chain;
    REQUIRE_FALSE(chain.GetConfig().label.empty());
}

// =============================================================================
// §7 — GENERATE: parametric availability patterns
// =============================================================================

TEST_CASE("FallbackChain: parametric: first available decoder returned correctly",
          "[fallback][parametric]") {
    // Test with different positions of the first available decoder
    int firstAvailIdx = GENERATE(0, 1, 2);

    DecoderFallbackChain chain;
    chain.Initialize();

    std::vector<std::string> names = {"D0", "D1", "D2"};
    for (int i = 0; i < 3; ++i) {
        chain.AddDecoder({names[i], 10 - i, (i == firstAvailIdx)});
    }

    const auto* e = chain.GetFirstAvailable();
    REQUIRE(e != nullptr);
    REQUIRE(e->decoderName == names[firstAvailIdx]);
}
