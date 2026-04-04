// VDBVolumeDecoder.h — OpenVDB Volumetric Data Preview
// Copyright (c) 2026 ExplorerLens Project
//
// OpenVDB volumetric data preview decoder. Detects VDB magic bytes, generates
// density slice thumbnails from volumetric grid data.
//
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class VDBGridType : uint8_t {
    Float,
    Double,
    Vec3s,
    Int32,
    Bool,
    Unknown
};

struct VDBGridInfo
{
    std::string name;
    VDBGridType type = VDBGridType::Unknown;
    uint64_t voxelCount = 0;
    std::array<double, 3> bboxMin = {0.0, 0.0, 0.0};
    std::array<double, 3> bboxMax = {0.0, 0.0, 0.0};
};

struct VDBFileInfo
{
    uint64_t magic = 0;
    uint32_t version = 0;
    bool hasGrids = false;
    uint32_t gridCount = 0;
    std::vector<VDBGridInfo> grids;
    bool isValid = false;
};

class VDBVolumeDecoder
{
  public:
    static constexpr uint64_t VDB_MAGIC = 0x56444220;

    static VDBVolumeDecoder& Instance()
    {
        static VDBVolumeDecoder instance;
        return instance;
    }

    inline bool IsVDBFile(const uint8_t* data, size_t size) const
    {
        if (!data || size < 8)
            return false;
        uint64_t magic = 0;
        std::memcpy(&magic, data, sizeof(uint64_t));
        return (magic & 0xFFFFFFFF) == VDB_MAGIC
               || (data[0] == 0x20 && data[1] == 0x42 && data[2] == 0x44 && data[3] == 0x56);
    }

    inline VDBFileInfo ParseHeader(const uint8_t* data, size_t size) const
    {
        VDBFileInfo info;
        if (!data || size < 16)
            return info;

        std::memcpy(&info.magic, data, 8);
        info.isValid = IsVDBFile(data, size);
        if (!info.isValid)
            return info;

        if (size >= 12) {
            info.version = static_cast<uint32_t>(data[8]) | (static_cast<uint32_t>(data[9]) << 8)
                           | (static_cast<uint32_t>(data[10]) << 16) | (static_cast<uint32_t>(data[11]) << 24);
        }

        info.hasGrids = size > 64;
        info.gridCount = info.hasGrids ? 1 : 0;
        return info;
    }

    inline std::vector<uint8_t> GenerateDensitySlice(const float* densityData, uint32_t dimX, uint32_t dimY,
                                                     uint32_t dimZ, uint32_t sliceZ) const
    {
        std::vector<uint8_t> slice(static_cast<size_t>(dimX) * dimY, 0);
        if (!densityData || dimX == 0 || dimY == 0 || dimZ == 0)
            return slice;

        sliceZ = (std::min)(sliceZ, dimZ - 1);

        float minVal = 1e30f, maxVal = -1e30f;
        size_t sliceOffset = static_cast<size_t>(sliceZ) * dimX * dimY;
        for (uint32_t y = 0; y < dimY; ++y) {
            for (uint32_t x = 0; x < dimX; ++x) {
                float v = densityData[sliceOffset + y * dimX + x];
                if (v < minVal)
                    minVal = v;
                if (v > maxVal)
                    maxVal = v;
            }
        }

        float range = maxVal - minVal;
        if (range < 1e-8f)
            range = 1.0f;

        for (uint32_t y = 0; y < dimY; ++y) {
            for (uint32_t x = 0; x < dimX; ++x) {
                float v = densityData[sliceOffset + y * dimX + x];
                float normalized = (v - minVal) / range;
                slice[static_cast<size_t>(y) * dimX + x] =
                    static_cast<uint8_t>((std::max)(0.0f, (std::min)(1.0f, normalized)) * 255.0f);
            }
        }
        return slice;
    }

    inline std::vector<uint8_t> GenerateMaxIntensityProjection(const float* densityData, uint32_t dimX, uint32_t dimY,
                                                               uint32_t dimZ) const
    {
        std::vector<uint8_t> projection(static_cast<size_t>(dimX) * dimY, 0);
        if (!densityData || dimX == 0 || dimY == 0 || dimZ == 0)
            return projection;

        std::vector<float> maxValues(static_cast<size_t>(dimX) * dimY, -1e30f);
        float globalMin = 1e30f, globalMax = -1e30f;

        for (uint32_t z = 0; z < dimZ; ++z) {
            size_t sliceOffset = static_cast<size_t>(z) * dimX * dimY;
            for (uint32_t y = 0; y < dimY; ++y) {
                for (uint32_t x = 0; x < dimX; ++x) {
                    float v = densityData[sliceOffset + y * dimX + x];
                    size_t idx = static_cast<size_t>(y) * dimX + x;
                    if (v > maxValues[idx])
                        maxValues[idx] = v;
                }
            }
        }

        for (auto v : maxValues) {
            if (v < globalMin)
                globalMin = v;
            if (v > globalMax)
                globalMax = v;
        }

        float range = globalMax - globalMin;
        if (range < 1e-8f)
            range = 1.0f;

        for (size_t i = 0; i < maxValues.size(); ++i) {
            float normalized = (maxValues[i] - globalMin) / range;
            projection[i] = static_cast<uint8_t>((std::max)(0.0f, (std::min)(1.0f, normalized)) * 255.0f);
        }
        return projection;
    }

    inline std::string GridTypeToString(VDBGridType type) const
    {
        switch (type) {
            case VDBGridType::Float:
                return "Float";
            case VDBGridType::Double:
                return "Double";
            case VDBGridType::Vec3s:
                return "Vec3s";
            case VDBGridType::Int32:
                return "Int32";
            case VDBGridType::Bool:
                return "Bool";
            default:
                return "Unknown";
        }
    }

  private:
    VDBVolumeDecoder() = default;
};

}  // namespace Engine
}  // namespace ExplorerLens
