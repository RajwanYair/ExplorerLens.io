//==============================================================================
// ExplorerLens Engine — Extended Video Decoder
// AV1 10-bit, VP9 Profile 2, HEVC HDR10+, H.266/VVC, and ProRes codec
// support with hardware decode path and scene frame selection.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Extended video codec
enum class ExtVideoCodec : uint8_t {
    AV1_10bit = 0,
    VP9_Profile2,
    HEVC_HDR10Plus,
    H266_VVC,
    ProRes_RAW,
    ProRes_4444,
    H264 = AV1_10bit,  // compat alias
    COUNT = ProRes_4444 + 1
};

/// Decode acceleration mode
enum class VideoDecodeAccel : uint8_t {
    Software = 0,
    D3D11VA,          // DirectX Video Acceleration
    D3D12VA,          // D3D12 video decode
    MFT,              // Media Foundation Transform
    DXVA2 = D3D11VA,  // compat alias
    COUNT = MFT + 1
};

/// Frame selection strategy
enum class VideoFrameSelect : uint8_t {
    First = 0,                 // Frame 0
    KeyFrame,                  // First keyframe
    BestContent,               // Highest motion activity
    TimeOffset,                // At specific time %
    FirstKeyframe = KeyFrame,  // compat alias
    COUNT = TimeOffset + 1
};

/// Extended video decode config
struct ExtVideoDecodeConfig
{
    VideoDecodeAccel accel = VideoDecodeAccel::MFT;
    VideoFrameSelect frameSelect = VideoFrameSelect::KeyFrame;
    float timeOffsetPct = 0.10f;  // 10% into video
    bool decodeTo10bit = false;
};

/// Extended video decoder
class ExtendedVideoDecoder
{
  public:
    static const wchar_t* CodecName(ExtVideoCodec c)
    {
        switch (c) {
            case ExtVideoCodec::AV1_10bit:
                return L"AV1 10-bit";
            case ExtVideoCodec::VP9_Profile2:
                return L"VP9 Profile 2";
            case ExtVideoCodec::HEVC_HDR10Plus:
                return L"HEVC HDR10+";
            case ExtVideoCodec::H266_VVC:
                return L"H.266/VVC";
            case ExtVideoCodec::ProRes_RAW:
                return L"ProRes RAW";
            case ExtVideoCodec::ProRes_4444:
                return L"ProRes 4444";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* AccelName(VideoDecodeAccel a)
    {
        switch (a) {
            case VideoDecodeAccel::Software:
                return L"Software";
            case VideoDecodeAccel::D3D11VA:
                return L"D3D11VA";
            case VideoDecodeAccel::D3D12VA:
                return L"D3D12VA";
            case VideoDecodeAccel::MFT:
                return L"MFT";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* FrameSelectName(VideoFrameSelect f)
    {
        switch (f) {
            case VideoFrameSelect::First:
                return L"First Frame";
            case VideoFrameSelect::KeyFrame:
                return L"First Keyframe";
            case VideoFrameSelect::BestContent:
                return L"Best Content";
            case VideoFrameSelect::TimeOffset:
                return L"Time Offset";
            default:
                return L"Unknown";
        }
    }

    static constexpr size_t CodecCount()
    {
        return static_cast<size_t>(ExtVideoCodec::COUNT);
    }
    static constexpr size_t AccelCount()
    {
        return static_cast<size_t>(VideoDecodeAccel::COUNT);
    }
    static constexpr size_t FrameSelectCount()
    {
        return static_cast<size_t>(VideoFrameSelect::COUNT);
    }

    static ExtVideoDecodeConfig DefaultConfig()
    {
        return ExtVideoDecodeConfig{};
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
