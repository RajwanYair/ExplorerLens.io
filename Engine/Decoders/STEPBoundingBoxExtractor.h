// STEPBoundingBoxExtractor.h — ISO 10303 STEP/IGES Bounding-Box Extractor
// Copyright (c) 2026 ExplorerLens Project
//
// Scans STEP (AP203/AP214/AP242) and IGES files for CARTESIAN_POINT,
// VERTEX_POINT, and IGES entity-112/116 geometry records to compute
// an axis-aligned bounding box. Renders a 3-axis wireframe preview.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <array>

namespace ExplorerLens {
namespace Engine {

struct BoundingBox3D
{
    double minX, minY, minZ;
    double maxX, maxY, maxZ;
    bool   valid;

    double SizeX() const noexcept { return maxX - minX; }
    double SizeY() const noexcept { return maxY - minY; }
    double SizeZ() const noexcept { return maxZ - minZ; }
};

enum class CADFileFormat : uint8_t
{
    Unknown = 0,
    STEP,    // ISO 10303
    IGES,    // ASME Y14.26M
};

class STEPBoundingBoxExtractor
{
public:
    // Returns STEP if data starts with "ISO-10303" line; IGES if it has 80-col fixed record.
    static CADFileFormat DetectFormat(const uint8_t* data, size_t size) noexcept;

    // Extract bounding box by scanning CARTESIAN_POINT / VERTEX_POINT records.
    // Large files are sub-sampled (every Nth line, N = file/65536 + 1).
    static BoundingBox3D ExtractSTEP(const uint8_t* data, size_t size) noexcept;

    // Extract bounding box from IGES entity type 116 (POINT) and 110 (LINE) records.
    static BoundingBox3D ExtractIGES(const uint8_t* data, size_t size) noexcept;

    // Dispatch to the correct extractor based on format.
    static BoundingBox3D Extract(const uint8_t* data, size_t size) noexcept;

    // Render a 256×256 BGRA32 isometric wireframe box from the bounding box.
    // Returns an 80% grey gradient background with white axis lines.
    static std::vector<uint8_t> RenderBBoxPreview(const BoundingBox3D& bbox,
                                                   uint32_t width  = 256,
                                                   uint32_t height = 256);
};

} // namespace Engine
} // namespace ExplorerLens
