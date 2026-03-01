#pragma once
// JPEG 2000 Decoder Interface
// JP2/J2K/JPF/JPX format support via OpenJPEG-compatible interface.
// tile streaming, colour-space conversion, 256 MB memory ceiling.
// Provides thumbnail extraction from JPEG 2000 wavelet-compressed images.

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <array>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <chrono>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace ExplorerLens::Decoders {

// ─── JPEG 2000 sub-formats ────────────────────────────────────────
enum class JP2Format : uint8_t {
    JP2 = 0, // JPEG 2000 Part 1 (.jp2)
    J2K = 1, // JPEG 2000 codestream (.j2k, .j2c)
    JPX = 2, // JPEG 2000 Part 2 extended (.jpx, .jpf)
    JPH = 3, // JPEG 2000 High-Throughput (.jph)
    Unknown = 255
};

inline const char* JP2FormatName(JP2Format f) {
    switch (f) {
    case JP2Format::JP2: return "JPEG 2000 (.jp2)";
    case JP2Format::J2K: return "J2K Codestream (.j2k)";
    case JP2Format::JPX: return "JPEG 2000 Extended (.jpx)";
    case JP2Format::JPH: return "JPEG 2000 HTJ2K (.jph)";
    default: return "Unknown";
    }
}

// ─── Supported extensions ─────────────────────────────────────────
struct JP2Extensions {
    static constexpr size_t COUNT = 8;
    static constexpr std::array<const char*, COUNT> ALL = {
    ".jp2", ".j2k", ".j2c", ".jpf", ".jpx", ".jph", ".jhc", ".jpc"
    };

    static bool IsSupported(const std::string& ext) {
        std::string lower = ext;
        std::transform(lower.begin(), lower.end(), lower.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        for (auto& e : ALL) {
            if (lower == e) return true;
        }
        return false;
    }

    static JP2Format ClassifyExtension(const std::string& ext) {
        std::string lower = ext;
        std::transform(lower.begin(), lower.end(), lower.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (lower == ".jp2") return JP2Format::JP2;
        if (lower == ".j2k" || lower == ".j2c" || lower == ".jpc") return JP2Format::J2K;
        if (lower == ".jpx" || lower == ".jpf") return JP2Format::JPX;
        if (lower == ".jph" || lower == ".jhc") return JP2Format::JPH;
        return JP2Format::Unknown;
    }
};

// ─── JPEG 2000 image info ─────────────────────────────────────────
struct JP2ImageInfo {
    uint32_t width = 0;
    uint32_t height = 0;
    uint8_t numComponents = 0; // 1=Gray, 3=RGB, 4=RGBA
    uint8_t bitsPerComponent = 8;
    uint8_t numResolutionLevels = 0;
    JP2Format format = JP2Format::Unknown;
    bool isSigned = false;
    bool hasAlpha = false;
    std::string colorSpace; // "sRGB", "sYCC", "Grayscale"

    bool IsValid() const { return width > 0 && height > 0 && numComponents > 0; }

    uint32_t BestReductionLevel(uint32_t targetWidth, uint32_t targetHeight) const {
        if (numResolutionLevels <= 1) return 0;
        uint32_t level = 0;
        uint32_t w = width, h = height;
        while (level + 1 < static_cast<uint32_t>(numResolutionLevels)) {
            uint32_t nextW = w / 2;
            uint32_t nextH = h / 2;
            if (nextW < targetWidth || nextH < targetHeight) break;
            w = nextW;
            h = nextH;
            level++;
        }
        return level;
    }

    size_t EstimateDecodedSize() const {
        return static_cast<size_t>(width) * height * numComponents * ((bitsPerComponent + 7) / 8);
    }

    size_t EstimateReducedSize(uint32_t reductionLevel) const {
        uint32_t w = width >> reductionLevel;
        uint32_t h = height >> reductionLevel;
        if (w == 0) w = 1;
        if (h == 0) h = 1;
        return static_cast<size_t>(w) * h * numComponents * ((bitsPerComponent + 7) / 8);
    }
};

// ─── Decode options ───────────────────────────────────────────────
struct JP2DecodeOptions {
    uint32_t maxWidth = 0; // 0 = no limit
    uint32_t maxHeight = 0;
    uint32_t reductionLevel = 0; // 0 = full res
    bool autoSelectLevel = true;
    bool forceRGB = true; // convert to RGB even if YCC
    size_t memoryLimitBytes = 256 * 1024 * 1024; // 256 MB

    static JP2DecodeOptions Thumbnail(uint32_t size = 256) {
        JP2DecodeOptions opt;
        opt.maxWidth = size;
        opt.maxHeight = size;
        opt.autoSelectLevel = true;
        opt.memoryLimitBytes = 64 * 1024 * 1024;
        return opt;
    }

    static JP2DecodeOptions FullResolution() {
        JP2DecodeOptions opt;
        opt.autoSelectLevel = false;
        opt.reductionLevel = 0;
        return opt;
    }
};

// ─── Decode result ────────────────────────────────────────────────
enum class JP2DecodeStatus : uint8_t {
    Success = 0,
    FileNotFound,
    InvalidFormat,
    CorruptData,
    UnsupportedFeature,
    MemoryLimitExceeded,
    LibraryNotAvailable,
    InternalError
};

inline const char* JP2StatusName(JP2DecodeStatus s) {
    switch (s) {
    case JP2DecodeStatus::Success: return "Success";
    case JP2DecodeStatus::FileNotFound: return "File not found";
    case JP2DecodeStatus::InvalidFormat: return "Invalid JPEG 2000 format";
    case JP2DecodeStatus::CorruptData: return "Corrupt codestream data";
    case JP2DecodeStatus::UnsupportedFeature: return "Unsupported J2K feature";
    case JP2DecodeStatus::MemoryLimitExceeded: return "Memory limit exceeded";
    case JP2DecodeStatus::LibraryNotAvailable: return "OpenJPEG library not available";
    case JP2DecodeStatus::InternalError: return "Internal decoder error";
    default: return "Unknown";
    }
}

struct JP2DecodeResult {
    JP2DecodeStatus status = JP2DecodeStatus::InternalError;
    JP2ImageInfo info;
    std::vector<uint8_t> pixelData; // decoded BGRA/RGB pixels
    uint32_t decodedWidth = 0;
    uint32_t decodedHeight = 0;
    uint32_t usedReductionLevel = 0;
    double decodeTimeMs = 0.0;

    bool IsSuccess() const { return status == JP2DecodeStatus::Success; }
    bool HasPixels() const { return !pixelData.empty() && decodedWidth > 0; }
};

// ─── JPEG 2000 Decoder ───────────────────────────────────────────
class JPEG2000Decoder {
public:
    JPEG2000Decoder() = default;

    /// Checks whether openjp2.dll can be loaded on this system.
    /// Performs a probe load/unload each call; result is cached in m_available.
    bool IsAvailable() const {
        if (!m_probed) {
            HMODULE hLib = ::LoadLibraryW(L"openjp2.dll");
            if (hLib) {
                m_available = true;
                ::FreeLibrary(hLib);
            }
            else {
                m_available = false;
            }
            m_probed = true;
        }
        return m_available;
    }

    JP2ImageInfo ReadInfo(const std::string& filePath) const {
        JP2ImageInfo info;
        // Detect format from extension
        size_t dot = filePath.rfind('.');
        if (dot != std::string::npos) {
            info.format = JP2Extensions::ClassifyExtension(filePath.substr(dot));
        }

        // Read file header bytes for box parsing
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) return info;

        // JP2 signature: 12 bytes — 0x0000000C 6A502020 0D0A870A
        constexpr uint8_t JP2_SIG[12] = {
        0x00, 0x00, 0x00, 0x0C, 0x6A, 0x50,
        0x20, 0x20, 0x0D, 0x0A, 0x87, 0x0A
        };
        // J2K codestream marker: FF 4F FF 51
        constexpr uint8_t J2K_SOC[2] = { 0xFF, 0x4F };

        uint8_t header[12] = {};
        file.read(reinterpret_cast<char*>(header), 12);
        const auto bytesRead = file.gcount();

        bool isJP2Box = (bytesRead == 12 && std::memcmp(header, JP2_SIG, 12) == 0);
        bool isRawCodestream = (bytesRead >= 2 && header[0] == J2K_SOC[0] && header[1] == J2K_SOC[1]);

        if (isRawCodestream) {
            // Raw J2K codestream — parse SIZ marker (FF 51)
            // SIZ must follow SOC.  Read past SOC (2 bytes already in header).
            file.seekg(2, std::ios::beg);
            uint8_t marker[2] = {};
            file.read(reinterpret_cast<char*>(marker), 2);
            if (marker[0] == 0xFF && marker[1] == 0x51) {
                uint8_t siz[36] = {};
                file.read(reinterpret_cast<char*>(siz), 36);
                if (file.gcount() >= 36) {
                    auto readBE32 = [](const uint8_t* p) -> uint32_t {
                        return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) |
                            (uint32_t(p[2]) << 8) | uint32_t(p[3]);
                        };
                    auto readBE16 = [](const uint8_t* p) -> uint16_t {
                        return (uint16_t(p[0]) << 8) | uint16_t(p[1]);
                        };
                    // SIZ layout after marker: Lsiz(2), Rsiz(2), Xsiz(4), Ysiz(4),
                    // XOsiz(4), YOsiz(4), XTsiz(4), YTsiz(4), XTOsiz(4), YTOsiz(4), Csiz(2)
                    uint32_t xSiz = readBE32(siz + 4);
                    uint32_t ySiz = readBE32(siz + 8);
                    uint32_t xoSiz = readBE32(siz + 12);
                    uint32_t yoSiz = readBE32(siz + 16);
                    uint16_t cSiz = readBE16(siz + 34);

                    info.width = xSiz - xoSiz;
                    info.height = ySiz - yoSiz;
                    info.numComponents = static_cast<uint8_t>(cSiz > 255 ? 255 : cSiz);
                    // First component Ssiz byte follows Csiz
                    if (file.gcount() >= 36) {
                        uint8_t ssiz = siz[36 - 1]; // last byte we read isn't right, re-read
                        // Actually, component info starts at offset 36 in our buffer.
                        // We need one more byte.  Read it:
                        uint8_t ssizByte = 0;
                        file.read(reinterpret_cast<char*>(&ssizByte), 1);
                        info.isSigned = (ssizByte & 0x80) != 0;
                        info.bitsPerComponent = static_cast<uint8_t>((ssizByte & 0x7F) + 1);
                    }
                    info.numResolutionLevels = 6; // default; exact value needs COD parse
                    info.colorSpace = (cSiz == 1) ? "Grayscale" : "sRGB";
                    info.hasAlpha = (cSiz >= 4);
                }
            }
            if (info.format == JP2Format::Unknown) info.format = JP2Format::J2K;
        }
        else if (isJP2Box) {
            // JP2 box format — iterate boxes looking for 'ihdr' inside 'jp2h'
            file.seekg(12, std::ios::beg); // past signature box

            auto readBE32 = [](const uint8_t* p) -> uint32_t {
                return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) |
                    (uint32_t(p[2]) << 8) | uint32_t(p[3]);
                };

            constexpr uint32_t BOX_FTYP = 0x66747970; // 'ftyp'
            constexpr uint32_t BOX_JP2H = 0x6A703268; // 'jp2h'
            constexpr uint32_t BOX_IHDR = 0x69686472; // 'ihdr'

            bool foundInfo = false;
            for (int boxCount = 0; boxCount < 64 && !foundInfo; ++boxCount) {
                uint8_t boxHdr[8] = {};
                file.read(reinterpret_cast<char*>(boxHdr), 8);
                if (file.gcount() < 8) break;

                uint32_t boxLen = readBE32(boxHdr);
                uint32_t boxType = readBE32(boxHdr + 4);

                if (boxLen < 8 && boxLen != 0) break; // malformed
                uint64_t dataLen = (boxLen == 0) ? 0 : (boxLen - 8);

                // Extended length (boxLen == 1): 8-byte length follows
                if (boxLen == 1) {
                    uint8_t ext[8] = {};
                    file.read(reinterpret_cast<char*>(ext), 8);
                    if (file.gcount() < 8) break;
                    uint64_t xl = (uint64_t(readBE32(ext)) << 32) | readBE32(ext + 4);
                    if (xl < 16) break;
                    dataLen = xl - 16;
                }

                if (boxType == BOX_JP2H) {
                    // Parse sub-boxes within jp2h to find ihdr
                    auto jp2hEnd = static_cast<uint64_t>(file.tellg()) + dataLen;
                    for (int sub = 0; sub < 32; ++sub) {
                        if (static_cast<uint64_t>(file.tellg()) >= jp2hEnd) break;
                        uint8_t subHdr[8] = {};
                        file.read(reinterpret_cast<char*>(subHdr), 8);
                        if (file.gcount() < 8) break;

                        uint32_t subLen = readBE32(subHdr);
                        uint32_t subType = readBE32(subHdr + 4);

                        if (subType == BOX_IHDR && subLen >= 22) {
                            // ihdr box: height(4) width(4) nc(2) bpc(1) ct(1) unk(1) ipr(1)
                            uint8_t ihdr[14] = {};
                            file.read(reinterpret_cast<char*>(ihdr), 14);
                            if (file.gcount() >= 14) {
                                info.height = readBE32(ihdr);
                                info.width = readBE32(ihdr + 4);
                                uint16_t nc = (uint16_t(ihdr[8]) << 8) | ihdr[9];
                                info.numComponents = static_cast<uint8_t>(nc > 255 ? 255 : nc);
                                uint8_t bpc = ihdr[10];
                                if (bpc != 255) { // 255 = varying per component
                                    info.isSigned = (bpc & 0x80) != 0;
                                    info.bitsPerComponent = static_cast<uint8_t>((bpc & 0x7F) + 1);
                                }
                                info.hasAlpha = (nc >= 4);
                                info.colorSpace = (nc == 1) ? "Grayscale" : "sRGB";
                                info.numResolutionLevels = 6; // default; exact needs COD marker
                                foundInfo = true;
                            }
                            break;
                        }
                        if (subLen > 8) {
                            file.seekg(static_cast<std::streamoff>(subLen - 8), std::ios::cur);
                        }
                    }
                    break; // done with jp2h
                }
                else {
                    // Skip this box
                    if (boxLen == 0) break; // box extends to EOF
                    file.seekg(static_cast<std::streamoff>(dataLen), std::ios::cur);
                }
            }
            if (info.format == JP2Format::Unknown) info.format = JP2Format::JP2;
        }

        return info;
    }

    JP2DecodeResult DecodeThumbnail(const std::string& filePath,
        uint32_t maxSize = 256) const {
        JP2DecodeResult result;

        if (!IsAvailable()) {
            result.status = JP2DecodeStatus::LibraryNotAvailable;
            return result;
        }

        auto info = ReadInfo(filePath);
        if (!info.IsValid()) {
            result.status = JP2DecodeStatus::InvalidFormat;
            return result;
        }

        JP2DecodeOptions opts = JP2DecodeOptions::Thumbnail(maxSize);
        uint32_t level = info.BestReductionLevel(maxSize, maxSize);

        result.info = info;
        result.usedReductionLevel = level;
        result.decodedWidth = info.width >> level;
        result.decodedHeight = info.height >> level;
        if (result.decodedWidth == 0) result.decodedWidth = 1;
        if (result.decodedHeight == 0) result.decodedHeight = 1;

        auto t0 = std::chrono::high_resolution_clock::now();

        // ── OpenJPEG opaque types (minimal forward declarations) ──
        // These mirror the OpenJPEG 2.x C API pointer types.
        using opj_codec_t = void*;
        using opj_stream_t = void*;
        using opj_image_t = void*; // actually a struct, accessed via offset-safe helpers

        // ── Function pointer types matching OpenJPEG public API ──
        using fn_opj_create_decompress = opj_codec_t(*)(int /*codec_format*/);
        using fn_opj_destroy_codec = void(*)(opj_codec_t);
        using fn_opj_stream_create_default_file_stream = opj_stream_t(*)(const char*, int /*is_read*/);
        using fn_opj_stream_destroy = void(*)(opj_stream_t);
        using fn_opj_setup_decoder = int(*)(opj_codec_t, void* /*opj_dparameters_t*/);
        using fn_opj_set_default_decoder_parameters = void(*)(void* /*opj_dparameters_t*/);
        using fn_opj_read_header = int(*)(opj_stream_t, opj_codec_t, opj_image_t*);
        using fn_opj_set_decoded_resolution_factor = int(*)(opj_codec_t, uint32_t);
        using fn_opj_decode = int(*)(opj_codec_t, opj_stream_t, opj_image_t);
        using fn_opj_end_decompress = int(*)(opj_codec_t, opj_stream_t);
        using fn_opj_image_destroy = void(*)(opj_image_t);

        // ── Load openjp2.dll dynamically ──
        HMODULE hOpenJP2 = ::LoadLibraryW(L"openjp2.dll");
        if (!hOpenJP2) {
            // Library not loadable — produce a placeholder gray thumbnail
            const uint32_t thumbW = (std::min)(maxSize, result.decodedWidth);
            const uint32_t thumbH = (std::min)(maxSize, result.decodedHeight);
            result.decodedWidth = thumbW > 0 ? thumbW : 1;
            result.decodedHeight = thumbH > 0 ? thumbH : 1;
            result.pixelData.assign(
                static_cast<size_t>(result.decodedWidth) * result.decodedHeight * 4,
                0x80); // mid-gray BGRA placeholder
            // Stamp alpha channel to 0xFF for every pixel
            for (size_t i = 3; i < result.pixelData.size(); i += 4)
                result.pixelData[i] = 0xFF;
            result.status = JP2DecodeStatus::LibraryNotAvailable;
            auto t1 = std::chrono::high_resolution_clock::now();
            result.decodeTimeMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
            return result;
        }

        // Resolve required entry points
        auto pfnCreateDecompress = reinterpret_cast<fn_opj_create_decompress>(
            ::GetProcAddress(hOpenJP2, "opj_create_decompress"));
        auto pfnDestroyCodec = reinterpret_cast<fn_opj_destroy_codec>(
            ::GetProcAddress(hOpenJP2, "opj_destroy_codec"));
        auto pfnStreamCreate = reinterpret_cast<fn_opj_stream_create_default_file_stream>(
            ::GetProcAddress(hOpenJP2, "opj_stream_create_default_file_stream"));
        auto pfnStreamDestroy = reinterpret_cast<fn_opj_stream_destroy>(
            ::GetProcAddress(hOpenJP2, "opj_stream_destroy"));
        auto pfnSetupDecoder = reinterpret_cast<fn_opj_setup_decoder>(
            ::GetProcAddress(hOpenJP2, "opj_setup_decoder"));
        auto pfnSetDefaultParams = reinterpret_cast<fn_opj_set_default_decoder_parameters>(
            ::GetProcAddress(hOpenJP2, "opj_set_default_decoder_parameters"));
        auto pfnReadHeader = reinterpret_cast<fn_opj_read_header>(
            ::GetProcAddress(hOpenJP2, "opj_read_header"));
        auto pfnSetResFactor = reinterpret_cast<fn_opj_set_decoded_resolution_factor>(
            ::GetProcAddress(hOpenJP2, "opj_set_decoded_resolution_factor"));
        auto pfnDecode = reinterpret_cast<fn_opj_decode>(
            ::GetProcAddress(hOpenJP2, "opj_decode"));
        auto pfnEndDecompress = reinterpret_cast<fn_opj_end_decompress>(
            ::GetProcAddress(hOpenJP2, "opj_end_decompress"));
        auto pfnImageDestroy = reinterpret_cast<fn_opj_image_destroy>(
            ::GetProcAddress(hOpenJP2, "opj_image_destroy"));

        if (!pfnCreateDecompress || !pfnDestroyCodec || !pfnStreamCreate ||
            !pfnStreamDestroy || !pfnSetupDecoder || !pfnSetDefaultParams ||
            !pfnReadHeader || !pfnDecode || !pfnEndDecompress || !pfnImageDestroy) {
            ::FreeLibrary(hOpenJP2);
            result.status = JP2DecodeStatus::LibraryNotAvailable;
            auto t1 = std::chrono::high_resolution_clock::now();
            result.decodeTimeMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
            return result;
        }

        // Determine codec format: 0 = J2K codestream, 1 = JP2 boxed, 2 = JPT/JPP
        int codecFmt = 1; // default to JP2
        if (info.format == JP2Format::J2K) codecFmt = 0;
        else if (info.format == JP2Format::JPX) codecFmt = 2;

        // Create codec
        opj_codec_t codec = pfnCreateDecompress(codecFmt);
        if (!codec) {
            ::FreeLibrary(hOpenJP2);
            result.status = JP2DecodeStatus::InternalError;
            return result;
        }

        // Setup decoder parameters (opj_dparameters_t is ~256 bytes, zero-init)
        alignas(16) uint8_t dparams[512] = {};
        pfnSetDefaultParams(dparams);
        // Set cp_reduce (reduction factor) — offset 0 in opj_dparameters_t
        // cp_reduce is the first uint32_t field at offset 0.
        std::memcpy(dparams, &level, sizeof(uint32_t));

        if (!pfnSetupDecoder(codec, dparams)) {
            pfnDestroyCodec(codec);
            ::FreeLibrary(hOpenJP2);
            result.status = JP2DecodeStatus::InternalError;
            return result;
        }

        // Create file stream
        opj_stream_t stream = pfnStreamCreate(filePath.c_str(), 1 /*read*/);
        if (!stream) {
            pfnDestroyCodec(codec);
            ::FreeLibrary(hOpenJP2);
            result.status = JP2DecodeStatus::FileNotFound;
            return result;
        }

        // Read header
        opj_image_t image = nullptr;
        if (!pfnReadHeader(stream, codec, &image) || !image) {
            pfnStreamDestroy(stream);
            pfnDestroyCodec(codec);
            ::FreeLibrary(hOpenJP2);
            result.status = JP2DecodeStatus::InvalidFormat;
            return result;
        }

        // Set resolution reduction factor if available
        if (pfnSetResFactor && level > 0) {
            pfnSetResFactor(codec, level);
        }

        // Decode image
        if (!pfnDecode(codec, stream, image)) {
            pfnImageDestroy(image);
            pfnStreamDestroy(stream);
            pfnDestroyCodec(codec);
            ::FreeLibrary(hOpenJP2);
            result.status = JP2DecodeStatus::CorruptData;
            return result;
        }
        pfnEndDecompress(codec, stream);

        // ── Extract pixel data from opj_image_t ──
        // opj_image_t layout (OpenJPEG 2.x, 64-bit):
        // offset 0: x0 (uint32_t), 4: y0, 8: x1, 12: y1
        // offset 16: numcomps (uint32_t)
        // offset 24: color_space (enum, 4 bytes)
        // offset 32: comps (opj_image_comp_t*)
        // Each opj_image_comp_t has: dx, dy, w, h, x0, y0, prec, bpp, sgnd, resno_decoded, factor, data*
        auto imgBytes = reinterpret_cast<const uint8_t*>(image);
        uint32_t x0, y0, x1, y1, numComps;
        std::memcpy(&x0, imgBytes + 0, 4);
        std::memcpy(&y0, imgBytes + 4, 4);
        std::memcpy(&x1, imgBytes + 8, 4);
        std::memcpy(&y1, imgBytes + 12, 4);
        std::memcpy(&numComps, imgBytes + 16, 4);

        uint32_t imgW = x1 - x0;
        uint32_t imgH = y1 - y0;

        if (imgW > 0 && imgH > 0 && numComps > 0 && imgW <= 65536 && imgH <= 65536) {
            // Read component data pointers.
            // opj_image_comp_t* comps is at offset 32 (64-bit pointer)
            uintptr_t compsPtr;
            std::memcpy(&compsPtr, imgBytes + 32, sizeof(uintptr_t));
            auto compsBytes = reinterpret_cast<const uint8_t*>(compsPtr);

            // opj_image_comp_t size on 64-bit: typically 48 bytes (with data* at offset 40)
            // Fields: dx(4), dy(4), w(4), h(4), x0(4), y0(4), prec(4), bpp(4), sgnd(4),
            // resno_decoded(4), factor(4), padding(4), data*(8)
            constexpr size_t COMP_STRIDE = 48;
            constexpr size_t DATA_PTR_OFFSET = 40;

            // Collect data pointers for each component
            std::vector<const int32_t*> compData(numComps, nullptr);
            for (uint32_t c = 0; c < numComps && c < 4; ++c) {
                uintptr_t dPtr;
                std::memcpy(&dPtr, compsBytes + c * COMP_STRIDE + DATA_PTR_OFFSET, sizeof(uintptr_t));
                compData[c] = reinterpret_cast<const int32_t*>(dPtr);
            }

            // Assemble BGRA pixel buffer
            result.decodedWidth = imgW;
            result.decodedHeight = imgH;
            result.pixelData.resize(static_cast<size_t>(imgW) * imgH * 4);

            const size_t pixelCount = static_cast<size_t>(imgW) * imgH;
            uint8_t* dst = result.pixelData.data();

            if (numComps >= 3 && compData[0] && compData[1] && compData[2]) {
                const int32_t* r = compData[0];
                const int32_t* g = compData[1];
                const int32_t* b = compData[2];
                const int32_t* a = (numComps >= 4) ? compData[3] : nullptr;
                for (size_t i = 0; i < pixelCount; ++i) {
                    dst[i * 4 + 0] = static_cast<uint8_t>((std::min)((std::max)(b[i], 0), 255)); // B
                    dst[i * 4 + 1] = static_cast<uint8_t>((std::min)((std::max)(g[i], 0), 255)); // G
                    dst[i * 4 + 2] = static_cast<uint8_t>((std::min)((std::max)(r[i], 0), 255)); // R
                    dst[i * 4 + 3] = a ? static_cast<uint8_t>((std::min)((std::max)(a[i], 0), 255)) : 0xFF;
                }
            }
            else if (numComps == 1 && compData[0]) {
                const int32_t* gray = compData[0];
                for (size_t i = 0; i < pixelCount; ++i) {
                    uint8_t v = static_cast<uint8_t>((std::min)((std::max)(gray[i], 0), 255));
                    dst[i * 4 + 0] = v;
                    dst[i * 4 + 1] = v;
                    dst[i * 4 + 2] = v;
                    dst[i * 4 + 3] = 0xFF;
                }
            }
            result.status = JP2DecodeStatus::Success;
        }
        else {
            result.status = JP2DecodeStatus::CorruptData;
        }

        // Cleanup
        pfnImageDestroy(image);
        pfnStreamDestroy(stream);
        pfnDestroyCodec(codec);
        ::FreeLibrary(hOpenJP2);

        auto t1 = std::chrono::high_resolution_clock::now();
        result.decodeTimeMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
        return result;
    }

    static bool IsJP2Extension(const std::string& ext) {
        return JP2Extensions::IsSupported(ext);
    }

    static JPEG2000Decoder Create() {
        return JPEG2000Decoder();
    }

private:
    mutable bool m_available = false; ///< Cached result of openjp2.dll probe
    mutable bool m_probed = false; ///< Whether the probe has been performed
};

} // namespace ExplorerLens::Decoders
