// StreamingDecoderTests.cpp — Catch2 tests for IStreamingDecoder contract
// Copyright (c) 2026 ExplorerLens Project
//
// Tests the two-phase decode contract (ProbeHeader / DecodeAtSize) using
// a minimal in-memory stub decoder. Does NOT link against real decoder libs.
//
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

#include "../../Core/IStreamingDecoder.h"

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
    r.status     = ProbeStatus::OK;
    r.width      = w;
    r.height     = h;
    r.formatId   = fmt;
    r.colorSpace = ColorSpace::SRGB;
    r.hasAlpha   = false;
    r.frameCount = 1;
    return r;
}

static DecodedThumb MakeOkThumb(uint32_t w = 32, uint32_t h = 32) {
    DecodedThumb t;
    t.status     = DecodeStatus::OK;
    t.width      = w;
    t.height     = h;
    t.stride     = w * 4;
    t.pixelFormat = PixelFormat::BGRA32;
    t.pixels.resize(static_cast<size_t>(t.stride) * h, 0xFF);
    return t;
}

// ---------------------------------------------------------------------------
// Stub decoder: always succeeds for "JPEG"; fails for "FAIL"
// ---------------------------------------------------------------------------

class StubDecoder final : public IStreamingDecoder {
public:
    std::string GetFormatId() const noexcept override { return "STUB"; }

    bool CanHandle(std::span<const uint8_t> header,
                   const std::string&       extension) const noexcept override {
        (void)header; (void)extension;
        return true;  // accepts everything
    }

    ProbeResult ProbeHeader(std::span<const uint8_t> data) noexcept override {
        if (data.size() >= 4 &&
            data[0] == 0xFF && data[1] == 0xD8) {  // JPEG SOI
            return MakeOkProbe(800, 600, "JPEG");
        }
        if (data.size() >= 1 && data[0] == 0x00) {
            ProbeResult r;
            r.status  = ProbeStatus::UNSUPPORTED_FORMAT;
            r.formatId = "UNKNOWN";
            return r;
        }
        return MakeOkProbe(64, 64, "STUB");
    }

    DecodedThumb DecodeAtSize(std::span<const uint8_t> data,
                               uint32_t                 targetSize,
                               std::stop_token          stop) noexcept override {
        if (stop.stop_requested()) {
            DecodedThumb t;
            t.status = DecodeStatus::CANCELLED;
            return t;
        }
        if (data.size() >= 1 && data[0] == 0x00) {
            DecodedThumb t;
            t.status = DecodeStatus::DECODE_ERROR;
            return t;
        }
        auto thumb = MakeOkThumb(targetSize, targetSize);
        thumb.durationMs = 5.0f;
        return thumb;
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
    std::string GetFormatId() const noexcept override { return "STUB"; }
    int         GetPriority() const noexcept override { return 50; }
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
    CHECK(r.width  == 800);
    CHECK(r.height == 600);
    CHECK(r.frameCount == 1);
}

TEST_CASE("IStreamingDecoder: ProbeHeader returns UNSUPPORTED for null magic", "[streaming][probe]") {
    StubDecoder dec;
    std::array<uint8_t, 4> hdr = { 0x00, 0x00, 0x00, 0x00 };
    auto r = dec.ProbeHeader(hdr);
    REQUIRE(r.status == ProbeStatus::UNSUPPORTED_FORMAT);
}

TEST_CASE("IStreamingDecoder: DecodeAtSize returns correct size", "[streaming][decode]") {
    StubDecoder dec;
    std::array<uint8_t, 8> data = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
    std::stop_source ss;
    auto t = dec.DecodeAtSize(data, 128, ss.get_token());
    REQUIRE(t.status == DecodeStatus::OK);
    CHECK(t.width  == 128);
    CHECK(t.height == 128);
    CHECK(t.stride == 128 * 4);
    REQUIRE_FALSE(t.pixels.empty());
    CHECK(t.pixels.size() == 128u * 128u * 4u);
}

TEST_CASE("IStreamingDecoder: DecodeAtSize honours stop_token cancellation", "[streaming][cancel]") {
    StubDecoder dec;
    std::array<uint8_t, 4> data = { 0x01, 0x02, 0x03, 0x04 };
    std::stop_source ss;
    ss.request_stop();  // cancel immediately
    auto t = dec.DecodeAtSize(data, 64, ss.get_token());
    REQUIRE(t.status == DecodeStatus::CANCELLED);
}

TEST_CASE("IStreamingDecoder: DecodeAtSize returns DECODE_ERROR for bad data", "[streaming][error]") {
    StubDecoder dec;
    std::array<uint8_t, 4> data = { 0x00, 0x00, 0x00, 0x00 };
    std::stop_source ss;
    auto t = dec.DecodeAtSize(data, 64, ss.get_token());
    REQUIRE(t.status == DecodeStatus::DECODE_ERROR);
}

TEST_CASE("IStreamingDecoder: GetFormatId returns non-empty string", "[streaming]") {
    StubDecoder dec;
    CHECK_FALSE(dec.GetFormatId().empty());
}

TEST_CASE("IStreamingDecoder: CanHandle returns bool without throwing", "[streaming]") {
    StubDecoder dec;
    std::array<uint8_t, 4> hdr = { 0xFF, 0xD8, 0xFF, 0xE0 };
    REQUIRE_NOTHROW(dec.CanHandle(hdr, ".jpg"));
}

// ---------------------------------------------------------------------------
// Tests: IStreamingDecoderFactory
// ---------------------------------------------------------------------------

TEST_CASE("IStreamingDecoderFactory: Create returns non-null decoder", "[factory]") {
    StubDecoderFactory f;
    auto dec = f.Create();
    REQUIRE(dec != nullptr);
}

TEST_CASE("IStreamingDecoderFactory: GetFormatId + GetPriority match", "[factory]") {
    StubDecoderFactory f;
    CHECK_FALSE(f.GetFormatId().empty());
    CHECK(f.GetPriority() >= 0);
    CHECK(f.GetPriority() <= 100);
}

// ---------------------------------------------------------------------------
// Tests: ProbeResult / DecodedThumb value semantics
// ---------------------------------------------------------------------------

TEST_CASE("ProbeResult: default status is UNKNOWN", "[probe-result]") {
    ProbeResult r{};
    CHECK(r.status == ProbeStatus::UNKNOWN);
    CHECK(r.width  == 0);
    CHECK(r.height == 0);
    CHECK(r.frameCount == 0);
}

TEST_CASE("DecodedThumb: default status is UNKNOWN", "[decode-result]") {
    DecodedThumb t{};
    CHECK(t.status == DecodeStatus::UNKNOWN);
    CHECK(t.pixels.empty());
}

TEST_CASE("DecodedThumb: copy of pixels is independent", "[decode-result]") {
    auto a = MakeOkThumb(16, 16);
    auto b = a;
    b.pixels[0] = 0xAB;
    CHECK(a.pixels[0] != 0xAB);
}

// ---------------------------------------------------------------------------
// Tests: ColorSpace enum completeness
// ---------------------------------------------------------------------------

TEST_CASE("ColorSpace enum values are distinct", "[colorspace]") {
    CHECK(static_cast<int>(ColorSpace::SRGB)        != static_cast<int>(ColorSpace::LINEAR_RGB));
    CHECK(static_cast<int>(ColorSpace::LINEAR_RGB)  != static_cast<int>(ColorSpace::CMYK));
    CHECK(static_cast<int>(ColorSpace::CMYK)        != static_cast<int>(ColorSpace::GRAYSCALE));
    CHECK(static_cast<int>(ColorSpace::GRAYSCALE)   != static_cast<int>(ColorSpace::UNKNOWN));
}

TEST_CASE("PixelFormat enum values are distinct", "[pixelformat]") {
    CHECK(static_cast<int>(PixelFormat::BGRA32) != static_cast<int>(PixelFormat::RGBA32));
    CHECK(static_cast<int>(PixelFormat::RGBA32) != static_cast<int>(PixelFormat::RGB24));
    CHECK(static_cast<int>(PixelFormat::RGB24)  != static_cast<int>(PixelFormat::GRAY8));
}
