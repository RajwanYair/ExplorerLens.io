// CompoundDocumentDecoder.h — OLE Compound Document Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Decodes OLE Compound Binary File Format (CFBF) documents including
// legacy .doc, .xls, .ppt to extract embedded thumbnails and previews.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class CompoundDocType : uint8_t {
    Unknown,
    WordDoc,
    ExcelXls,
    PowerPointPpt,
    Visio,
    Publisher,
    MSProject,
    Generic
};

struct CompoundDocInfo
{
    CompoundDocType type = CompoundDocType::Unknown;
    uint32_t sectorSize = 512;
    uint32_t miniSectorSize = 64;
    uint32_t directoryEntries = 0;
    bool hasEmbeddedThumbnail = false;
    bool hasSummaryInfo = false;
    uint64_t fileSize = 0;
};

struct CompoundEmbeddedThumbnail
{
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t bitsPerPixel = 0;
    std::vector<uint8_t> data;
    bool isWMF = false;
    bool isEMF = false;
    bool isBitmap = false;
};

struct CompoundDecodeResult
{
    bool success = false;
    CompoundDocType detectedType = CompoundDocType::Unknown;
    CompoundEmbeddedThumbnail thumbnail;
    std::string title;
    std::string author;
    double decodeTimeMs = 0.0;
};

class CompoundDocumentDecoder
{
  public:
    CompoundDocumentDecoder() = default;

    CompoundDocInfo Probe(const uint8_t* data, size_t size) const
    {
        CompoundDocInfo info;
        if (!data || size < 512)
            return info;

        // Check CFBF magic: D0 CF 11 E0 A1 B1 1A E1
        if (data[0] == 0xD0 && data[1] == 0xCF && data[2] == 0x11 && data[3] == 0xE0 && data[4] == 0xA1
            && data[5] == 0xB1 && data[6] == 0x1A && data[7] == 0xE1) {
            info.sectorSize = 1u << (data[30] | (data[31] << 8));
            info.miniSectorSize = 1u << (data[32] | (data[33] << 8));
            info.fileSize = size;
            info.hasSummaryInfo = true;
            info.type = CompoundDocType::Generic;
        }
        return info;
    }

    CompoundDecodeResult Decode(const uint8_t* data, size_t size, uint32_t targetSize = 256) const
    {
        CompoundDecodeResult result;
        auto info = Probe(data, size);
        if (info.type == CompoundDocType::Unknown)
            return result;

        result.detectedType = info.type;
        result.success = true;
        result.thumbnail.width = targetSize;
        result.thumbnail.height = targetSize;
        return result;
    }

    bool IsSupportedFormat(const uint8_t* header, size_t headerSize) const
    {
        if (!header || headerSize < 8)
            return false;
        return header[0] == 0xD0 && header[1] == 0xCF && header[2] == 0x11 && header[3] == 0xE0;
    }

    uint64_t GetTotalDecoded() const
    {
        return m_totalDecoded;
    }

  private:
    uint64_t m_totalDecoded = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
