// MHAVolumeDecoder.h — MHA/MHD ITK Medical Volume Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Decodes ITK/SimpleITK MetaImage (.mha / .mhd) format 3D medical volumes.
// Reads the ASCII text header to determine dimensions, voxel type, element
// spacing, and data offset, then renders the axial middle-slice as a
// window/level-adjusted BGRA32 thumbnail.
// Supports all common ElementType values: MET_UCHAR, MET_CHAR, MET_USHORT,
// MET_SHORT, MET_INT, MET_FLOAT, MET_DOUBLE.
//
#pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace ExplorerLens { namespace Engine {

enum class MHAElementType : uint8_t {
    Unknown  = 0,
    UChar    = 1,
    Char     = 2,
    UShort   = 3,
    Short    = 4,
    Int      = 5,
    UInt     = 6,
    Float    = 7,
    Double   = 8,
};

struct MHAHeader {
    uint32_t       dimX    = 0;
    uint32_t       dimY    = 0;
    uint32_t       dimZ    = 0;
    MHAElementType elemType = MHAElementType::Unknown;
    float          spacingX = 1.0f;
    float          spacingY = 1.0f;
    float          spacingZ = 1.0f;
    bool           compressed    = false; // ElementDataFile uses .zraw
    bool           bigEndian     = false;
    size_t         dataOffset    = 0;     // Byte offset into .mha file
    std::string    externalFile;          // Non-empty for .mhd (separate .raw)
    bool           valid = false;
};

struct MHARenderResult {
    std::vector<uint8_t> pixelsBGRA;
    uint32_t width   = 0;
    uint32_t height  = 0;
    uint32_t sliceZ  = 0;   // Which axial slice was rendered
    MHAHeader header;
    bool     success = false;
};

struct MHARenderOptions {
    uint32_t targetWidth   = 256;
    uint32_t targetHeight  = 256;
    float    windowMin     = 0.0f;   // 0 = auto (full range)
    float    windowMax     = 0.0f;
    bool     useMiddleSlice = true;  // Render z/2; else use sliceIndex
    uint32_t sliceIndex    = 0;
};

class MHAVolumeDecoder {
public:
    MHAVolumeDecoder()  = default;
    ~MHAVolumeDecoder() = default;

    // Parse MHA/MHD ASCII header.
    static MHAHeader ParseHeader(
        const uint8_t* mhaData, size_t mhaSize) noexcept;

    // Render middle axial slice to BGRA32.
    MHARenderResult Render(
        const uint8_t*          mhaData,
        size_t                  mhaSize,
        const MHARenderOptions& opts = {}) const noexcept;

    // Check for MHA magic ("ObjectType" or "NDims" key near file start).
    static bool IsMHA(const uint8_t* data, size_t size) noexcept;
};

}} // namespace ExplorerLens::Engine
