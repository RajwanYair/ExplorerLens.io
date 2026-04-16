// ExifOrientationNormalizer.cpp — in-place BGRA pixel rotation
// Copyright (c) 2026 ExplorerLens Project
//
#include "ExifOrientationNormalizer.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ---------------------------------------------------------------------------
// Rotate 90° clockwise: (x, y) → (H-1-y, x)  new dims: (H, W)
// ---------------------------------------------------------------------------
void ExifOrientationNormalizer::Rotate90CW(PixelBuffer& buf)
{
    const uint32_t W = buf.width, H = buf.height;
    const uint32_t bpp = BytesPerPixel(buf.bpp);
    std::vector<uint8_t> dst(static_cast<size_t>(H) * W * bpp);

    for (uint32_t y = 0; y < H; ++y) {
        for (uint32_t x = 0; x < W; ++x) {
            // src pixel at (x,y) → dst at (H-1-y, x)
            const uint8_t* src = buf.pixels + y * buf.stride + x * bpp;
            uint8_t*       d   = dst.data() + x * H * bpp + (H - 1 - y) * bpp;
            std::memcpy(d, src, bpp);
        }
    }
    std::memcpy(buf.pixels, dst.data(), dst.size());
    buf.stride = H * bpp;
    std::swap(buf.width, buf.height);
}

// ---------------------------------------------------------------------------
// Rotate 90° counter-clockwise: (x, y) → (y, W-1-x)  new dims: (H, W)
// ---------------------------------------------------------------------------
void ExifOrientationNormalizer::Rotate90CCW(PixelBuffer& buf)
{
    const uint32_t W = buf.width, H = buf.height;
    const uint32_t bpp = BytesPerPixel(buf.bpp);
    std::vector<uint8_t> dst(static_cast<size_t>(H) * W * bpp);

    for (uint32_t y = 0; y < H; ++y) {
        for (uint32_t x = 0; x < W; ++x) {
            const uint8_t* src = buf.pixels + y * buf.stride + x * bpp;
            uint8_t*       d   = dst.data() + (W - 1 - x) * H * bpp + y * bpp;
            std::memcpy(d, src, bpp);
        }
    }
    std::memcpy(buf.pixels, dst.data(), dst.size());
    buf.stride = H * bpp;
    std::swap(buf.width, buf.height);
}

// ---------------------------------------------------------------------------
// Rotate 180°: reverse row order + reverse each row
// ---------------------------------------------------------------------------
void ExifOrientationNormalizer::Rotate180(PixelBuffer& buf)
{
    FlipH(buf);
    FlipV(buf);
}

// ---------------------------------------------------------------------------
// Flip horizontal (mirror left-right)
// ---------------------------------------------------------------------------
void ExifOrientationNormalizer::FlipH(PixelBuffer& buf)
{
    const uint32_t bpp = BytesPerPixel(buf.bpp);
    for (uint32_t y = 0; y < buf.height; ++y) {
        uint8_t* row = buf.pixels + y * buf.stride;
        uint8_t* lo  = row;
        uint8_t* hi  = row + (buf.width - 1) * bpp;
        while (lo < hi) {
            for (uint32_t b = 0; b < bpp; ++b) std::swap(lo[b], hi[b]);
            lo += bpp;
            hi -= bpp;
        }
    }
}

// ---------------------------------------------------------------------------
// Flip vertical (mirror top-bottom)
// ---------------------------------------------------------------------------
void ExifOrientationNormalizer::FlipV(PixelBuffer& buf)
{
    const uint32_t rowBytes = buf.width * BytesPerPixel(buf.bpp);
    std::vector<uint8_t> tmp(rowBytes);
    uint32_t top = 0, bot = buf.height - 1;
    while (top < bot) {
        uint8_t* Rtop = buf.pixels + top * buf.stride;
        uint8_t* Rbot = buf.pixels + bot * buf.stride;
        std::memcpy(tmp.data(), Rtop, rowBytes);
        std::memcpy(Rtop, Rbot, rowBytes);
        std::memcpy(Rbot, tmp.data(), rowBytes);
        ++top; --bot;
    }
}

// ---------------------------------------------------------------------------
// Transpose: (x, y) → (y, x)
// ---------------------------------------------------------------------------
void ExifOrientationNormalizer::Transpose(PixelBuffer& buf)
{
    Rotate90CW(buf);
    FlipV(buf);
}

// ---------------------------------------------------------------------------
// Transverse: (x, y) → (H-1-y, W-1-x)
// ---------------------------------------------------------------------------
void ExifOrientationNormalizer::Transverse(PixelBuffer& buf)
{
    Rotate90CCW(buf);
    FlipV(buf);
}

// ---------------------------------------------------------------------------
// Normalize — apply the orientation correction
// ---------------------------------------------------------------------------
bool ExifOrientationNormalizer::Normalize(PixelBuffer& buf, ExifOrientation orientation)
{
    if (!buf.pixels || buf.width == 0 || buf.height == 0) return false;

    switch (orientation) {
        case ExifOrientation::NORMAL:            return true;
        case ExifOrientation::FLIP_HORIZONTAL:   FlipH(buf);       return true;
        case ExifOrientation::ROTATE_180:        Rotate180(buf);   return true;
        case ExifOrientation::FLIP_VERTICAL:     FlipV(buf);       return true;
        case ExifOrientation::TRANSPOSE:         Transpose(buf);   return true;
        case ExifOrientation::ROTATE_90_CW:      Rotate90CW(buf);  return true;
        case ExifOrientation::TRANSVERSE:        Transverse(buf);  return true;
        case ExifOrientation::ROTATE_90_CCW:     Rotate90CCW(buf); return true;
        default:                                                    return false;
    }
}

// ---------------------------------------------------------------------------
// ParseFromExif — read orientation from raw EXIF/TIFF bytes
// ---------------------------------------------------------------------------
static uint16_t ReadU16LE(const uint8_t* p) {
    return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
}
static uint16_t ReadU16BE(const uint8_t* p) {
    return (static_cast<uint16_t>(p[0]) << 8) | static_cast<uint16_t>(p[1]);
}
static uint32_t ReadU32LE(const uint8_t* p) {
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}
static uint32_t ReadU32BE(const uint8_t* p) {
    return (static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) |
           (static_cast<uint32_t>(p[2]) << 8) | static_cast<uint32_t>(p[3]);
}

ExifOrientation ExifOrientationNormalizer::ParseFromExif(const uint8_t* data, size_t size)
{
    if (!data || size < 8) return ExifOrientation::NORMAL;

    // TIFF header: byte order mark
    bool littleEndian = (data[0] == 0x49 && data[1] == 0x49);
    bool bigEndian    = (data[0] == 0x4D && data[1] == 0x4D);
    if (!littleEndian && !bigEndian) return ExifOrientation::NORMAL;

    auto r16 = littleEndian ? ReadU16LE : ReadU16BE;
    auto r32 = littleEndian ? ReadU32LE : ReadU32BE;

    uint16_t magic = r16(data + 2);
    if (magic != 42) return ExifOrientation::NORMAL;  // TIFF magic

    uint32_t ifdOffset = r32(data + 4);
    if (ifdOffset + 2 > size) return ExifOrientation::NORMAL;

    uint16_t numEntries = r16(data + ifdOffset);
    for (uint16_t i = 0; i < numEntries; ++i) {
        size_t pos = ifdOffset + 2 + static_cast<size_t>(i) * 12;
        if (pos + 12 > size) break;

        uint16_t tag = r16(data + pos);
        if (tag != 0x0112) continue;  // Orientation tag

        // type(2) = SHORT, count(4) = 1, value(2) in offset field
        uint16_t val = r16(data + pos + 8);
        if (val >= 1 && val <= 8) return static_cast<ExifOrientation>(val);
        return ExifOrientation::NORMAL;
    }
    return ExifOrientation::NORMAL;
}

ExifOrientation ExifOrientationNormalizer::ParseFromJpeg(const uint8_t* jpegData, size_t jpegSize)
{
    if (!jpegData || jpegSize < 4) return ExifOrientation::NORMAL;
    if (jpegData[0] != 0xFF || jpegData[1] != 0xD8) return ExifOrientation::NORMAL;

    size_t pos = 2;
    while (pos + 4 <= jpegSize) {
        if (jpegData[pos] != 0xFF) break;
        uint8_t marker = jpegData[pos + 1];
        uint16_t len = static_cast<uint16_t>((static_cast<uint16_t>(jpegData[pos + 2]) << 8)
                       | jpegData[pos + 3]);
        if (len < 2) break;

        if (marker == 0xE1 && pos + 2 + len <= jpegSize) {
            // APP1 — EXIF header: "Exif\0\0"
            const uint8_t* app1 = jpegData + pos + 4;
            size_t app1len = static_cast<size_t>(len) - 2;
            if (app1len > 6 && std::memcmp(app1, "Exif\0\0", 6) == 0) {
                return ParseFromExif(app1 + 6, app1len - 6);
            }
        }
        pos += 2 + static_cast<size_t>(len);
        if (marker == 0xDA) break; // SOS
    }
    return ExifOrientation::NORMAL;
}

} // namespace Engine
} // namespace ExplorerLens
