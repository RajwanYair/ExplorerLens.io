// FITSDecoderV2.h — Canonical FITS Decoder (V1 + V2 consolidated)
// Copyright (c) 2026 ExplorerLens Project
//
// Consolidated FITS decoder: V1 types/class + V2 enhanced decoder.
// Minimal FITS reader for 2D images without cfitsio dependency.
//
#pragma once
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ── FITSDecoder V1 types (consolidated from FITSDecoder.h) ─────────────────

/// FITS data type (BITPIX values)
enum class FITSBitpix : int16_t {
    UInt8 = 8,
    Int16 = 16,
    Int32 = 32,
    Float32 = -32,
    Float64 = -64
};

/// FITS image metadata
struct FITSImageInfo {
    uint32_t naxis = 0; ///< Number of axes
    uint32_t naxis1 = 0; ///< Width (pixels)
    uint32_t naxis2 = 0; ///< Height (pixels)
    uint32_t naxis3 = 0; ///< Depth (for RGB cubes)
    FITSBitpix bitpix = FITSBitpix::Int16;
    double bzero = 0.0; ///< Data offset
    double bscale = 1.0; ///< Data scale factor
    double datamin = 0.0;
    double datamax = 0.0;
    bool hasMinMax = false;
    std::wstring objectName;
    std::wstring instrument;
    std::wstring telescope;
    size_t dataOffset = 0; ///< Start of pixel data
};

/// FITS stretch algorithm for visualization
enum class FITSStretch : uint8_t {
    Linear,
    Logarithmic,
    SquareRoot,
    Asinh
};

/// FITS stretch mode (extended, for Sprint 1001-1010 decoders)
enum class FITSStretchMode : uint8_t {
    Linear,
    Logarithmic,
    Sqrt,
    Asinh,
    HistogramEqualization
};

/// FITS false-color lookup table
enum class FITSColorLUT : uint8_t {
    Grayscale,
    Heat,
    Cool,
    Rainbow,
    STScIDefault
};

/// FITS header metadata (extended)
struct FITSHeaderInfo {
    int32_t     bitpix = 16;
    uint32_t    naxis = 2;
    uint32_t    naxis1 = 0;
    uint32_t    naxis2 = 0;
    double      bzero = 0.0;
    double      bscale = 1.0;
    std::string objectName;
    std::string telescopeName;
    std::string dateObs;
    double      expTime = 0.0;
};

/// FITS decoder (V1)
class FITSDecoder {
public:
    FITSDecoder();

    /// Check if file has valid FITS header
    static bool IsFITSFile(const uint8_t* data, size_t size);

    /// Parse FITS header keyword records
    bool ParseHeader(const uint8_t* data, size_t size);

    /// Get parsed image info
    const FITSImageInfo& GetImageInfo() const { return m_info; }

    /// Apply stretch and convert to 0-255
    uint8_t ApplyStretch(double physicalValue, double vmin, double vmax,
        FITSStretch stretch = FITSStretch::Linear) const;

    /// Compute min/max from data (for auto-stretch)
    void ComputeMinMax(const uint8_t* data, size_t size);

    /// Extract thumbnail (returns BGRA pixel data)
    std::vector<uint8_t> ExtractThumbnail(const uint8_t* data, size_t size,
        uint32_t thumbWidth, uint32_t thumbHeight) const;

    /// Get supported extensions
    static const wchar_t* const* GetExtensions();
    static uint32_t GetExtensionCount();

    /// Get BITPIX name
    static const wchar_t* GetBitpixName(FITSBitpix bp);
    static const wchar_t* GetStretchName(FITSStretch s);
    static size_t GetBytesPerPixel(FITSBitpix bp);

private:
    FITSImageInfo m_info;

    std::string ParseKeyword(const char* record, std::string& value) const;
    double ReadPixelValue(const uint8_t* data, size_t offset) const;
};

// ── FITSDecoderV2 (enhanced decoder) ───────────────────────────────────────

/// FITS image axis ordering
enum class FITSAxisOrder : uint8_t {
    RowMajor, // NAXIS1=width, NAXIS2=height
    ColumnMajor, // NAXIS1=height, NAXIS2=width (rare)
    Cube // NAXIS3 present — select first slice
};

/// FITS header keyword-value pair
struct FITSKeyword {
    std::string keyword; // 8-char keyword
    std::string value; // Associated value
    std::string comment; // Optional comment
};

/// Enhanced FITS decoder with minimal parser
class FITSDecoderV2 {
public:
    /// FITS magic check — starts with "SIMPLE ="
    static bool IsFITSFile(const uint8_t* data, size_t size) {
        if (size < 80)
            return false;
        // First card must be "SIMPLE = "
        return data[0] == 'S' && data[1] == 'I' && data[2] == 'M' &&
            data[3] == 'P' && data[4] == 'L' && data[5] == 'E' &&
            data[6] == ' ' && data[7] == ' ';
    }

    /// FITS header block size (always 2880 bytes)
    static constexpr size_t HEADER_BLOCK_SIZE = 2880;

    /// FITS card image size (always 80 bytes)
    static constexpr size_t CARD_SIZE = 80;

    /// Cards per header block
    static constexpr size_t CARDS_PER_BLOCK = HEADER_BLOCK_SIZE / CARD_SIZE; // 36

    /// Bytes per pixel for given BITPIX
    static size_t BytesPerPixel(FITSBitpix bp) {
        switch (bp) {
        case FITSBitpix::UInt8:
            return 1;
        case FITSBitpix::Int16:
            return 2;
        case FITSBitpix::Int32:
            return 4;
        case FITSBitpix::Float32:
            return 4;
        case FITSBitpix::Float64:
            return 8;
        default:
            return 0;
        }
    }

    /// Bitpix display name
    static const wchar_t* BitpixName(FITSBitpix bp) {
        switch (bp) {
        case FITSBitpix::UInt8:
            return L"8-bit unsigned";
        case FITSBitpix::Int16:
            return L"16-bit signed";
        case FITSBitpix::Int32:
            return L"32-bit signed";
        case FITSBitpix::Float32:
            return L"32-bit float";
        case FITSBitpix::Float64:
            return L"64-bit float";
        default:
            return L"Unknown";
        }
    }

    /// Count of supported BITPIX types
    static constexpr size_t BitpixTypeCount() { return 5; }

    /// Calculate total pixel data size
    static size_t CalculateDataSize(const FITSImageInfo& info) {
        size_t bpp = BytesPerPixel(info.bitpix);
        uint32_t depth = (info.naxis3 > 0) ? info.naxis3 : 1;
        return static_cast<size_t>(info.naxis1) * info.naxis2 * depth * bpp;
    }

    /// Validate parsed FITS info
    static bool ValidateInfo(const FITSImageInfo& info) {
        if (info.naxis < 2)
            return false;
        if (info.naxis1 == 0 || info.naxis2 == 0)
            return false;
        if (BytesPerPixel(info.bitpix) == 0)
            return false;
        return true;
    }

    /// Apply BSCALE and BZERO to a pixel value
    static double ApplyScaling(double raw, double bscale, double bzero) {
        return raw * bscale + bzero;
    }

    /// Normalize a value to 0-255 range
    static uint8_t NormalizeTo8Bit(double value, double minVal, double maxVal) {
        if (maxVal <= minVal)
            return 128;
        double normalized = (value - minVal) / (maxVal - minVal);
        if (normalized < 0.0)
            normalized = 0.0;
        if (normalized > 1.0)
            normalized = 1.0;
        return static_cast<uint8_t>(normalized * 255.0);
    }

    /// Asinh stretch for astronomical images
    static double AsinhStretch(double value, double softening = 0.05) {
        return std::asinh(value / softening) / std::asinh(1.0 / softening);
    }
};

} // namespace Engine
} // namespace ExplorerLens
