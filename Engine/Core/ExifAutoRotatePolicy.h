// ============================================================================
// ExifAutoRotatePolicy.h -- S253 / ROADMAP v6.0 H19
//
// EXIF-aware auto-rotate policy baked into every decoder output (H19 from
// digiKam).  Header-only.  The rotation itself is applied by each decoder
// (LibRaw, libjpeg-turbo, libheif) using its native fast-path when available,
// otherwise by GDI+ / D3D11 in the resize stage.
//
// NOTE: there is already an `ExifOrientation` enum in ExifOrientationFixer.h.
// This header adds the *policy* types (what transform to apply, in which stage,
// for which decoder family) without duplicating the enum.  Consumers pull in
// both headers.
// ============================================================================
#pragma once

#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

// Which pipeline stage applies the rotation -- decoders should prefer NATIVE
// when the backing library can rotate during decode (libjpeg-turbo transform,
// LibRaw flip).  Stage = NONE means the source is already correctly oriented.
enum class ExifAutoRotateStage : uint8_t
{
    NONE               = 0,
    NATIVE_IN_DECODER  = 1,   // Decoder rotates while producing pixels
    POST_DECODE_CPU    = 2,   // CPU pixel rotate after decode (GDI+ / software)
    POST_DECODE_GPU    = 3,   // D3D11 resize pass folds rotation in
};

// Which concrete geometric transform is needed.  1:1 with the 8 EXIF values.
enum class ExifAutoRotateTransform : uint8_t
{
    IDENTITY           = 0,   // EXIF 1
    FLIP_HORIZONTAL    = 1,   // EXIF 2
    ROTATE_180         = 2,   // EXIF 3
    FLIP_VERTICAL      = 3,   // EXIF 4
    TRANSPOSE          = 4,   // EXIF 5
    ROTATE_90_CW       = 5,   // EXIF 6
    TRANSVERSE         = 6,   // EXIF 7
    ROTATE_270_CW      = 7,   // EXIF 8
};

struct ExifAutoRotatePolicy
{
    ExifAutoRotateStage     preferredStage = ExifAutoRotateStage::NATIVE_IN_DECODER;
    ExifAutoRotateTransform transform      = ExifAutoRotateTransform::IDENTITY;
    bool                    writeAutoRotatedFlag = true;   // advertise to cache
};

// Map from the raw 1..8 EXIF orientation tag to our transform enum.  Any out-
// of-range tag maps to IDENTITY (defensive -- never throw from decoder path).
constexpr ExifAutoRotateTransform ExifTagToTransform(int exifTag) noexcept
{
    switch (exifTag) {
        case 1:  return ExifAutoRotateTransform::IDENTITY;
        case 2:  return ExifAutoRotateTransform::FLIP_HORIZONTAL;
        case 3:  return ExifAutoRotateTransform::ROTATE_180;
        case 4:  return ExifAutoRotateTransform::FLIP_VERTICAL;
        case 5:  return ExifAutoRotateTransform::TRANSPOSE;
        case 6:  return ExifAutoRotateTransform::ROTATE_90_CW;
        case 7:  return ExifAutoRotateTransform::TRANSVERSE;
        case 8:  return ExifAutoRotateTransform::ROTATE_270_CW;
        default: return ExifAutoRotateTransform::IDENTITY;
    }
}

// Whether the transform swaps width and height.
constexpr bool ExifTransformSwapsDims(ExifAutoRotateTransform t) noexcept
{
    switch (t) {
        case ExifAutoRotateTransform::TRANSPOSE:
        case ExifAutoRotateTransform::ROTATE_90_CW:
        case ExifAutoRotateTransform::TRANSVERSE:
        case ExifAutoRotateTransform::ROTATE_270_CW:
            return true;
        default:
            return false;
    }
}

static_assert(std::is_trivially_copyable_v<ExifAutoRotatePolicy>,
              "ExifAutoRotatePolicy must be trivially copyable");
static_assert(ExifTagToTransform(6) == ExifAutoRotateTransform::ROTATE_90_CW,
              "EXIF tag 6 must map to 90 CW");
static_assert(ExifTagToTransform(99) == ExifAutoRotateTransform::IDENTITY,
              "Out-of-range EXIF must be defensive IDENTITY");
static_assert(ExifTransformSwapsDims(ExifAutoRotateTransform::ROTATE_90_CW),
              "Rotate 90 CW must swap dims");
static_assert(!ExifTransformSwapsDims(ExifAutoRotateTransform::FLIP_HORIZONTAL),
              "Flip horizontal must not swap dims");

} // namespace ExplorerLens::Engine
