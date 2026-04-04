// DICOMDecoderV2.h — Canonical DICOM Decoder (V1 + V2 consolidated)
// Copyright (c) 2026 ExplorerLens Project
//
// Consolidated DICOM decoder: V1 types/class + V2 enhanced decoder.
// Minimal DICOM parser for common transfer syntaxes without DCMTK dependency.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ── DICOMDecoder V1 types (consolidated from DICOMDecoder.h) ───────────────

/// DICOM photometric interpretation
enum class DICOMPhotometric : uint8_t {
    Unknown,
    Monochrome1,  ///< Min = white
    Monochrome2,  ///< Min = black (most common)
    RGB,
    YBR_Full,
    Palette
};

/// DICOM transfer syntax
enum class DICOMTransferSyntax : uint8_t {
    ImplicitVRLittleEndian,  ///< 1.2.840.10008.1.2
    ExplicitVRLittleEndian,  ///< 1.2.840.10008.1.2.1
    ExplicitVRBigEndian,     ///< 1.2.840.10008.1.2.2
    JPEGBaseline,            ///< 1.2.840.10008.1.2.4.50
    JPEG2000,                ///< 1.2.840.10008.1.2.4.90
    JPEGLossless,            ///< 1.2.840.10008.1.2.4.70
    JPEG2000Lossless,        ///< 1.2.840.10008.1.2.4.91
    RLELossless,             ///< 1.2.840.10008.1.2.5
    Unsupported
};

/// DICOM window/level settings
struct DICOMWindowLevel
{
    double windowCenter = 40.0;
    double windowWidth = 400.0;
    bool isDefault = true;
};

/// DICOM image metadata
struct DICOMImageInfo
{
    uint32_t rows = 0;
    uint32_t columns = 0;
    uint16_t bitsAllocated = 16;
    uint16_t bitsStored = 12;
    uint16_t highBit = 11;
    uint16_t samplesPerPixel = 1;
    uint16_t pixelRepresentation = 0;  ///< 0=unsigned, 1=signed
    DICOMPhotometric photometric = DICOMPhotometric::Monochrome2;
    DICOMTransferSyntax transferSyntax = DICOMTransferSyntax::ExplicitVRLittleEndian;
    DICOMWindowLevel windowLevel;
    std::wstring patientName;
    std::wstring modality;
    uint32_t numberOfFrames = 1;
    size_t pixelDataOffset = 0;
    size_t pixelDataLength = 0;
};

/// DICOM decoder (V1)
class DICOMDecoder
{
  public:
    DICOMDecoder();

    /// Check if file has valid DICOM header
    static bool IsDICOMFile(const uint8_t* data, size_t size);

    /// Parse DICOM metadata from file data
    bool ParseHeader(const uint8_t* data, size_t size);

    /// Get parsed image info
    const DICOMImageInfo& GetImageInfo() const
    {
        return m_info;
    }

    /// Apply window/level to pixel value (returns 0-255)
    uint8_t ApplyWindowLevel(int16_t pixelValue) const;

    /// Extract thumbnail (returns BGRA pixel data)
    std::vector<uint8_t> ExtractThumbnail(const uint8_t* data, size_t size, uint32_t thumbWidth,
                                          uint32_t thumbHeight) const;

    /// Get supported extensions
    static const wchar_t* const* GetExtensions();
    static uint32_t GetExtensionCount();

    /// Get photometric name
    static const wchar_t* GetPhotometricName(DICOMPhotometric p);
    static const wchar_t* GetTransferSyntaxName(DICOMTransferSyntax ts);

  private:
    DICOMImageInfo m_info;

    bool ParseDataElement(const uint8_t* data, size_t size, size_t& offset);
    uint16_t ReadUint16LE(const uint8_t* data, size_t offset) const;
    uint32_t ReadUint32LE(const uint8_t* data, size_t offset) const;
};

// ── DICOMDecoderV2 (enhanced decoder) ──────────────────────────────────────

/// Enhanced DICOM decoder with minimal parser
class DICOMDecoderV2
{
  public:
    /// DICOM magic check (DICM prefix at offset 128)
    static bool IsDICOMFile(const uint8_t* data, size_t size)
    {
        if (size < 132)
            return false;
        return data[128] == 'D' && data[129] == 'I' && data[130] == 'C' && data[131] == 'M';
    }

    /// Transfer syntax name
    static const wchar_t* TransferSyntaxName(DICOMTransferSyntax ts)
    {
        switch (ts) {
            case DICOMTransferSyntax::ImplicitVRLittleEndian:
                return L"Implicit VR Little Endian";
            case DICOMTransferSyntax::ExplicitVRLittleEndian:
                return L"Explicit VR Little Endian";
            case DICOMTransferSyntax::ExplicitVRBigEndian:
                return L"Explicit VR Big Endian";
            case DICOMTransferSyntax::JPEGBaseline:
                return L"JPEG Baseline";
            case DICOMTransferSyntax::JPEG2000:
                return L"JPEG 2000";
            case DICOMTransferSyntax::JPEGLossless:
                return L"JPEG Lossless";
            case DICOMTransferSyntax::JPEG2000Lossless:
                return L"JPEG 2000 Lossless";
            case DICOMTransferSyntax::RLELossless:
                return L"RLE Lossless";
            case DICOMTransferSyntax::Unsupported:
                return L"Unsupported";
            default:
                return L"Unknown";
        }
    }

    /// Photometric name
    static const wchar_t* PhotometricName(DICOMPhotometric pm)
    {
        switch (pm) {
            case DICOMPhotometric::Unknown:
                return L"UNKNOWN";
            case DICOMPhotometric::Monochrome1:
                return L"MONOCHROME1";
            case DICOMPhotometric::Monochrome2:
                return L"MONOCHROME2";
            case DICOMPhotometric::RGB:
                return L"RGB";
            case DICOMPhotometric::YBR_Full:
                return L"YBR_FULL";
            case DICOMPhotometric::Palette:
                return L"PALETTE_COLOR";
            default:
                return L"UNKNOWN";
        }
    }

    /// Modality display name
    static const wchar_t* ModalityName(const std::wstring& code)
    {
        if (code == L"CT")
            return L"Computed Tomography";
        if (code == L"MR")
            return L"Magnetic Resonance";
        if (code == L"US")
            return L"Ultrasound";
        if (code == L"XR" || code == L"CR" || code == L"DX")
            return L"X-Ray";
        if (code == L"MG")
            return L"Mammography";
        if (code == L"NM")
            return L"Nuclear Medicine";
        if (code == L"PT")
            return L"PET Scan";
        if (code == L"RF")
            return L"Fluoroscopy";
        return L"Unknown Modality";
    }

    /// Count of supported transfer syntaxes
    static constexpr size_t TransferSyntaxCount()
    {
        return 8;
    }

    /// Count of supported modalities
    static constexpr size_t ModalityCount()
    {
        return 8;
    }

    /// Validate DICOM image info
    static bool ValidateInfo(const DICOMImageInfo& info)
    {
        if (info.rows == 0 || info.columns == 0)
            return false;
        if (info.bitsAllocated != 8 && info.bitsAllocated != 16)
            return false;
        if (info.bitsStored > info.bitsAllocated)
            return false;
        if (info.samplesPerPixel != 1 && info.samplesPerPixel != 3)
            return false;
        return true;
    }

    /// Check if we can decode this transfer syntax natively
    static bool CanDecodeNatively(DICOMTransferSyntax ts)
    {
        return ts == DICOMTransferSyntax::ImplicitVRLittleEndian || ts == DICOMTransferSyntax::ExplicitVRLittleEndian;
    }

    /// Calculate pixel data size
    static size_t CalculatePixelSize(const DICOMImageInfo& info)
    {
        size_t bytesPerPixel = (info.bitsAllocated / 8) * info.samplesPerPixel;
        return static_cast<size_t>(info.rows) * info.columns * bytesPerPixel * info.numberOfFrames;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
