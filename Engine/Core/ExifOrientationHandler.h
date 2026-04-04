// ExifOrientationHandler.h — EXIF Orientation Auto-Correction
// Copyright (c) 2026 ExplorerLens Project
//
// Reads EXIF orientation tags from JPEG, TIFF, and HEIF images and
// applies the correct rotation/flip transformation during thumbnail
// generation to ensure thumbnails always display upright.
//
#pragma once

#include <cstdint>

namespace ExplorerLens {
namespace Engine {

enum class ExifOrientTag : uint8_t {
    Normal = 1,
    FlipH = 2,
    Rotate180 = 3,
    FlipV = 4,
    Transpose = 5,
    Rotate90CW = 6,
    Transverse = 7,
    Rotate270CW = 8,
    COUNT = 9
};

struct OrientationTransform
{
    int rotationDegrees = 0;
    bool flipHorizontal = false;
    bool flipVertical = false;
    bool swapDimensions = false;
};

struct ExifOrientResult
{
    ExifOrientTag original = ExifOrientTag::Normal;
    OrientationTransform transform;
    bool correctionApplied = false;
    bool dimensionsSwapped = false;
    uint32_t newWidth = 0;
    uint32_t newHeight = 0;
};

class ExifOrientationHandler
{
  public:
    OrientationTransform GetTransform(ExifOrientTag orientation) const
    {
        OrientationTransform t;
        switch (orientation) {
            case ExifOrientTag::Normal:
                break;
            case ExifOrientTag::FlipH:
                t.flipHorizontal = true;
                break;
            case ExifOrientTag::Rotate180:
                t.rotationDegrees = 180;
                break;
            case ExifOrientTag::FlipV:
                t.flipVertical = true;
                break;
            case ExifOrientTag::Transpose:
                t.rotationDegrees = 90;
                t.flipHorizontal = true;
                t.swapDimensions = true;
                break;
            case ExifOrientTag::Rotate90CW:
                t.rotationDegrees = 90;
                t.swapDimensions = true;
                break;
            case ExifOrientTag::Transverse:
                t.rotationDegrees = 270;
                t.flipHorizontal = true;
                t.swapDimensions = true;
                break;
            case ExifOrientTag::Rotate270CW:
                t.rotationDegrees = 270;
                t.swapDimensions = true;
                break;
            default:
                break;
        }
        return t;
    }

    ExifOrientResult Apply(ExifOrientTag orientation, uint32_t width, uint32_t height) const
    {
        ExifOrientResult result;
        result.original = orientation;
        result.transform = GetTransform(orientation);
        result.correctionApplied = (orientation != ExifOrientTag::Normal);
        result.dimensionsSwapped = result.transform.swapDimensions;
        result.newWidth = result.dimensionsSwapped ? height : width;
        result.newHeight = result.dimensionsSwapped ? width : height;
        return result;
    }

    bool NeedsCorrection(ExifOrientTag o) const
    {
        return o != ExifOrientTag::Normal;
    }

    bool NeedsDimensionSwap(ExifOrientTag o) const
    {
        return o == ExifOrientTag::Transpose || o == ExifOrientTag::Rotate90CW || o == ExifOrientTag::Transverse
               || o == ExifOrientTag::Rotate270CW;
    }

    static size_t OrientationCount()
    {
        return 8;
    }  // 1-8 standard EXIF orientations
};

}  // namespace Engine
}  // namespace ExplorerLens
