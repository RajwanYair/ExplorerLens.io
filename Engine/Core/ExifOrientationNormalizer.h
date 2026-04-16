// ExifOrientationNormalizer.h — EXIF orientation tag → correct pixel rotation
// Copyright (c) 2026 ExplorerLens Project
//
// EXIF orientation values 1-8 describe how the camera was held when shooting.
// Thumbnails must be rotated/flipped to match visual expectation regardless of
// the underlying decode library's handling.
//
// This header is intentionally standalone (no external lib dependency); it
// performs in-place BGRA32 or BGRA64 pixel rotation using swap operations.
//
#pragma once

#include <cstdint>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ---------------------------------------------------------------------------
// ExifOrientation — EXIF orientation tag values (TIFF/EXIF spec §4.6.4)
// ---------------------------------------------------------------------------

enum class ExifOrientation : uint16_t {
    NORMAL            = 1,   // Top-left (no transform needed)
    FLIP_HORIZONTAL   = 2,
    ROTATE_180        = 3,
    FLIP_VERTICAL     = 4,
    TRANSPOSE         = 5,   // Rotate 90 CW + flip horizontal
    ROTATE_90_CW      = 6,
    TRANSVERSE        = 7,   // Rotate 90 CCW + flip horizontal
    ROTATE_90_CCW     = 8,
};

// ---------------------------------------------------------------------------
// PixelBuffer — thin view over an pixel array
// ---------------------------------------------------------------------------

struct PixelBuffer {
    uint8_t* pixels  = nullptr;
    uint32_t width   = 0;
    uint32_t height  = 0;
    uint32_t stride  = 0;   // bytes per row
    uint32_t bpp     = 32;  // 32 = BGRA8, 64 = BGRA16
};

// ---------------------------------------------------------------------------
// ExifOrientationNormalizer
// ---------------------------------------------------------------------------

class ExifOrientationNormalizer {
public:
    // Apply in-place orientation correction to a pixel buffer.
    // After this call, buf.pixels / width / height reflect the corrected image.
    // The pixels vector is resized/replaced if the transform changes dimensions.
    //
    // Returns true on success, false if orientation is unrecognised or
    // the buffer dimensions are inconsistent.
    static bool Normalize(PixelBuffer& buf, ExifOrientation orientation);

    // Parse EXIF orientation from a raw JPEG/TIFF EXIF block.
    // Returns ExifOrientation::NORMAL if tag is absent or unreadable.
    static ExifOrientation ParseFromExif(const uint8_t* exifData, size_t exifSize);

    // Parse EXIF orientation directly from a JPEG file header (scans APP1 markers).
    static ExifOrientation ParseFromJpeg(const uint8_t* jpegData, size_t jpegSize);

private:
    // Low-level transforms
    static void Rotate90CW  (PixelBuffer& buf);
    static void Rotate90CCW (PixelBuffer& buf);
    static void Rotate180   (PixelBuffer& buf);
    static void FlipH       (PixelBuffer& buf);
    static void FlipV       (PixelBuffer& buf);
    static void Transpose   (PixelBuffer& buf);
    static void Transverse  (PixelBuffer& buf);

    static uint32_t BytesPerPixel(uint32_t bpp) noexcept { return bpp / 8; }
};

} // namespace Engine
} // namespace ExplorerLens
