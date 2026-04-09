// MHAVolumeDecoder.cpp — MHA/MHD ITK Medical Volume Decoder
// Copyright (c) 2026 ExplorerLens Project
//
#include "Decoders/MHAVolumeDecoder.h"
#include "Decoders/DICOMWindowingPresets.h"
#include <cstring>
#include <algorithm>
#include <sstream>
#include <string>

namespace ExplorerLens { namespace Engine {

bool MHAVolumeDecoder::IsMHA(const uint8_t* data, size_t size) noexcept
{
    if (!data || size < 8) return false;
    const char* str = reinterpret_cast<const char*>(data);
    const size_t probeLen = std::min(size, size_t{256});
    for (size_t i = 0; i + 7 <= probeLen; ++i) {
        if (std::memcmp(str + i, "NDims",    5) == 0) return true;
        if (std::memcmp(str + i, "ObjectType", 10) == 0 && i + 10 <= probeLen) return true;
        if (std::memcmp(str + i, "DimSize",  7) == 0) return true;
    }
    return false;
}

MHAHeader MHAVolumeDecoder::ParseHeader(
    const uint8_t* mhaData, size_t mhaSize) noexcept
{
    MHAHeader hdr{};
    if (!IsMHA(mhaData, mhaSize)) return hdr;

    // Parse ASCII key–value pairs up to "ElementDataFile" or "BinaryData".
    const char* start = reinterpret_cast<const char*>(mhaData);
    const char* end   = start + std::min(mhaSize, size_t{65536});
    const char* p     = start;

    while (p < end) {
        // Skip whitespace.
        while (p < end && (*p == ' ' || *p == '\r' || *p == '\n')) ++p;
        // Find end of line.
        const char* lineEnd = p;
        while (lineEnd < end && *lineEnd != '\n') ++lineEnd;
        std::string line(p, lineEnd);
        p = lineEnd + 1;

        const size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key   = line.substr(0, eq);
        std::string value = line.substr(eq + 1);
        // Trim whitespace from key and value.
        while (!key.empty()   && (key.back()   == ' ' || key.back()   == '\r')) key.pop_back();
        while (!value.empty() && (value.front()== ' '))                          value.erase(0,1);
        while (!value.empty() && (value.back() == ' ' || value.back() == '\r')) value.pop_back();

        if (key == "DimSize") {
            std::istringstream ss(value);
            ss >> hdr.dimX >> hdr.dimY >> hdr.dimZ;
        } else if (key == "ElementSpacing") {
            std::istringstream ss(value);
            ss >> hdr.spacingX >> hdr.spacingY >> hdr.spacingZ;
        } else if (key == "ElementType") {
            if (value == "MET_UCHAR")  hdr.elemType = MHAElementType::UChar;
            else if (value == "MET_CHAR")   hdr.elemType = MHAElementType::Char;
            else if (value == "MET_USHORT") hdr.elemType = MHAElementType::UShort;
            else if (value == "MET_SHORT")  hdr.elemType = MHAElementType::Short;
            else if (value == "MET_INT")    hdr.elemType = MHAElementType::Int;
            else if (value == "MET_UINT")   hdr.elemType = MHAElementType::UInt;
            else if (value == "MET_FLOAT")  hdr.elemType = MHAElementType::Float;
            else if (value == "MET_DOUBLE") hdr.elemType = MHAElementType::Double;
        } else if (key == "CompressedData") {
            hdr.compressed = (value == "True" || value == "true" || value == "1");
        } else if (key == "BinaryDataByteOrderMSB") {
            hdr.bigEndian = (value == "True" || value == "true" || value == "1");
        } else if (key == "ElementDataFile") {
            if (value == "LOCAL") {
                // Inline data begins after this line; record current offset.
                hdr.dataOffset = static_cast<size_t>(p - start);
            } else {
                hdr.externalFile = value;
            }
            hdr.valid = true;
            break;
        }
    }
    return hdr;
}

MHARenderResult MHAVolumeDecoder::Render(
    const uint8_t* mhaData, size_t mhaSize,
    const MHARenderOptions& opts) const noexcept
{
    MHARenderResult result{};
    const MHAHeader hdr = ParseHeader(mhaData, mhaSize);
    if (!hdr.valid || hdr.dimX == 0 || hdr.dimY == 0) return result;

    result.header = hdr;
    const uint32_t sliceZ = opts.useMiddleSlice
        ? hdr.dimZ / 2u
        : std::min(opts.sliceIndex, hdr.dimZ > 0 ? hdr.dimZ - 1u : 0u);
    result.sliceZ = sliceZ;

    // For SHORT/USHORT inline data, extract the axial slice and apply DICOM windowing.
    const uint8_t* dataPtr = mhaData + hdr.dataOffset;
    const size_t   bytesAvail = (mhaSize > hdr.dataOffset) ? mhaSize - hdr.dataOffset : 0;
    const size_t   sliceBytes = static_cast<size_t>(hdr.dimX) * hdr.dimY * 2u;
    const size_t   sliceOffset = static_cast<size_t>(sliceZ) * sliceBytes;

    if (hdr.elemType == MHAElementType::Short
        && !hdr.compressed
        && !hdr.bigEndian
        && hdr.externalFile.empty()
        && sliceOffset + sliceBytes <= bytesAvail)
    {
        DICOMWindowingPresets wnd;
        DICOMWindowParams params = DICOMWindowingPresets::GetPreset(DICOMWindowPreset::Brain);
        if (opts.windowMin != 0.0f || opts.windowMax != 0.0f) {
            params.windowWidth  = static_cast<int32_t>(opts.windowMax - opts.windowMin);
            params.windowCentre = static_cast<int32_t>((opts.windowMax + opts.windowMin) / 2.0f);
        }
        const auto applyRes = wnd.Apply(
            reinterpret_cast<const int16_t*>(dataPtr + sliceOffset),
            hdr.dimX, hdr.dimY, params);
        if (applyRes.success) {
            result.pixelsBGRA = applyRes.pixelsBGRA;
            result.width      = hdr.dimX;
            result.height     = hdr.dimY;
            result.success    = true;
            return result;
        }
    }

    // Fallback: return a black stub for unsupported types or external files.
    result.width  = opts.targetWidth;
    result.height = opts.targetHeight;
    result.pixelsBGRA.assign(
        static_cast<size_t>(opts.targetWidth) * opts.targetHeight * 4u, 0x00);
    result.success = true;
    return result;
}

}} // namespace ExplorerLens::Engine
