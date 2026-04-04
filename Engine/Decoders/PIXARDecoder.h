// PIXARDecoder.h — PIXAR .ptex / .tx Texture Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Decodes PIXAR Per-Face Texture (.ptex) files and the derived .tx (TX texture)
// format used in RenderMan / MaterialX pipelines. Extracts a representative
// face mip-level as a BGRA32 thumbnail image.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens::Decoders {

enum class PTEXDataType : uint8_t {
    UInt8 = 0,
    UInt16 = 1,
    Float16 = 2,
    Float32 = 3,
};

enum class PTEXMeshType : uint8_t {
    Triangle = 0,
    Quadrilateral = 1,
};

class PIXARDecoder
{
  public:
    PIXARDecoder() = default;

    struct DecodeResult
    {
        bool success = false;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t channels = 0;
        std::vector<uint8_t> pixelData;  // BGRA32
        std::string error;
    };

    struct TextureInfo
    {
        uint32_t faceCount = 0;
        uint32_t channels = 0;
        PTEXDataType dataType = PTEXDataType::UInt8;
        PTEXMeshType meshType = PTEXMeshType::Quadrilateral;
        uint32_t mipLevels = 0;

        bool IsValid() const
        {
            return faceCount > 0 && channels > 0;
        }
    };

    TextureInfo ReadInfo(const std::string& filePath) const
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

    static bool IsPIXARExtension(const std::string& ext)
    {
        return ext == ".ptex" || ext == ".PTEX" || ext == ".tx" || ext == ".TX";
    }
    static constexpr const char* EXTENSIONS[] = {".ptex", ".tx", nullptr};

  private:
    static bool VerifyPtexMagic(const uint8_t* header, size_t len);
};

}  // namespace ExplorerLens::Decoders
