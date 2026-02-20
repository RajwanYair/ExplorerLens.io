//==============================================================================
// DICOMDecoder — Sprint 199
// Medical imaging DICOM format decoder implementation
//==============================================================================

#include "DICOMDecoder.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace DarkThumbs { namespace Engine {

static const wchar_t* s_dicomExtensions[] = { L".dcm", L".dicom" };

DICOMDecoder::DICOMDecoder() {}

//------------------------------------------------------------------------------
bool DICOMDecoder::IsDICOMFile(const uint8_t* data, size_t size) {
    // DICOM Part 10 files have 128-byte preamble + "DICM" magic
    if (!data || size < 132) return false;
    return (data[128] == 'D' && data[129] == 'I' &&
            data[130] == 'C' && data[131] == 'M');
}

//------------------------------------------------------------------------------
uint16_t DICOMDecoder::ReadUint16LE(const uint8_t* data, size_t offset) const {
    return static_cast<uint16_t>(data[offset]) |
           (static_cast<uint16_t>(data[offset + 1]) << 8);
}

uint32_t DICOMDecoder::ReadUint32LE(const uint8_t* data, size_t offset) const {
    return static_cast<uint32_t>(data[offset]) |
           (static_cast<uint32_t>(data[offset + 1]) << 8) |
           (static_cast<uint32_t>(data[offset + 2]) << 16) |
           (static_cast<uint32_t>(data[offset + 3]) << 24);
}

//------------------------------------------------------------------------------
bool DICOMDecoder::ParseDataElement(const uint8_t* data, size_t size, size_t& offset) {
    if (offset + 8 > size) return false;

    uint16_t group = ReadUint16LE(data, offset);
    uint16_t element = ReadUint16LE(data, offset + 2);
    offset += 4;

    // Determine VR and length based on transfer syntax
    char vr[3] = { 0 };
    uint32_t length = 0;

    if (m_info.transferSyntax != DICOMTransferSyntax::ImplicitVRLittleEndian) {
        // Explicit VR
        if (offset + 4 > size) return false;
        vr[0] = static_cast<char>(data[offset]);
        vr[1] = static_cast<char>(data[offset + 1]);
        offset += 2;

        // Check for 4-byte length VRs
        bool longVR = (std::strcmp(vr, "OB") == 0 || std::strcmp(vr, "OW") == 0 ||
                       std::strcmp(vr, "OF") == 0 || std::strcmp(vr, "SQ") == 0 ||
                       std::strcmp(vr, "UC") == 0 || std::strcmp(vr, "UN") == 0 ||
                       std::strcmp(vr, "UR") == 0 || std::strcmp(vr, "UT") == 0);
        if (longVR) {
            offset += 2; // skip reserved bytes
            if (offset + 4 > size) return false;
            length = ReadUint32LE(data, offset);
            offset += 4;
        } else {
            length = ReadUint16LE(data, offset);
            offset += 2;
        }
    } else {
        // Implicit VR: always 4-byte length
        if (offset + 4 > size) return false;
        length = ReadUint32LE(data, offset);
        offset += 4;
    }

    // Handle undefined length
    if (length == 0xFFFFFFFF) {
        // Skip sequences with undefined length for now
        offset = size; // bail out
        return false;
    }

    if (offset + length > size) return false;

    // Extract key DICOM tags
    if (group == 0x0028) {
        switch (element) {
            case 0x0010: // Rows
                if (length >= 2) m_info.rows = ReadUint16LE(data, offset);
                break;
            case 0x0011: // Columns
                if (length >= 2) m_info.columns = ReadUint16LE(data, offset);
                break;
            case 0x0100: // BitsAllocated
                if (length >= 2) m_info.bitsAllocated = ReadUint16LE(data, offset);
                break;
            case 0x0101: // BitsStored
                if (length >= 2) m_info.bitsStored = ReadUint16LE(data, offset);
                break;
            case 0x0102: // HighBit
                if (length >= 2) m_info.highBit = ReadUint16LE(data, offset);
                break;
            case 0x0103: // PixelRepresentation
                if (length >= 2) m_info.pixelRepresentation = ReadUint16LE(data, offset);
                break;
            case 0x0002: // SamplesPerPixel
                if (length >= 2) m_info.samplesPerPixel = ReadUint16LE(data, offset);
                break;
            case 0x0004: { // PhotometricInterpretation
                std::string pi(reinterpret_cast<const char*>(data + offset), length);
                // Trim trailing spaces
                while (!pi.empty() && pi.back() == ' ') pi.pop_back();
                if (pi == "MONOCHROME1") m_info.photometric = DICOMPhotometric::Monochrome1;
                else if (pi == "MONOCHROME2") m_info.photometric = DICOMPhotometric::Monochrome2;
                else if (pi == "RGB") m_info.photometric = DICOMPhotometric::RGB;
                else if (pi == "YBR_FULL") m_info.photometric = DICOMPhotometric::YBR_Full;
                else if (pi == "PALETTE COLOR") m_info.photometric = DICOMPhotometric::Palette;
                break;
            }
            case 0x1050: { // WindowCenter
                std::string val(reinterpret_cast<const char*>(data + offset), length);
                try { m_info.windowLevel.windowCenter = std::stod(val); m_info.windowLevel.isDefault = false; } catch (...) {}
                break;
            }
            case 0x1051: { // WindowWidth
                std::string val(reinterpret_cast<const char*>(data + offset), length);
                try { m_info.windowLevel.windowWidth = std::stod(val); m_info.windowLevel.isDefault = false; } catch (...) {}
                break;
            }
        }
    } else if (group == 0x0008 && element == 0x0060) { // Modality
        std::string mod(reinterpret_cast<const char*>(data + offset), length);
        while (!mod.empty() && mod.back() == ' ') mod.pop_back();
        m_info.modality = std::wstring(mod.begin(), mod.end());
    } else if (group == 0x7FE0 && element == 0x0010) { // PixelData
        m_info.pixelDataOffset = offset;
        m_info.pixelDataLength = length;
    }

    offset += length;
    return true;
}

//------------------------------------------------------------------------------
bool DICOMDecoder::ParseHeader(const uint8_t* data, size_t size) {
    if (!IsDICOMFile(data, size)) return false;

    m_info = DICOMImageInfo{};
    m_info.transferSyntax = DICOMTransferSyntax::ExplicitVRLittleEndian;

    // Skip 132-byte preamble + magic
    size_t offset = 132;

    // Parse File Meta Information (group 0x0002) then image data elements
    while (offset < size) {
        if (!ParseDataElement(data, size, offset)) break;
    }

    return (m_info.rows > 0 && m_info.columns > 0);
}

//------------------------------------------------------------------------------
uint8_t DICOMDecoder::ApplyWindowLevel(int16_t pixelValue) const {
    double wc = m_info.windowLevel.windowCenter;
    double ww = m_info.windowLevel.windowWidth;
    if (ww <= 0) ww = 1.0;
    double lower = wc - ww / 2.0;
    double upper = wc + ww / 2.0;
    double normalized = (pixelValue - lower) / (upper - lower);
    normalized = (std::max)(0.0, (std::min)(1.0, normalized));
    if (m_info.photometric == DICOMPhotometric::Monochrome1) {
        normalized = 1.0 - normalized; // Invert for MONOCHROME1
    }
    return static_cast<uint8_t>(normalized * 255.0);
}

//------------------------------------------------------------------------------
std::vector<uint8_t> DICOMDecoder::ExtractThumbnail(const uint8_t* data, size_t size,
                                                     uint32_t thumbWidth, uint32_t thumbHeight) const {
    std::vector<uint8_t> thumbnail(thumbWidth * thumbHeight * 4, 0);

    if (m_info.pixelDataOffset == 0 || m_info.rows == 0 || m_info.columns == 0)
        return thumbnail;

    // Simple nearest-neighbor resize from pixel data to thumbnail
    for (uint32_t y = 0; y < thumbHeight; ++y) {
        uint32_t srcY = y * m_info.rows / thumbHeight;
        for (uint32_t x = 0; x < thumbWidth; ++x) {
            uint32_t srcX = x * m_info.columns / thumbWidth;
            size_t dstIdx = (y * thumbWidth + x) * 4;

            if (m_info.bitsAllocated == 16 && m_info.samplesPerPixel == 1) {
                size_t srcIdx = m_info.pixelDataOffset + (srcY * m_info.columns + srcX) * 2;
                if (srcIdx + 1 < size) {
                    int16_t val = static_cast<int16_t>(ReadUint16LE(data, srcIdx));
                    uint8_t gray = ApplyWindowLevel(val);
                    thumbnail[dstIdx + 0] = gray; // B
                    thumbnail[dstIdx + 1] = gray; // G
                    thumbnail[dstIdx + 2] = gray; // R
                    thumbnail[dstIdx + 3] = 255;  // A
                }
            } else if (m_info.bitsAllocated == 8 && m_info.samplesPerPixel == 3) {
                size_t srcIdx = m_info.pixelDataOffset + (srcY * m_info.columns + srcX) * 3;
                if (srcIdx + 2 < size) {
                    thumbnail[dstIdx + 0] = data[srcIdx + 2]; // B
                    thumbnail[dstIdx + 1] = data[srcIdx + 1]; // G
                    thumbnail[dstIdx + 2] = data[srcIdx + 0]; // R
                    thumbnail[dstIdx + 3] = 255;               // A
                }
            } else if (m_info.bitsAllocated == 8 && m_info.samplesPerPixel == 1) {
                size_t srcIdx = m_info.pixelDataOffset + (srcY * m_info.columns + srcX);
                if (srcIdx < size) {
                    uint8_t gray = data[srcIdx];
                    if (m_info.photometric == DICOMPhotometric::Monochrome1)
                        gray = 255 - gray;
                    thumbnail[dstIdx + 0] = gray;
                    thumbnail[dstIdx + 1] = gray;
                    thumbnail[dstIdx + 2] = gray;
                    thumbnail[dstIdx + 3] = 255;
                }
            }
        }
    }
    return thumbnail;
}

//------------------------------------------------------------------------------
const wchar_t* const* DICOMDecoder::GetExtensions() {
    return s_dicomExtensions;
}

uint32_t DICOMDecoder::GetExtensionCount() {
    return static_cast<uint32_t>(std::size(s_dicomExtensions));
}

//------------------------------------------------------------------------------
const wchar_t* DICOMDecoder::GetPhotometricName(DICOMPhotometric p) {
    switch (p) {
        case DICOMPhotometric::Monochrome1: return L"MONOCHROME1";
        case DICOMPhotometric::Monochrome2: return L"MONOCHROME2";
        case DICOMPhotometric::RGB:         return L"RGB";
        case DICOMPhotometric::YBR_Full:    return L"YBR_FULL";
        case DICOMPhotometric::Palette:     return L"PALETTE COLOR";
        default: return L"UNKNOWN";
    }
}

const wchar_t* DICOMDecoder::GetTransferSyntaxName(DICOMTransferSyntax ts) {
    switch (ts) {
        case DICOMTransferSyntax::ImplicitVRLittleEndian: return L"Implicit VR Little Endian";
        case DICOMTransferSyntax::ExplicitVRLittleEndian: return L"Explicit VR Little Endian";
        case DICOMTransferSyntax::ExplicitVRBigEndian:    return L"Explicit VR Big Endian";
        case DICOMTransferSyntax::JPEGBaseline:           return L"JPEG Baseline";
        case DICOMTransferSyntax::JPEG2000:               return L"JPEG 2000";
        default: return L"Unsupported";
    }
}

}} // namespace DarkThumbs::Engine
