#pragma once
//==============================================================================
// FITSDecoder
// Astronomy FITS (Flexible Image Transport System) format decoder.
// Supports .fits, .fit, .fts files with basic HDU parsing.
//
// Design:
// - Reads primary HDU header (2880-byte blocks of 80-char keyword records)
// - Extracts NAXIS, NAXIS1, NAXIS2, BITPIX, BZERO, BSCALE
// - Supports BITPIX 8, 16, 32, -32(float), -64(double)
// - Auto-stretch with BZERO/BSCALE for display
// - Generates grayscale thumbnail from pixel data
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

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

//==============================================================================
// FITSDecoder
//==============================================================================
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

}} // namespace ExplorerLens::Engine

