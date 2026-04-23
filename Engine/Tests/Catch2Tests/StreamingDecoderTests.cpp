// StreamingDecoderTests.cpp — Catch2 tests for IStreamingDecoder contract
// Copyright (c) 2026 ExplorerLens Project
//
// Tests the two-phase decode contract (ProbeHeader / DecodeAtSize) using
// a minimal in-memory stub decoder. Does NOT link against real decoder libs.
//
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

#include "../../Core/IStreamingDecoder.h"

#include <array>
#include <cstring>
#include <vector>
#include <stop_token>
#include <sstream>

using namespace ExplorerLens::Engine;

// ---------------------------------------------------------------------------
// Minimal ProbeResult / DecodeResult helpers
// ---------------------------------------------------------------------------

static ProbeResult MakeOkProbe(uint32_t w = 64, uint32_t h = 64,
                                 const std::string& fmt = "JPEG") {
    ProbeResult r;
    r.status          = ProbeStatus::OK;
    r.formatId        = fmt;
    r.meta.width      = w;
    r.meta.height     = h;
    r.meta.hasAlpha   = false;
    r.meta.frameCount = 1;
    return r;
}

static DecodedThumb MakeOkThumb(uint32_t w = 32, uint32_t h = 32) {
    DecodedThumb t;
    t.status = DecodeStatus::OK;
    t.width  = w;
    t.height = h;
    t.stride = w * 4;
    t.pixels.resize(static_cast<size_t>(t.stride) * h, 0xFF);
    return t;
}

// ---------------------------------------------------------------------------
// Stub decoder: always succeeds for "JPEG"; fails for "FAIL"
// ---------------------------------------------------------------------------

class StubDecoder final : public IStreamingDecoder {
public:
    const char* DecoderId() const noexcept override { return "stub"; }
    const char* DisplayName() const noexcept override { return "Stub Decoder (test)"; }

    ProbeResult ProbeHeader(std::span<const uint8_t> data) const override {
        if (data.size() >= 4 &&
            data[0] == 0xFF && data[1] == 0xD8) {  // JPEG SOI
            return MakeOkProbe(800, 600, "JPEG");
        }
        if (data.size() >= 1 && data[0] == 0x00) {
            ProbeResult r;
            r.status   = ProbeStatus::UNSUPPORTED;
            r.formatId = "UNKNOWN";
            return r;
        }
        return MakeOkProbe(64, 64, "STUB");
    }

    DecodedThumb DecodeAtSize(void*             /*stream*/,
                               uint32_t          targetSize,
                               std::stop_token   stop) override {
        if (stop.stop_requested()) {
            DecodedThumb t;
            t.status = DecodeStatus::CANCELLED;
            return t;
        }
        // Use targetSize to determine success/failure:
        // If targetSize == 0 we simulate a corrupt error.
        if (targetSize == 0) {
            DecodedThumb t;
            t.status = DecodeStatus::CORRUPT;
            return t;
        }
        return MakeOkThumb(targetSize, targetSize);
    }

    std::vector<std::string> SupportedExtensions() const override {
        return { ".stub", ".test" };
    }
};

// ---------------------------------------------------------------------------
// Stub factory
// ---------------------------------------------------------------------------

class StubDecoderFactory final : public IStreamingDecoderFactory {
public:
    std::unique_ptr<IStreamingDecoder> Create() const override {
        return std::make_unique<StubDecoder>();
    }
    const char* DecoderId() const noexcept override { return "stub"; }
};

// ---------------------------------------------------------------------------
// Tests: IStreamingDecoder contract
// ---------------------------------------------------------------------------

TEST_CASE("IStreamingDecoder: ProbeHeader recognises JPEG SOI bytes", "[streaming][probe]") {
    StubDecoder dec;
    std::array<uint8_t, 4> hdr = { 0xFF, 0xD8, 0xFF, 0xE0 };
    auto r = dec.ProbeHeader(hdr);
    REQUIRE(r.status == ProbeStatus::OK);
    CHECK(r.formatId == "JPEG");
    CHECK(r.meta.width  == 800);
    CHECK(r.meta.height == 600);
    CHECK(r.meta.frameCount == 1);
}

TEST_CASE("IStreamingDecoder: ProbeHeader returns UNSUPPORTED for null magic", "[streaming][probe]") {
    StubDecoder dec;
    std::array<uint8_t, 4> hdr = { 0x00, 0x00, 0x00, 0x00 };
    auto r = dec.ProbeHeader(hdr);
    REQUIRE(r.status == ProbeStatus::UNSUPPORTED);
}

TEST_CASE("IStreamingDecoder: DecodeAtSize returns correct size", "[streaming][decode]") {
    StubDecoder dec;
    int dummy = 0;  // fake stream pointer
    std::stop_source ss;
    auto t = dec.DecodeAtSize(&dummy, 128, ss.get_token());
    REQUIRE(t.status == DecodeStatus::OK);
    CHECK(t.width  == 128);
    CHECK(t.height == 128);
    CHECK(t.stride == 128 * 4);
    REQUIRE_FALSE(t.pixels.empty());
    CHECK(t.pixels.size() == 128u * 128u * 4u);
}

TEST_CASE("IStreamingDecoder: DecodeAtSize honours stop_token cancellation", "[streaming][cancel]") {
    StubDecoder dec;
    int dummy = 0;
    std::stop_source ss;
    ss.request_stop();  // cancel immediately
    auto t = dec.DecodeAtSize(&dummy, 64, ss.get_token());
    REQUIRE(t.status == DecodeStatus::CANCELLED);
}

TEST_CASE("IStreamingDecoder: DecodeAtSize returns CORRUPT for bad data", "[streaming][error]") {
    StubDecoder dec;
    int dummy = 0;
    std::stop_source ss;
    // targetSize 0 triggers CORRUPT in our stub
    auto t = dec.DecodeAtSize(&dummy, 0, ss.get_token());
    REQUIRE(t.status == DecodeStatus::CORRUPT);
}

TEST_CASE("IStreamingDecoder: DecoderId returns non-empty string", "[streaming]") {
    StubDecoder dec;
    CHECK(std::string(dec.DecoderId()).size() > 0);
}

// ---------------------------------------------------------------------------
// Tests: IStreamingDecoderFactory
// ---------------------------------------------------------------------------

TEST_CASE("IStreamingDecoderFactory: Create returns non-null decoder", "[factory]") {
    StubDecoderFactory f;
    auto dec = f.Create();
    REQUIRE(dec != nullptr);
}

TEST_CASE("IStreamingDecoderFactory: DecoderId is non-empty", "[factory]") {
    StubDecoderFactory f;
    CHECK(std::string(f.DecoderId()).size() > 0);
}

// ---------------------------------------------------------------------------
// Tests: ProbeResult / DecodedThumb value semantics
// ---------------------------------------------------------------------------

TEST_CASE("ProbeResult: default status is UNSUPPORTED", "[probe-result]") {
    ProbeResult r{};
    CHECK(r.status == ProbeStatus::UNSUPPORTED);
    CHECK(r.meta.width  == 0);
    CHECK(r.meta.height == 0);
    CHECK(r.meta.frameCount == 1);
}

TEST_CASE("DecodedThumb: default status is UNSUPPORTED", "[decode-result]") {
    DecodedThumb t{};
    CHECK(t.status == DecodeStatus::UNSUPPORTED);
    CHECK(t.pixels.empty());
}

TEST_CASE("DecodedThumb: copy of pixels is independent", "[decode-result]") {
    auto a = MakeOkThumb(16, 16);
    auto b = a;
    b.pixels[0] = 0xAB;
    CHECK(a.pixels[0] != 0xAB);
}

// end of file
