//==============================================================================
// DarkThumbs Engine — Sprint 341: GPU Decode Acceleration V2
// Vendor-agnostic hardware decode routing for JPEG/HEVC/AV1/JPEG XL via
// D3D12 video decode, Intel Quick Sync V3, AMD VCE/VCN, and NVIDIA NVDEC
// with graceful CPU fallback and decode pipeline telemetry.
//==============================================================================
#pragma once
#include <string>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

enum class GPUDecodeVendor  : uint8_t { Intel=0, AMD, NVIDIA, ARM, CPU, COUNT };
enum class GPUDecodeAPI     : uint8_t { D3D12Video=0, QuickSyncV3, VCNVCE, NVDEC, MediaFoundation, COUNT };
enum class GPUDecodeCodec   : uint8_t { MJPEG=0, HEVC, AV1, JPEGXL, VP9, COUNT };

struct GPUDecodeCapability {
    GPUDecodeVendor vendor      = GPUDecodeVendor::CPU;
    GPUDecodeAPI    api         = GPUDecodeAPI::MediaFoundation;
    GPUDecodeCodec  codec       = GPUDecodeCodec::MJPEG;
    bool            hwAvailable = false;
    float           expectedSpeedup = 1.0f; // vs CPU baseline
};

struct GPUDecodeSessionStats {
    uint64_t framesDecoded      = 0;
    uint64_t framesFromGPU      = 0;
    uint64_t framesFromCPU      = 0;
    float    gpuUtilizationPct  = 0.0f;
    float    avgDecodeMs        = 0.0f;
};

class GPUDecodeAccelerationV2 {
public:
    static const wchar_t* VendorName(GPUDecodeVendor v) {
        switch(v) {
            case GPUDecodeVendor::Intel:  return L"Intel";
            case GPUDecodeVendor::AMD:    return L"AMD";
            case GPUDecodeVendor::NVIDIA: return L"NVIDIA";
            case GPUDecodeVendor::ARM:    return L"ARM";
            case GPUDecodeVendor::CPU:    return L"CPU Fallback";
            default: return L"Unknown";
        }
    }
    static const wchar_t* APIName(GPUDecodeAPI a) {
        switch(a) {
            case GPUDecodeAPI::D3D12Video:      return L"D3D12 Video";
            case GPUDecodeAPI::QuickSyncV3:     return L"Intel Quick Sync V3";
            case GPUDecodeAPI::VCNVCE:          return L"AMD VCN/VCE";
            case GPUDecodeAPI::NVDEC:           return L"NVIDIA NVDEC";
            case GPUDecodeAPI::MediaFoundation: return L"Media Foundation";
            default: return L"Unknown";
        }
    }
    static const wchar_t* CodecName(GPUDecodeCodec c) {
        switch(c) {
            case GPUDecodeCodec::MJPEG:  return L"MJPEG";
            case GPUDecodeCodec::HEVC:   return L"HEVC/H.265";
            case GPUDecodeCodec::AV1:    return L"AV1";
            case GPUDecodeCodec::JPEGXL: return L"JPEG XL";
            case GPUDecodeCodec::VP9:    return L"VP9";
            default: return L"Unknown";
        }
    }
    static constexpr size_t VendorCount() { return static_cast<size_t>(GPUDecodeVendor::COUNT); }
    static constexpr size_t APICount()    { return static_cast<size_t>(GPUDecodeAPI::COUNT); }
    static constexpr size_t CodecCount()  { return static_cast<size_t>(GPUDecodeCodec::COUNT); }
};

}} // namespace DarkThumbs::Engine
