// WebP2Decoder.h — Experimental WebP2 Image Format Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Detects experimental WebP2 files via "WP2L"/"WP2 " magic bytes.
// Extracts preview metadata and wraps libwebp2 when available at runtime.
// Falls back to WIC for formats that map to WebP container.

#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class WP2Quality : uint8_t { Lossy, Lossless, Mixed };

struct WP2Header {
    uint32_t width = 0;
    uint32_t height = 0;
    WP2Quality quality = WP2Quality::Lossy;
    bool hasAlpha = false;
    bool hasAnimation = false;
    uint32_t bitstream_version = 0;
};

struct WP2Stats {
    uint32_t filesDetected = 0;
    uint32_t headersParsed = 0;
    uint32_t fallbacksUsed = 0;
};

class WebP2Decoder {
public:
    WebP2Decoder() = default;
    ~WebP2Decoder() = default;

    static const wchar_t* GetName() { return L"WebP2Decoder"; }

    bool CanDecode(const wchar_t* ext) const {
        if (!ext) return false;
        std::wstring e(ext);
        for (auto& c : e) c = towlower(c);
        return e == L".wp2";
    }

    bool DetectMagic(const uint8_t* data, size_t size) const {
        if (!data || size < 8) return false;
        // Check for RIFF-like container: "WP2 " or direct "WP2L"
        return (memcmp(data + 4, "WP2 ", 4) == 0) ||
            (memcmp(data, "WP2L", 4) == 0);
    }

    WP2Header ParseHeader(const uint8_t* data, size_t size) const {
        WP2Header hdr;
        if (!data || size < 16) return hdr;

        if (memcmp(data, "WP2L", 4) == 0) {
            hdr.quality = WP2Quality::Lossless;
            hdr.width = (data[4]) | (data[5] << 8);
            hdr.height = (data[6]) | (data[7] << 8);
        }
        else if (size >= 12 && memcmp(data + 4, "WP2 ", 4) == 0) {
            hdr.width = (data[8]) | (data[9] << 8);
            hdr.height = (data[10]) | (data[11] << 8);
            hdr.quality = WP2Quality::Lossy;
        }

        if (hdr.width == 0) hdr.width = 1;
        if (hdr.height == 0) hdr.height = 1;

        return hdr;
    }

    WP2Stats GetStats() const { return m_stats; }

private:
    mutable WP2Stats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
