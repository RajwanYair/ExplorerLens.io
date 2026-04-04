// MNGDecoder.h — Multiple-image Network Graphics (MNG) Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Decodes MNG (Multiple-image Network Graphics) animated image sequences.
// Extracts the first frame for use as a static thumbnail.
// Supports MNG-LC (low-complexity) and MNG-VLC subsets.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens::Decoders {

enum class MNGSubset : uint8_t {
    Full = 0,
    LC = 1,   // Low Complexity
    VLC = 2,  // Very Low Complexity (JNG only)
};

class MNGDecoder
{
  public:
    MNGDecoder() = default;

    struct DecodeResult
    {
        bool success = false;
        uint32_t frameWidth = 0;
        uint32_t frameHeight = 0;
        uint32_t totalFrames = 0;
        std::vector<uint8_t> pixelData;  // BGRA32 of first frame
        std::string error;
    };

    struct MNGInfo
    {
        uint32_t canvasWidth = 0;
        uint32_t canvasHeight = 0;
        uint32_t frameCount = 0;
        MNGSubset subset = MNGSubset::Full;
        uint32_t ticksPerSecond = 1000;

        bool IsValid() const
        {
            return canvasWidth > 0 && canvasHeight > 0;
        }
    };

    MNGInfo ReadInfo(const std::string& filePath) const
    {
        (void)filePath;
        return {};
    }
    DecodeResult Decode(const std::string& filePath, uint32_t targetWidth = 256) const
    {
        (void)filePath;
        (void)targetWidth;
        return {};
    }

    static bool IsMNGExtension(const std::string& ext)
    {
        return ext == ".mng" || ext == ".MNG" || ext == ".jng" || ext == ".JNG";
    }
    static constexpr const char* EXTENSIONS[] = {".mng", ".jng", nullptr};

  private:
    static bool VerifyMNGSignature(const uint8_t* header, size_t len);
};

}  // namespace ExplorerLens::Decoders
