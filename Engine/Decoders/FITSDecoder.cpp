// FITSDecoder.cpp — FITS Astronomy Image Decoder (V1 implementation)
// Copyright (c) 2026 ExplorerLens Project
//
// This translation unit provides implementations for the FITSDecoder V1 class
// declared in FITSDecoderV2.h (the canonical consolidated header).
//
#include "FITSDecoderV2.h"

namespace ExplorerLens::Engine {

FITSDecoder::FITSDecoder() = default;

bool FITSDecoder::IsFITSFile(const uint8_t* data, size_t size) {
    if (!data || size < 6) return false;
    return data[0]=='S' && data[1]=='I' && data[2]=='M' &&
           data[3]=='P' && data[4]=='L' && data[5]=='E';
}

bool FITSDecoder::ParseHeader(const uint8_t*, size_t) { return false; }

uint8_t FITSDecoder::ApplyStretch(double val, double vmin, double vmax, FITSStretch) const {
    if (vmax <= vmin) return 0;
    double norm = (val - vmin) / (vmax - vmin);
    if (norm < 0.0) norm = 0.0;
    if (norm > 1.0) norm = 1.0;
    return static_cast<uint8_t>(norm * 255.0 + 0.5);
}

void FITSDecoder::ComputeMinMax(const uint8_t*, size_t) {}

std::vector<uint8_t> FITSDecoder::ExtractThumbnail(const uint8_t*, size_t, uint32_t, uint32_t) const { return {}; }

const wchar_t* const* FITSDecoder::GetExtensions() {
    static const wchar_t* kExt[] = { L".fits", L".fit", L".fts", nullptr };
    return kExt;
}

uint32_t FITSDecoder::GetExtensionCount() { return 3; }

const wchar_t* FITSDecoder::GetBitpixName(FITSBitpix bp) {
    switch (bp) {
        case FITSBitpix::UInt8:   return L"8-bit unsigned";
        case FITSBitpix::Int16:   return L"16-bit integer";
        case FITSBitpix::Int32:   return L"32-bit integer";
        case FITSBitpix::Float32: return L"32-bit float";
        case FITSBitpix::Float64: return L"64-bit float";
        default:                  return L"unknown";
    }
}

const wchar_t* FITSDecoder::GetStretchName(FITSStretch s) {
    switch (s) {
        case FITSStretch::Linear:      return L"Linear";
        case FITSStretch::Logarithmic: return L"Log";
        case FITSStretch::SquareRoot:  return L"Sqrt";
        case FITSStretch::Asinh:       return L"Asinh";
        default:                        return L"Unknown";
    }
}

size_t FITSDecoder::GetBytesPerPixel(FITSBitpix bp) {
    switch (bp) {
        case FITSBitpix::UInt8:   return 1;
        case FITSBitpix::Int16:   return 2;
        case FITSBitpix::Int32:   return 4;
        case FITSBitpix::Float32: return 4;
        case FITSBitpix::Float64: return 8;
        default:                  return 0;
    }
}

std::string FITSDecoder::ParseKeyword(const char*, std::string&) const { return {}; }
double      FITSDecoder::ReadPixelValue(const uint8_t*, size_t) const  { return 0.0; }

} // namespace ExplorerLens::Engine
