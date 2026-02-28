//==============================================================================
// ExplorerLens Engine — FITS Decoder Completion
// Minimal FITS reader for 2D images without cfitsio dependency.
// Supports SIMPLE FITS with BITPIX 8/16/32/-32/-64 pixel types.
//==============================================================================
#pragma once
#include "FITSDecoder.h"
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

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
 static bool IsFITSFile(const uint8_t *data, size_t size) {
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
 static const wchar_t *BitpixName(FITSBitpix bp) {
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
 static size_t CalculateDataSize(const FITSImageInfo &info) {
 size_t bpp = BytesPerPixel(info.bitpix);
 uint32_t depth = (info.naxis3 > 0) ? info.naxis3 : 1;
 return static_cast<size_t>(info.naxis1) * info.naxis2 * depth * bpp;
 }

 /// Validate parsed FITS info
 static bool ValidateInfo(const FITSImageInfo &info) {
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
