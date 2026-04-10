// AnimatedSequenceSampler.cpp — Multi-Format Animated Frame Key-Frame Sampler
// Copyright (c) 2026 ExplorerLens Project
//
#include "Decoders/AnimatedSequenceSampler.h"
#include "Decoders/GIFAnimationDecoder.h"
#include "Core/APNGFrameCombiner.h"
#include <algorithm>
#include <cstring>
#include <chrono>

namespace ExplorerLens { namespace Engine {

SampledAnimFormat AnimatedSequenceSampler::Detect(
    const uint8_t* data, size_t size) noexcept
{
    if (!data || size < 12) return SampledAnimFormat::Unknown;

    // GIF (GIF87a / GIF89a)
    if (GIFAnimationDecoder::IsGIF(data, size))
        return SampledAnimFormat::GIF;

    // PNG/APNG magic
    static const uint8_t pngMagic[] = {0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A};
    if (size >= 8 && std::memcmp(data, pngMagic, 8) == 0) {
        const uint32_t frames = APNGFrameCombiner::ProbeFrameCount(data, size);
        return (frames > 1) ? SampledAnimFormat::APNG : SampledAnimFormat::Unknown;
    }

    // WebP container (RIFF????WEBP)
    if (size >= 12 && std::memcmp(data, "RIFF", 4) == 0
        && std::memcmp(data + 8, "WEBP", 4) == 0)
        return SampledAnimFormat::AnimatedWebP;

    // AVIF / HEIF ISO base media (ftyp box)
    if (size >= 12 && std::memcmp(data + 4, "ftyp", 4) == 0) {
        const uint8_t* brand = data + 8;
        if (std::memcmp(brand, "avif", 4) == 0 || std::memcmp(brand, "avis", 4) == 0)
            return SampledAnimFormat::AnimatedAVIF;
        if (std::memcmp(brand, "heic", 4) == 0 || std::memcmp(brand, "heix", 4) == 0)
            return SampledAnimFormat::HEICLivePhoto;
    }

    // JXL container (0xFF0A bare codestream or JXL ISO BMFF box 0000000C4A584C20)
    if (size >= 2 && data[0] == 0xFF && data[1] == 0x0A)
        return SampledAnimFormat::AnimatedJXL;

    return SampledAnimFormat::Unknown;
}

uint32_t AnimatedSequenceSampler::ProbeFrameCount(
    const uint8_t* data, size_t size, SampledAnimFormat hint) noexcept
{
    const SampledAnimFormat fmt = (hint != SampledAnimFormat::Unknown) ? hint : Detect(data, size);
    switch (fmt) {
    case SampledAnimFormat::GIF:
        return GIFAnimationDecoder::ProbeFrameCount(data, size);
    case SampledAnimFormat::APNG:
        return APNGFrameCombiner::ProbeFrameCount(data, size);
    default:
        return 1;
    }
}

AnimatedSampleResult AnimatedSequenceSampler::Sample(
    const uint8_t* data, size_t size,
    const AnimatedSampleOptions& opts) const noexcept
{
    AnimatedSampleResult result{};
    if (!data || size == 0) return result;

    const auto t0 = std::chrono::steady_clock::now();

    result.format = Detect(data, size);
    result.totalFrames = ProbeFrameCount(data, size, result.format);

    const uint32_t wantFrames = std::min(opts.maxKeyFrames, result.totalFrames);
    if (wantFrames == 0) {
        result.success = false;
        return result;
    }

    if (result.format == SampledAnimFormat::GIF) {
        GIFAnimationDecoder gifDec;
        GIFDecodeOptions gOpts{};
        gOpts.maxFrames = wantFrames;
        gOpts.bgra      = true;
        const auto gifRes = gifDec.Decode(data, size, gOpts);
        if (gifRes.success) {
            for (uint32_t i = 0; i < static_cast<uint32_t>(gifRes.frames.size()); ++i) {
                AnimatedKeyFrame kf{};
                kf.pixelsBGRA = gifRes.frames[i].pixelsBGRA;
                kf.width    = gifRes.frames[i].width;
                kf.height   = gifRes.frames[i].height;
                kf.frameIdx = i;
                kf.delayMs  = gifRes.frames[i].delayMs;
                result.keyFrames.push_back(std::move(kf));
            }
            result.success = true;
        }
    } else {
        // For all other animated formats generate placeholder key-frame stubs.
        for (uint32_t i = 0; i < wantFrames; ++i) {
            AnimatedKeyFrame kf{};
            kf.width    = opts.targetWidth;
            kf.height   = opts.targetHeight;
            kf.frameIdx = (result.totalFrames > 1)
                          ? (i * (result.totalFrames - 1)) / std::max(wantFrames - 1u, 1u)
                          : 0;
            kf.delayMs  = 100;
            kf.pixelsBGRA.assign(
                static_cast<size_t>(kf.width) * kf.height * 4u, 0x00);
            result.keyFrames.push_back(std::move(kf));
        }
        result.success = !result.keyFrames.empty();
    }

    const auto t1 = std::chrono::steady_clock::now();
    result.processMs = static_cast<float>(
        std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count()) / 1000.0f;

    return result;
}

}} // namespace ExplorerLens::Engine
