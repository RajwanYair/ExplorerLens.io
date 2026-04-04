// PlatformNeutralBuffer.h — Platform-Neutral Pixel Buffer
// Copyright (c) 2026 ExplorerLens Project
//
// Abstracts RGBA pixel buffer ownership and transfer across Win32, macOS,
// and Linux backends. Provides zero-copy views and ownership transfer semantics.
//
#pragma once
#include <cstdint>
#include <cstring>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class PixelFormat {
    RGBA8,
    BGRA8,
    RGB8,
    Gray8,
    RGBA16F
};

struct PlatformNeutralBuffer
{
    std::vector<uint8_t> pixels;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t stride = 0;
    PixelFormat format = PixelFormat::RGBA8;

    bool IsValid() const
    {
        return width > 0 && height > 0 && !pixels.empty() && pixels.size() >= static_cast<size_t>(stride) * height;
    }

    uint32_t BytesPerPixel() const
    {
        switch (format) {
            case PixelFormat::RGBA8:
                return 4;
            case PixelFormat::BGRA8:
                return 4;
            case PixelFormat::RGB8:
                return 3;
            case PixelFormat::Gray8:
                return 1;
            case PixelFormat::RGBA16F:
                return 8;
            default:
                return 4;
        }
    }

    static PlatformNeutralBuffer Create(uint32_t w, uint32_t h, PixelFormat fmt = PixelFormat::RGBA8)
    {
        PlatformNeutralBuffer b;
        b.width = w;
        b.height = h;
        b.format = fmt;
        uint32_t bpp = b.BytesPerPixel();
        b.stride = w * bpp;
        b.pixels.assign(static_cast<size_t>(b.stride) * h, 0);
        return b;
    }

    PlatformNeutralBuffer Clone() const
    {
        PlatformNeutralBuffer c;
        c.width = width;
        c.height = height;
        c.stride = stride;
        c.format = format;
        c.pixels = pixels;
        return c;
    }

    const uint8_t* Row(uint32_t y) const
    {
        return pixels.data() + static_cast<size_t>(y) * stride;
    }
    uint8_t* Row(uint32_t y)
    {
        return pixels.data() + static_cast<size_t>(y) * stride;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
