// VolumetricPreviewEngine.h — 3D Volumetric Data Preview
// Copyright (c) 2026 ExplorerLens Project
//
// 3D volumetric data preview for DICOM/NIfTI datasets. Extracts representative
// slices from volume data and generates 2D projection thumbnails.
//
#pragma once

#include <cstdint>
#include <vector>
#include <array>
#include <algorithm>
#include <cmath>
#include <string>
#include <numeric>

namespace ExplorerLens {
namespace Engine {

enum class SliceAxis : uint8_t {
    Axial,
    Coronal,
    Sagittal
};

enum class ProjectionMode : uint8_t {
    MaxIntensityProjection,
    AverageIntensityProjection,
    MinIntensityProjection,
    CentralSlice
};

struct VolumeInfo {
    uint32_t dimX = 0;
    uint32_t dimY = 0;
    uint32_t dimZ = 0;
    double spacingX = 1.0;
    double spacingY = 1.0;
    double spacingZ = 1.0;
    double windowCenter = 0.0;
    double windowWidth = 400.0;
};

class VolumetricPreviewEngine {
public:
    static VolumetricPreviewEngine& Instance() {
        static VolumetricPreviewEngine instance;
        return instance;
    }

    inline std::vector<uint8_t> ExtractSlice(const int16_t* volumeData, const VolumeInfo& info,
        SliceAxis axis, uint32_t sliceIndex) const {
        uint32_t outW = 0, outH = 0;
        GetSliceDimensions(info, axis, outW, outH);
        std::vector<uint8_t> slice(static_cast<size_t>(outW) * outH, 0);

        if (!volumeData || outW == 0 || outH == 0) return slice;

        sliceIndex = (std::min)(sliceIndex, GetSliceCount(info, axis) - 1);

        for (uint32_t y = 0; y < outH; ++y) {
            for (uint32_t x = 0; x < outW; ++x) {
                int16_t voxel = SampleVoxel(volumeData, info, axis, sliceIndex, x, y);
                slice[static_cast<size_t>(y) * outW + x] = ApplyWindowing(voxel, info.windowCenter, info.windowWidth);
            }
        }
        return slice;
    }

    inline std::vector<uint8_t> GenerateProjection(const int16_t* volumeData, const VolumeInfo& info,
        SliceAxis axis, ProjectionMode mode) const {
        uint32_t outW = 0, outH = 0;
        GetSliceDimensions(info, axis, outW, outH);
        uint32_t depth = GetSliceCount(info, axis);
        std::vector<uint8_t> output(static_cast<size_t>(outW) * outH, 0);

        if (!volumeData || outW == 0 || outH == 0 || depth == 0) return output;

        for (uint32_t y = 0; y < outH; ++y) {
            for (uint32_t x = 0; x < outW; ++x) {
                double accumulator = 0.0;
                int16_t maxVal = -32768;
                int16_t minVal = 32767;

                for (uint32_t d = 0; d < depth; ++d) {
                    int16_t voxel = SampleVoxel(volumeData, info, axis, d, x, y);
                    accumulator += voxel;
                    if (voxel > maxVal) maxVal = voxel;
                    if (voxel < minVal) minVal = voxel;
                }

                int16_t projected = 0;
                switch (mode) {
                case ProjectionMode::MaxIntensityProjection:
                    projected = maxVal; break;
                case ProjectionMode::AverageIntensityProjection:
                    projected = static_cast<int16_t>(accumulator / depth); break;
                case ProjectionMode::MinIntensityProjection:
                    projected = minVal; break;
                case ProjectionMode::CentralSlice:
                    projected = SampleVoxel(volumeData, info, axis, depth / 2, x, y); break;
                }
                output[static_cast<size_t>(y) * outW + x] = ApplyWindowing(projected, info.windowCenter, info.windowWidth);
            }
        }
        return output;
    }

    inline void GetSliceDimensions(const VolumeInfo& info, SliceAxis axis,
        uint32_t& outWidth, uint32_t& outHeight) const {
        switch (axis) {
        case SliceAxis::Axial:    outWidth = info.dimX; outHeight = info.dimY; break;
        case SliceAxis::Coronal:  outWidth = info.dimX; outHeight = info.dimZ; break;
        case SliceAxis::Sagittal: outWidth = info.dimY; outHeight = info.dimZ; break;
        }
    }

    inline uint32_t GetSliceCount(const VolumeInfo& info, SliceAxis axis) const {
        switch (axis) {
        case SliceAxis::Axial:    return info.dimZ;
        case SliceAxis::Coronal:  return info.dimY;
        case SliceAxis::Sagittal: return info.dimX;
        }
        return 0;
    }

private:
    VolumetricPreviewEngine() = default;

    inline int16_t SampleVoxel(const int16_t* data, const VolumeInfo& info,
        SliceAxis axis, uint32_t slice, uint32_t x, uint32_t y) const {
        size_t vx = 0, vy = 0, vz = 0;
        switch (axis) {
        case SliceAxis::Axial:    vx = x; vy = y; vz = slice; break;
        case SliceAxis::Coronal:  vx = x; vy = slice; vz = y; break;
        case SliceAxis::Sagittal: vx = slice; vy = x; vz = y; break;
        }
        size_t index = vz * info.dimX * info.dimY + vy * info.dimX + vx;
        return data[index];
    }

    inline uint8_t ApplyWindowing(int16_t value, double center, double width) const {
        double lower = center - width / 2.0;
        double upper = center + width / 2.0;
        double v = static_cast<double>(value);
        if (v <= lower) return 0;
        if (v >= upper) return 255;
        return static_cast<uint8_t>(((v - lower) / width) * 255.0);
    }
};

}
} // namespace ExplorerLens::Engine
