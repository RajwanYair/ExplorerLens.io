//==============================================================================
// FITSDecoder
// FITS astronomy image format decoder implementation
//==============================================================================

#include "FITSDecoder.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace ExplorerLens {
namespace Engine {

static const wchar_t* s_fitsExtensions[] = { L".fits", L".fit", L".fts" };

FITSDecoder::FITSDecoder() {}

//------------------------------------------------------------------------------
bool FITSDecoder::IsFITSFile(const uint8_t* data, size_t size) {
    // FITS files start with "SIMPLE  = " in first 80-char record
    // Keyword occupies bytes 0-7 (8 chars, space-padded), '=' at byte 8
    if (!data || size < 80) return false;
    return (std::memcmp(data, "SIMPLE  =", 9) == 0);
}

//------------------------------------------------------------------------------
std::string FITSDecoder::ParseKeyword(const char* record, std::string& value) const {
    // FITS keyword format: "KEYWORD = VALUE / COMMENT" (80 chars per record)
    std::string keyword(record, 8);
    // Trim trailing spaces from keyword
    while (!keyword.empty() && keyword.back() == ' ') keyword.pop_back();

    value.clear();
    if (record[8] == '=' && record[9] == ' ') {
        // Value starts at column 10
        std::string raw(record + 10, 70);
        // Trim trailing spaces and comments
        size_t slashPos = raw.find('/');
        if (slashPos != std::string::npos) raw.resize(slashPos);
        while (!raw.empty() && raw.back() == ' ') raw.pop_back();
        while (!raw.empty() && raw.front() == ' ') raw.erase(raw.begin());
        // Remove quotes for string values
        if (raw.size() >= 2 && raw.front() == '\'' && raw.back() == '\'') {
            raw = raw.substr(1, raw.size() - 2);
            while (!raw.empty() && raw.back() == ' ') raw.pop_back();
        }
        value = raw;
    }
    return keyword;
}

//------------------------------------------------------------------------------
bool FITSDecoder::ParseHeader(const uint8_t* data, size_t size) {
    if (!IsFITSFile(data, size)) return false;

    m_info = FITSImageInfo{};
    size_t offset = 0;

    // Parse 2880-byte header blocks
    while (offset + 80 <= size) {
        const char* record = reinterpret_cast<const char*>(data + offset);
        std::string value;
        std::string keyword = ParseKeyword(record, value);

        if (keyword == "END") {
            // Advance to next 2880-byte boundary (start of data)
            size_t blockEnd = ((offset / 2880) + 1) * 2880;
            m_info.dataOffset = blockEnd;
            break;
        }

        if (keyword == "BITPIX") {
            int bp = 0;
            try { bp = std::stoi(value); }
            catch (const std::exception&) { /* malformed FITS keyword — keep default */ }
            m_info.bitpix = static_cast<FITSBitpix>(bp);
        }
        else if (keyword == "NAXIS") {
            try { m_info.naxis = static_cast<uint32_t>(std::stoul(value)); }
            catch (const std::exception&) {}
        }
        else if (keyword == "NAXIS1") {
            try { m_info.naxis1 = static_cast<uint32_t>(std::stoul(value)); }
            catch (const std::exception&) {}
        }
        else if (keyword == "NAXIS2") {
            try { m_info.naxis2 = static_cast<uint32_t>(std::stoul(value)); }
            catch (const std::exception&) {}
        }
        else if (keyword == "NAXIS3") {
            try { m_info.naxis3 = static_cast<uint32_t>(std::stoul(value)); }
            catch (const std::exception&) {}
        }
        else if (keyword == "BZERO") {
            try { m_info.bzero = std::stod(value); }
            catch (const std::exception&) {}
        }
        else if (keyword == "BSCALE") {
            try { m_info.bscale = std::stod(value); }
            catch (const std::exception&) {}
        }
        else if (keyword == "DATAMIN") {
            try { m_info.datamin = std::stod(value); m_info.hasMinMax = true; }
            catch (const std::exception&) {}
        }
        else if (keyword == "DATAMAX") {
            try { m_info.datamax = std::stod(value); m_info.hasMinMax = true; }
            catch (const std::exception&) {}
        }
        else if (keyword == "OBJECT") {
            m_info.objectName = std::wstring(value.begin(), value.end());
        }
        else if (keyword == "INSTRUME") {
            m_info.instrument = std::wstring(value.begin(), value.end());
        }
        else if (keyword == "TELESCOP") {
            m_info.telescope = std::wstring(value.begin(), value.end());
        }

        offset += 80;
    }

    return (m_info.naxis >= 2 && m_info.naxis1 > 0 && m_info.naxis2 > 0);
}

//------------------------------------------------------------------------------
size_t FITSDecoder::GetBytesPerPixel(FITSBitpix bp) {
    switch (bp) {
    case FITSBitpix::UInt8: return 1;
    case FITSBitpix::Int16: return 2;
    case FITSBitpix::Int32: return 4;
    case FITSBitpix::Float32: return 4;
    case FITSBitpix::Float64: return 8;
    default: return 2;
    }
}

//------------------------------------------------------------------------------
double FITSDecoder::ReadPixelValue(const uint8_t* data, size_t offset) const {
    // FITS is big-endian
    double raw = 0.0;
    switch (m_info.bitpix) {
    case FITSBitpix::UInt8:
        raw = static_cast<double>(data[offset]);
        break;
    case FITSBitpix::Int16: {
        int16_t val = static_cast<int16_t>(
            (static_cast<uint16_t>(data[offset]) << 8) |
            static_cast<uint16_t>(data[offset + 1]));
        raw = static_cast<double>(val);
        break;
    }
    case FITSBitpix::Int32: {
        int32_t val = (static_cast<int32_t>(data[offset]) << 24) |
            (static_cast<int32_t>(data[offset + 1]) << 16) |
            (static_cast<int32_t>(data[offset + 2]) << 8) |
            static_cast<int32_t>(data[offset + 3]);
        raw = static_cast<double>(val);
        break;
    }
    case FITSBitpix::Float32: {
        uint32_t bits = (static_cast<uint32_t>(data[offset]) << 24) |
            (static_cast<uint32_t>(data[offset + 1]) << 16) |
            (static_cast<uint32_t>(data[offset + 2]) << 8) |
            static_cast<uint32_t>(data[offset + 3]);
        float f;
        std::memcpy(&f, &bits, sizeof(f));
        raw = static_cast<double>(f);
        break;
    }
    case FITSBitpix::Float64: {
        uint64_t bits = 0;
        for (int i = 0; i < 8; ++i) {
            bits = (bits << 8) | static_cast<uint64_t>(data[offset + i]);
        }
        double d;
        std::memcpy(&d, &bits, sizeof(d));
        raw = d;
        break;
    }
    }
    return m_info.bzero + m_info.bscale * raw;
}

//------------------------------------------------------------------------------
uint8_t FITSDecoder::ApplyStretch(double physicalValue, double vmin, double vmax,
    FITSStretch stretch) const {
    if (vmax <= vmin) return 128;
    double normalized = 0.0;
    switch (stretch) {
    case FITSStretch::Linear:
        normalized = (physicalValue - vmin) / (vmax - vmin);
        break;
    case FITSStretch::Logarithmic:
        normalized = std::log(1.0 + (physicalValue - vmin)) /
            std::log(1.0 + (vmax - vmin));
        break;
    case FITSStretch::SquareRoot:
        normalized = std::sqrt((std::max)(0.0, (physicalValue - vmin) / (vmax - vmin)));
        break;
    case FITSStretch::Asinh:
        normalized = std::asinh((physicalValue - vmin) / (vmax - vmin)) /
            std::asinh(1.0);
        break;
    }
    normalized = (std::max)(0.0, (std::min)(1.0, normalized));
    return static_cast<uint8_t>(normalized * 255.0);
}

//------------------------------------------------------------------------------
void FITSDecoder::ComputeMinMax(const uint8_t* data, size_t size) {
    if (m_info.dataOffset == 0 || m_info.naxis1 == 0 || m_info.naxis2 == 0)
        return;

    size_t bpp = GetBytesPerPixel(m_info.bitpix);
    double localMin = 1e30, localMax = -1e30;
    size_t stride = (std::max)(size_t(1), static_cast<size_t>(m_info.naxis1) * m_info.naxis2 / 10000);

    for (size_t i = 0; i < m_info.naxis1 * m_info.naxis2; i += stride) {
        size_t offset = m_info.dataOffset + i * bpp;
        if (offset + bpp > size) break;
        double val = ReadPixelValue(data, offset);
        if (val < localMin) localMin = val;
        if (val > localMax) localMax = val;
    }

    m_info.datamin = localMin;
    m_info.datamax = localMax;
    m_info.hasMinMax = true;
}

//------------------------------------------------------------------------------
std::vector<uint8_t> FITSDecoder::ExtractThumbnail(const uint8_t* data, size_t size,
    uint32_t thumbWidth, uint32_t thumbHeight) const {
    std::vector<uint8_t> thumbnail(thumbWidth * thumbHeight * 4, 0);

    if (m_info.dataOffset == 0 || m_info.naxis1 == 0 || m_info.naxis2 == 0)
        return thumbnail;

    double vmin = m_info.datamin;
    double vmax = m_info.datamax;
    size_t bpp = GetBytesPerPixel(m_info.bitpix);

    for (uint32_t y = 0; y < thumbHeight; ++y) {
        uint32_t srcY = y * m_info.naxis2 / thumbHeight;
        for (uint32_t x = 0; x < thumbWidth; ++x) {
            uint32_t srcX = x * m_info.naxis1 / thumbWidth;
            size_t dstIdx = (y * thumbWidth + x) * 4;
            size_t srcOffset = m_info.dataOffset + (srcY * m_info.naxis1 + srcX) * bpp;

            if (srcOffset + bpp <= size) {
                double val = ReadPixelValue(data, srcOffset);
                uint8_t gray = ApplyStretch(val, vmin, vmax, FITSStretch::SquareRoot);
                thumbnail[dstIdx + 0] = gray;
                thumbnail[dstIdx + 1] = gray;
                thumbnail[dstIdx + 2] = gray;
                thumbnail[dstIdx + 3] = 255;
            }
        }
    }
    return thumbnail;
}

//------------------------------------------------------------------------------
const wchar_t* const* FITSDecoder::GetExtensions() {
    return s_fitsExtensions;
}

uint32_t FITSDecoder::GetExtensionCount() {
    return static_cast<uint32_t>(std::size(s_fitsExtensions));
}

//------------------------------------------------------------------------------
const wchar_t* FITSDecoder::GetBitpixName(FITSBitpix bp) {
    switch (bp) {
    case FITSBitpix::UInt8: return L"8-bit unsigned";
    case FITSBitpix::Int16: return L"16-bit signed";
    case FITSBitpix::Int32: return L"32-bit signed";
    case FITSBitpix::Float32: return L"32-bit float";
    case FITSBitpix::Float64: return L"64-bit double";
    default: return L"Unknown";
    }
}

const wchar_t* FITSDecoder::GetStretchName(FITSStretch s) {
    switch (s) {
    case FITSStretch::Linear: return L"Linear";
    case FITSStretch::Logarithmic: return L"Logarithmic";
    case FITSStretch::SquareRoot: return L"Square Root";
    case FITSStretch::Asinh: return L"Asinh";
    default: return L"Unknown";
    }
}

}
} // namespace ExplorerLens::Engine
