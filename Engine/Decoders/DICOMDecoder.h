#pragma once
//==============================================================================
// DICOMDecoder
// Medical imaging DICOM format decoder for thumbnail generation.
// Supports .dcm, .dicom files with basic pixel data extraction.
//
// Design:
//   - Reads DICOM Part 10 file header (128-byte preamble + "DICM" magic)
//   - Parses data elements (group, element, VR, length, value)
//   - Extracts Rows, Columns, BitsAllocated, PhotometricInterpretation
//   - Supports MONOCHROME1/2 and RGB photometric interpretations
//   - Window/level adjustment for CT/MR data
//   - Generates thumbnail from pixel data or embedded icon
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// DICOM photometric interpretation
enum class DICOMPhotometric : uint8_t {
  Unknown,
  Monochrome1, ///< Min = white
  Monochrome2, ///< Min = black (most common)
  RGB,
  YBR_Full,
  Palette
};

/// DICOM transfer syntax
enum class DICOMTransferSyntax : uint8_t {
  ImplicitVRLittleEndian, ///< 1.2.840.10008.1.2
  ExplicitVRLittleEndian, ///< 1.2.840.10008.1.2.1
  ExplicitVRBigEndian,    ///< 1.2.840.10008.1.2.2
  JPEGBaseline,           ///< 1.2.840.10008.1.2.4.50
  JPEG2000,               ///< 1.2.840.10008.1.2.4.90
  JPEGLossless,           ///< 1.2.840.10008.1.2.4.70
  JPEG2000Lossless,       ///< 1.2.840.10008.1.2.4.91
  RLELossless,            ///< 1.2.840.10008.1.2.5
  Unsupported
};

/// DICOM window/level settings
struct DICOMWindowLevel {
  double windowCenter = 40.0;
  double windowWidth = 400.0;
  bool isDefault = true;
};

/// DICOM image metadata
struct DICOMImageInfo {
  uint32_t rows = 0;
  uint32_t columns = 0;
  uint16_t bitsAllocated = 16;
  uint16_t bitsStored = 12;
  uint16_t highBit = 11;
  uint16_t samplesPerPixel = 1;
  uint16_t pixelRepresentation = 0; ///< 0=unsigned, 1=signed
  DICOMPhotometric photometric = DICOMPhotometric::Monochrome2;
  DICOMTransferSyntax transferSyntax =
      DICOMTransferSyntax::ExplicitVRLittleEndian;
  DICOMWindowLevel windowLevel;
  std::wstring patientName;
  std::wstring modality;
  uint32_t numberOfFrames = 1;
  size_t pixelDataOffset = 0;
  size_t pixelDataLength = 0;
};

//==============================================================================
// DICOMDecoder
//==============================================================================
class DICOMDecoder {
public:
  DICOMDecoder();

  /// Check if file has valid DICOM header
  static bool IsDICOMFile(const uint8_t *data, size_t size);

  /// Parse DICOM metadata from file data
  bool ParseHeader(const uint8_t *data, size_t size);

  /// Get parsed image info
  const DICOMImageInfo &GetImageInfo() const { return m_info; }

  /// Apply window/level to pixel value (returns 0-255)
  uint8_t ApplyWindowLevel(int16_t pixelValue) const;

  /// Extract thumbnail (returns BGRA pixel data)
  std::vector<uint8_t> ExtractThumbnail(const uint8_t *data, size_t size,
                                        uint32_t thumbWidth,
                                        uint32_t thumbHeight) const;

  /// Get supported extensions
  static const wchar_t *const *GetExtensions();
  static uint32_t GetExtensionCount();

  /// Get photometric name
  static const wchar_t *GetPhotometricName(DICOMPhotometric p);
  static const wchar_t *GetTransferSyntaxName(DICOMTransferSyntax ts);

private:
  DICOMImageInfo m_info;

  bool ParseDataElement(const uint8_t *data, size_t size, size_t &offset);
  uint16_t ReadUint16LE(const uint8_t *data, size_t offset) const;
  uint32_t ReadUint32LE(const uint8_t *data, size_t offset) const;
};

} // namespace Engine
} // namespace ExplorerLens
