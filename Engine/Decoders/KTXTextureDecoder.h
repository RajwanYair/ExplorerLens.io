#pragma once
// Sprint 134+184 — KTX/KTX2 Texture Decoder
// GPU-native texture container support for Khronos KTX and KTX2 formats.
// Extracts thumbnails from compressed GPU textures (BC1-BC7, ASTC, ETC2).
// Sprint 184: Full implementation with KTX1/KTX2 header parsing, BC1 decompression.

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <array>
#include <algorithm>

namespace DarkThumbs::Decoders {

// ─── KTX version ──────────────────────────────────────────────────
enum class KTXVersion : uint8_t {
    KTX1 = 1,   // original KTX format (OpenGL-based)
    KTX2 = 2,   // KTX 2.0 (Vulkan-based, supports supercompression)
    Unknown = 0
};

// ─── GPU texture compression formats ──────────────────────────────
enum class TextureCompression : uint8_t {
    Uncompressed = 0,
    BC1_RGB,        // DXT1 — 4bpp, no alpha
    BC3_RGBA,       // DXT5 — 8bpp, interpolated alpha
    BC4_R,          // Single channel — 4bpp
    BC5_RG,         // Two channels — 8bpp
    BC6H_RGB_Float, // HDR — 8bpp
    BC7_RGBA,       // High quality — 8bpp
    ASTC_4x4,       // Adaptive — 8bpp
    ASTC_6x6,       // Adaptive — 3.56bpp
    ASTC_8x8,       // Adaptive — 2bpp
    ETC1_RGB,       // Ericsson — 4bpp
    ETC2_RGB,       // Ericsson v2 — 4bpp
    ETC2_RGBA,      // Ericsson v2 — 8bpp
    COUNT
};

inline const char* CompressionName(TextureCompression c) {
    switch (c) {
        case TextureCompression::Uncompressed:    return "Uncompressed";
        case TextureCompression::BC1_RGB:         return "BC1 (DXT1)";
        case TextureCompression::BC3_RGBA:        return "BC3 (DXT5)";
        case TextureCompression::BC4_R:           return "BC4 (ATI1)";
        case TextureCompression::BC5_RG:          return "BC5 (ATI2)";
        case TextureCompression::BC6H_RGB_Float:  return "BC6H (HDR)";
        case TextureCompression::BC7_RGBA:        return "BC7 (RGBA)";
        case TextureCompression::ASTC_4x4:        return "ASTC 4x4";
        case TextureCompression::ASTC_6x6:        return "ASTC 6x6";
        case TextureCompression::ASTC_8x8:        return "ASTC 8x8";
        case TextureCompression::ETC1_RGB:        return "ETC1";
        case TextureCompression::ETC2_RGB:        return "ETC2 RGB";
        case TextureCompression::ETC2_RGBA:       return "ETC2 RGBA";
        default: return "Unknown";
    }
}

inline bool IsBlockCompressed(TextureCompression c) {
    return c != TextureCompression::Uncompressed && c != TextureCompression::COUNT;
}

// ─── KTX supercompression (KTX2 only) ────────────────────────────
enum class KTXSupercompression : uint8_t {
    None    = 0,
    BasisLZ = 1,   // Basis Universal LZ
    Zstd    = 2,   // Zstandard
    ZLIB    = 3    // Deflate
};

inline const char* SupercompressionName(KTXSupercompression s) {
    switch (s) {
        case KTXSupercompression::None:    return "None";
        case KTXSupercompression::BasisLZ: return "BasisLZ";
        case KTXSupercompression::Zstd:    return "Zstandard";
        case KTXSupercompression::ZLIB:    return "ZLIB";
        default: return "Unknown";
    }
}

// ─── KTX texture info ─────────────────────────────────────────────
struct KTXTextureInfo {
    uint32_t           width = 0;
    uint32_t           height = 0;
    uint32_t           depth = 1;       // 1 for 2D textures
    uint32_t           mipLevels = 1;
    uint32_t           arrayLayers = 1;
    uint32_t           faces = 1;       // 6 for cubemaps
    KTXVersion         version = KTXVersion::Unknown;
    TextureCompression compression = TextureCompression::Uncompressed;
    KTXSupercompression supercompression = KTXSupercompression::None;
    bool               isCubemap = false;
    bool               isArray = false;
    bool               isSRGB = false;

    bool IsValid() const { return width > 0 && height > 0 && version != KTXVersion::Unknown; }
    bool Is3D() const { return depth > 1; }
    bool HasMipmaps() const { return mipLevels > 1; }

    uint32_t BestMipForThumbnail(uint32_t targetSize) const {
        uint32_t mip = 0;
        uint32_t w = width, h = height;
        while (mip < mipLevels - 1) {
            uint32_t nextW = (w > 1) ? w / 2 : 1;
            uint32_t nextH = (h > 1) ? h / 2 : 1;
            if (nextW < targetSize && nextH < targetSize) break;
            w = nextW;
            h = nextH;
            mip++;
        }
        return mip;
    }

    size_t EstimateCompressedSize() const {
        size_t blockSize = 0;
        switch (compression) {
            case TextureCompression::BC1_RGB:
            case TextureCompression::BC4_R:
            case TextureCompression::ETC1_RGB:
            case TextureCompression::ETC2_RGB:
                blockSize = 8; break;  // 4bpp
            case TextureCompression::BC3_RGBA:
            case TextureCompression::BC5_RG:
            case TextureCompression::BC6H_RGB_Float:
            case TextureCompression::BC7_RGBA:
            case TextureCompression::ETC2_RGBA:
            case TextureCompression::ASTC_4x4:
                blockSize = 16; break; // 8bpp
            default:
                return static_cast<size_t>(width) * height * 4;
        }
        uint32_t blocksW = (width + 3) / 4;
        uint32_t blocksH = (height + 3) / 4;
        return static_cast<size_t>(blocksW) * blocksH * blockSize;
    }
};

// ─── Supported extensions ─────────────────────────────────────────
struct KTXExtensions {
    static constexpr size_t COUNT = 2;
    static constexpr std::array<const char*, COUNT> ALL = { ".ktx", ".ktx2" };

    static bool IsSupported(const std::string& ext) {
        std::string lower = ext;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        for (auto& e : ALL) {
            if (lower == e) return true;
        }
        return false;
    }

    static KTXVersion VersionFromExtension(const std::string& ext) {
        std::string lower = ext;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (lower == ".ktx") return KTXVersion::KTX1;
        if (lower == ".ktx2") return KTXVersion::KTX2;
        return KTXVersion::Unknown;
    }
};

// ─── Decode result ────────────────────────────────────────────────
enum class KTXDecodeStatus : uint8_t {
    Success = 0,
    FileNotFound,
    InvalidHeader,
    UnsupportedFormat,
    DecompressionFailed,
    TranscodeFailed,
    MemoryLimitExceeded,
    InternalError
};

struct KTXDecodeResult {
    KTXDecodeStatus  status = KTXDecodeStatus::InternalError;
    KTXTextureInfo   info;
    std::vector<uint8_t> pixelData;
    uint32_t         decodedWidth = 0;
    uint32_t         decodedHeight = 0;
    uint32_t         usedMipLevel = 0;
    double           decodeTimeMs = 0.0;

    bool IsSuccess() const { return status == KTXDecodeStatus::Success; }
    bool HasPixels() const { return !pixelData.empty() && decodedWidth > 0; }
};

// ─── KTX Decoder ─────────────────────────────────────────────────
class KTXTextureDecoder {
public:
    KTXTextureDecoder() = default;

    bool IsAvailable() const { return m_available; }

    /// Read texture metadata from a KTX/KTX2 file without decoding pixel data.
    KTXTextureInfo ReadInfo(const std::string& filePath) const;

    /// Fully decode a KTX/KTX2 file to RGBA pixels, selecting the best mip level.
    KTXDecodeResult Decode(const std::string& filePath, uint32_t targetWidth = 256) const;

    /// Convenience wrapper around Decode().
    KTXDecodeResult DecodeThumbnail(const std::string& filePath,
                                     uint32_t maxSize = 256) const {
        return Decode(filePath, maxSize);
    }

    static bool IsKTXExtension(const std::string& ext) {
        return KTXExtensions::IsSupported(ext);
    }

    static KTXTextureDecoder Create() { return KTXTextureDecoder(); }

private:
    bool m_available = true;

    // Format mapping helpers
    TextureCompression MapGLFormat(uint32_t glFormat) const;
    TextureCompression MapVkFormat(uint32_t vkFormat) const;

    // Decode paths
    KTXDecodeResult DecodeUncompressed(const std::vector<uint8_t>& data,
                                        const KTXTextureInfo& info,
                                        uint32_t mipLevel) const;
    KTXDecodeResult DecodeBlockCompressed(const std::vector<uint8_t>& data,
                                           const KTXTextureInfo& info,
                                           uint32_t mipLevel) const;

    // BC1 (DXT1) block decompression
    void DecompressBC1Block(const uint8_t* block, uint8_t* output,
                            uint32_t outputStride) const;
};

} // namespace DarkThumbs::Decoders
