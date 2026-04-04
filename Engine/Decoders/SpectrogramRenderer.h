// SpectrogramRenderer.h — Audio Spectrogram Thumbnail Generator
// Copyright (c) 2026 ExplorerLens Project
//
// Computes Discrete Fourier Transform (DFT) on audio frames to generate
// frequency-domain spectrogram images. Uses Hann window and log-frequency
// mapping for visually informative audio thumbnails.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct SpectrogramConfig
{
    uint32_t fftSize = 1024;
    uint32_t hopSize = 512;
    uint32_t numBins = 128;  // Output frequency bins
    float minFreqHz = 20.0f;
    float maxFreqHz = 20000.0f;
    float dynamicRangeDB = 80.0f;
};

struct SpectrogramFrame
{
    std::vector<float> magnitudes;  // [0..1] per bin
};

struct SpectrogramStats
{
    uint32_t framesComputed = 0;
    uint32_t fftSize = 0;
    uint32_t totalFrames = 0;
};

class SpectrogramRenderer
{
  public:
    SpectrogramRenderer() = default;
    ~SpectrogramRenderer() = default;

    static const wchar_t* GetName()
    {
        return L"SpectrogramRenderer";
    }

    /// Apply Hann window to a frame of samples.
    void ApplyHannWindow(std::vector<float>& frame) const
    {
        const size_t N = frame.size();
        constexpr double PI = 3.14159265358979323846;
        for (size_t i = 0; i < N; ++i) {
            double w = 0.5 * (1.0 - std::cos(2.0 * PI * i / (N - 1)));
            frame[i] *= static_cast<float>(w);
        }
    }

    /// Compute magnitude spectrum using DFT (O(N^2) — suitable for small FFT sizes).
    std::vector<float> ComputeDFT(const std::vector<float>& frame) const
    {
        const size_t N = frame.size();
        const size_t halfN = N / 2 + 1;
        std::vector<float> mags(halfN);
        constexpr double PI = 3.14159265358979323846;

        for (size_t k = 0; k < halfN; ++k) {
            double re = 0.0, im = 0.0;
            for (size_t n = 0; n < N; ++n) {
                double angle = 2.0 * PI * k * n / N;
                re += frame[n] * std::cos(angle);
                im -= frame[n] * std::sin(angle);
            }
            mags[k] = static_cast<float>(std::sqrt(re * re + im * im) / N);
        }
        return mags;
    }

    /// Convert magnitude to dB scale, clamped to dynamic range.
    float MagnitudeToDB(float mag, float dynamicRangeDB) const
    {
        if (mag <= 0.0f)
            return 0.0f;
        float db = 20.0f * std::log10(mag);
        float normalized = 1.0f + db / dynamicRangeDB;
        return std::clamp(normalized, 0.0f, 1.0f);
    }

    /// Compute full spectrogram from sample buffer.
    std::vector<SpectrogramFrame> Compute(const float* samples, uint32_t count, const SpectrogramConfig& config) const
    {
        std::vector<SpectrogramFrame> result;
        if (!samples || count < config.fftSize)
            return result;

        for (uint32_t offset = 0; offset + config.fftSize <= count; offset += config.hopSize) {
            std::vector<float> frame(samples + offset, samples + offset + config.fftSize);
            ApplyHannWindow(frame);
            auto mags = ComputeDFT(frame);

            SpectrogramFrame sf;
            sf.magnitudes.resize(mags.size());
            for (size_t i = 0; i < mags.size(); ++i)
                sf.magnitudes[i] = MagnitudeToDB(mags[i], config.dynamicRangeDB);

            result.push_back(std::move(sf));
        }
        return result;
    }

    SpectrogramStats GetStats() const
    {
        return m_stats;
    }

  private:
    mutable SpectrogramStats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
