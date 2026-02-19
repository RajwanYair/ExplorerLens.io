//==============================================================================
// OpenRaster (.ora) Decoder — Implementation
// Sprint 185: Open image editor format support
// Extracts thumbnail.png or mergedimage.png from OpenRaster ZIP archives.
// Copyright (c) 2026 - DarkThumbs Project
//==============================================================================

#include "OpenRasterDecoder.h"
#include <fstream>
#include <cstring>
#include <algorithm>
#include <windows.h>
#include <gdiplus.h>
#include <shlwapi.h>

#pragma comment(lib, "gdiplus.lib")

namespace DarkThumbs::Decoders {

    //==========================================================================
    // Extension check
    //==========================================================================
    bool OpenRasterDecoder::IsORAExtension(const std::string& ext)
    {
        std::string lower = ext;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        return lower == ".ora";
    }

    //==========================================================================
    // ZIP local file header (minimal parsing)
    //==========================================================================
    struct ZIPLocalFileHeader {
        uint32_t signature;            // 0x04034b50
        uint16_t versionNeeded;
        uint16_t flags;
        uint16_t compressionMethod;    // 0 = stored, 8 = deflate
        uint16_t lastModTime;
        uint16_t lastModDate;
        uint32_t crc32;
        uint32_t compressedSize;
        uint32_t uncompressedSize;
        uint16_t fileNameLength;
        uint16_t extraFieldLength;
    };

    //==========================================================================
    // Find and extract a file from a ZIP archive (stored entries only)
    // For OpenRaster, thumbnail.png is typically stored uncompressed.
    //==========================================================================
    static bool FindFileInZIP(const std::vector<uint8_t>& zipData,
                               const std::string& targetName,
                               std::vector<uint8_t>& outData)
    {
        size_t pos = 0;
        while (pos + sizeof(ZIPLocalFileHeader) < zipData.size()) {
            const auto* hdr = reinterpret_cast<const ZIPLocalFileHeader*>(zipData.data() + pos);
            if (hdr->signature != 0x04034b50) break;

            size_t nameStart = pos + sizeof(ZIPLocalFileHeader);
            size_t nameEnd = nameStart + hdr->fileNameLength;
            if (nameEnd > zipData.size()) break;

            std::string fileName(reinterpret_cast<const char*>(zipData.data() + nameStart),
                                  hdr->fileNameLength);

            size_t dataStart = nameEnd + hdr->extraFieldLength;
            size_t dataEnd = dataStart + hdr->compressedSize;
            if (dataEnd > zipData.size()) break;

            // Case-insensitive name comparison
            std::string lowerName = fileName;
            std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
            std::string lowerTarget = targetName;
            std::transform(lowerTarget.begin(), lowerTarget.end(), lowerTarget.begin(), ::tolower);

            if (lowerName == lowerTarget) {
                if (hdr->compressionMethod == 0) {
                    // Stored (uncompressed)
                    outData.assign(zipData.data() + dataStart, zipData.data() + dataEnd);
                    return true;
                }
                // For deflate, we'd need zlib — for now, only handle stored
                return false;
            }

            pos = dataEnd;
        }
        return false;
    }

    //==========================================================================
    // Decode PNG data from memory using GDI+
    //==========================================================================
    static bool DecodePNGFromMemory(const std::vector<uint8_t>& pngData,
                                     uint32_t& outWidth, uint32_t& outHeight,
                                     std::vector<uint8_t>& outPixels)
    {
        // Initialize GDI+
        Gdiplus::GdiplusStartupInput startupInput;
        ULONG_PTR gdipToken;
        if (Gdiplus::GdiplusStartup(&gdipToken, &startupInput, nullptr) != Gdiplus::Ok)
            return false;

        bool ok = false;

        // Create IStream from memory
        IStream* stream = SHCreateMemStream(pngData.data(), static_cast<UINT>(pngData.size()));
        if (stream) {
            Gdiplus::Bitmap* bmp = Gdiplus::Bitmap::FromStream(stream);
            if (bmp && bmp->GetLastStatus() == Gdiplus::Ok) {
                outWidth = bmp->GetWidth();
                outHeight = bmp->GetHeight();

                Gdiplus::BitmapData bmpData;
                Gdiplus::Rect rect(0, 0, static_cast<INT>(outWidth), static_cast<INT>(outHeight));

                if (bmp->LockBits(&rect, Gdiplus::ImageLockModeRead,
                    PixelFormat32bppARGB, &bmpData) == Gdiplus::Ok) {
                    outPixels.resize(static_cast<size_t>(outWidth) * outHeight * 4);
                    for (uint32_t y = 0; y < outHeight; ++y) {
                        const uint8_t* srcRow = static_cast<const uint8_t*>(bmpData.Scan0) + y * bmpData.Stride;
                        uint8_t* dstRow = outPixels.data() + static_cast<size_t>(y) * outWidth * 4;
                        memcpy(dstRow, srcRow, static_cast<size_t>(outWidth) * 4);
                    }
                    bmp->UnlockBits(&bmpData);
                    ok = true;
                }
                delete bmp;
            }
            stream->Release();
        }

        Gdiplus::GdiplusShutdown(gdipToken);
        return ok;
    }

    //==========================================================================
    // Read info — parse mimetype and look for key files
    //==========================================================================
    OpenRasterDecoder::ImageInfo OpenRasterDecoder::ReadInfo(const std::string& filePath) const
    {
        ImageInfo info;
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) return info;

        size_t fileSize = static_cast<size_t>(file.tellg());
        if (fileSize < 30 || fileSize > 500 * 1024 * 1024) return info; // 500MB max
        file.seekg(0);
        std::vector<uint8_t> zipData(fileSize);
        file.read(reinterpret_cast<char*>(zipData.data()), fileSize);

        // Check for ZIP signature
        if (zipData[0] != 'P' || zipData[1] != 'K' || zipData[2] != 3 || zipData[3] != 4)
            return info;

        // OpenRaster must contain "mimetype" as first file with value "image/openraster"
        std::vector<uint8_t> mimeData;
        if (FindFileInZIP(zipData, "mimetype", mimeData)) {
            std::string mime(mimeData.begin(), mimeData.end());
            if (mime.find("image/openraster") == std::string::npos)
                return info;
        }

        // Check for thumbnail
        std::vector<uint8_t> thumbData;
        info.hasThumbnail = FindFileInZIP(zipData, "Thumbnails/thumbnail.png", thumbData);

        // Check for merged image
        std::vector<uint8_t> mergedData;
        info.hasMergedImage = FindFileInZIP(zipData, "mergedimage.png", mergedData);

        // Set basic dimensions (will be refined during decode)
        info.width = 256;
        info.height = 256;

        return info;
    }

    //==========================================================================
    // Decode OpenRaster
    //==========================================================================
    OpenRasterDecoder::DecodeResult OpenRasterDecoder::Decode(const std::string& filePath,
                                                               uint32_t targetWidth) const
    {
        DecodeResult result;
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            result.error = "Cannot open file";
            return result;
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        if (fileSize < 30) {
            result.error = "File too small";
            return result;
        }
        file.seekg(0);
        std::vector<uint8_t> zipData(fileSize);
        file.read(reinterpret_cast<char*>(zipData.data()), fileSize);

        // Verify ZIP signature
        if (zipData[0] != 'P' || zipData[1] != 'K') {
            result.error = "Not a ZIP archive";
            return result;
        }

        // Try thumbnail first (usually stored, fast)
        std::vector<uint8_t> pngData;
        bool found = FindFileInZIP(zipData, "Thumbnails/thumbnail.png", pngData);
        if (!found) {
            // Fall back to merged image
            found = FindFileInZIP(zipData, "mergedimage.png", pngData);
        }

        if (!found || pngData.empty()) {
            result.error = "No thumbnail or merged image found in OpenRaster archive";
            return result;
        }

        // Decode PNG via GDI+
        uint32_t w, h;
        std::vector<uint8_t> pixels;
        if (!DecodePNGFromMemory(pngData, w, h, pixels)) {
            result.error = "Failed to decode PNG from OpenRaster archive";
            return result;
        }

        result.width = w;
        result.height = h;
        result.pixelData = std::move(pixels);
        result.success = true;
        return result;
    }

} // namespace DarkThumbs::Decoders
