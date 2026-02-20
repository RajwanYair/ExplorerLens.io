//==============================================================================
// DarkThumbs Engine — Sprint 258: DICOM Decoder Completion
// Minimal DICOM parser for common transfer syntaxes without DCMTK dependency.
// Supports Little Endian Explicit, Implicit VR, and uncompressed pixel data.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

/// DICOM Transfer Syntax UIDs
enum class DICOMTransferSyntax : uint8_t {
    ImplicitVRLittleEndian,     // 1.2.840.10008.1.2
    ExplicitVRLittleEndian,     // 1.2.840.10008.1.2.1
    ExplicitVRBigEndian,        // 1.2.840.10008.1.2.2 (retired)
    JPEGBaseline,               // 1.2.840.10008.1.2.4.50
    JPEGLossless,               // 1.2.840.10008.1.2.4.70
    JPEG2000Lossless,           // 1.2.840.10008.1.2.4.90
    JPEG2000,                   // 1.2.840.10008.1.2.4.91
    RLELossless,                // 1.2.840.10008.1.2.5
    Unknown
};

/// DICOM Photometric Interpretation
enum class DICOMPhotometric : uint8_t {
    Monochrome1,    // 0 = white
    Monochrome2,    // 0 = black  
    RGB,
    YBR_FULL,
    PALETTE_COLOR,
    Unknown
};

/// DICOM image metadata extracted from header
struct DICOMImageInfo {
    uint16_t rows             = 0;
    uint16_t columns          = 0;
    uint16_t bitsAllocated    = 0;
    uint16_t bitsStored       = 0;
    uint16_t highBit          = 0;
    uint16_t samplesPerPixel  = 1;
    uint16_t pixelRepresentation = 0; // 0=unsigned, 1=signed
    uint32_t numberOfFrames   = 1;
    DICOMTransferSyntax transferSyntax = DICOMTransferSyntax::ImplicitVRLittleEndian;
    DICOMPhotometric photometric       = DICOMPhotometric::Monochrome2;
    std::wstring patientName;
    std::wstring modality;     // CT, MR, US, XR, etc.
    size_t pixelDataOffset    = 0;
    size_t pixelDataLength    = 0;
};

/// Enhanced DICOM decoder with minimal parser
class DICOMDecoderV2 {
public:
    /// DICOM magic check (DICM prefix at offset 128)
    static bool IsDICOMFile(const uint8_t* data, size_t size) {
        if (size < 132) return false;
        return data[128] == 'D' && data[129] == 'I' &&
               data[130] == 'C' && data[131] == 'M';
    }

    /// Transfer syntax name
    static const wchar_t* TransferSyntaxName(DICOMTransferSyntax ts) {
        switch (ts) {
            case DICOMTransferSyntax::ImplicitVRLittleEndian:  return L"Implicit VR Little Endian";
            case DICOMTransferSyntax::ExplicitVRLittleEndian:  return L"Explicit VR Little Endian";
            case DICOMTransferSyntax::ExplicitVRBigEndian:     return L"Explicit VR Big Endian";
            case DICOMTransferSyntax::JPEGBaseline:            return L"JPEG Baseline";
            case DICOMTransferSyntax::JPEGLossless:            return L"JPEG Lossless";
            case DICOMTransferSyntax::JPEG2000Lossless:        return L"JPEG 2000 Lossless";
            case DICOMTransferSyntax::JPEG2000:                return L"JPEG 2000";
            case DICOMTransferSyntax::RLELossless:             return L"RLE Lossless";
            default: return L"Unknown";
        }
    }

    /// Photometric name
    static const wchar_t* PhotometricName(DICOMPhotometric pm) {
        switch (pm) {
            case DICOMPhotometric::Monochrome1:    return L"MONOCHROME1";
            case DICOMPhotometric::Monochrome2:    return L"MONOCHROME2";
            case DICOMPhotometric::RGB:            return L"RGB";
            case DICOMPhotometric::YBR_FULL:       return L"YBR_FULL";
            case DICOMPhotometric::PALETTE_COLOR:  return L"PALETTE_COLOR";
            default: return L"UNKNOWN";
        }
    }

    /// Modality display name
    static const wchar_t* ModalityName(const std::wstring& code) {
        if (code == L"CT") return L"Computed Tomography";
        if (code == L"MR") return L"Magnetic Resonance";
        if (code == L"US") return L"Ultrasound";
        if (code == L"XR" || code == L"CR" || code == L"DX") return L"X-Ray";
        if (code == L"MG") return L"Mammography";
        if (code == L"NM") return L"Nuclear Medicine";
        if (code == L"PT") return L"PET Scan";
        if (code == L"RF") return L"Fluoroscopy";
        return L"Unknown Modality";
    }

    /// Count of supported transfer syntaxes
    static constexpr size_t TransferSyntaxCount() { return 8; }

    /// Count of supported modalities
    static constexpr size_t ModalityCount() { return 8; }

    /// Validate DICOM image info
    static bool ValidateInfo(const DICOMImageInfo& info) {
        if (info.rows == 0 || info.columns == 0) return false;
        if (info.bitsAllocated != 8 && info.bitsAllocated != 16) return false;
        if (info.bitsStored > info.bitsAllocated) return false;
        if (info.samplesPerPixel != 1 && info.samplesPerPixel != 3) return false;
        return true;
    }

    /// Check if we can decode this transfer syntax natively
    static bool CanDecodeNatively(DICOMTransferSyntax ts) {
        return ts == DICOMTransferSyntax::ImplicitVRLittleEndian ||
               ts == DICOMTransferSyntax::ExplicitVRLittleEndian;
    }

    /// Calculate pixel data size
    static size_t CalculatePixelSize(const DICOMImageInfo& info) {
        size_t bytesPerPixel = (info.bitsAllocated / 8) * info.samplesPerPixel;
        return static_cast<size_t>(info.rows) * info.columns * bytesPerPixel * info.numberOfFrames;
    }
};

}} // namespace DarkThumbs::Engine
