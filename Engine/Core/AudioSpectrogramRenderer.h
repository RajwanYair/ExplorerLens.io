// AudioSpectrogramRenderer.h — Audio Waveform/Spectrogram Visualization
// Copyright (c) 2026 ExplorerLens Project
//
// Generates procedural waveform and spectrogram visualizations for audio file
// thumbnails. Uses sine/cosine synthesis to simulate frequency bins without
// requiring actual audio decoding libraries.
//
#pragma once

#include <windows.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// ARGB color packed as 0xAARRGGBB.
using SpecColor = uint32_t;

/// A single frequency bin magnitude (0.0 – 1.0).
struct FrequencyBin
{
    float frequency = 0.0f;  // Hz (approximate centre)
    float magnitude = 0.0f;  // normalised 0..1
};

class AudioSpectrogramRenderer
{
  public:
    AudioSpectrogramRenderer() = default;

    // ── Sample generation ────────────────────────────────────────────

    /// Synthesise a simple test waveform: sum of sine waves at the given
    /// frequencies.  Returns `sampleCount` float samples in [-1, 1].
    std::vector<float> GenerateTestSamples(const std::vector<float>& frequenciesHz, uint32_t sampleRate,
                                           uint32_t sampleCount) const
    {
        std::vector<float> samples(sampleCount, 0.0f);
        if (frequenciesHz.empty() || sampleRate == 0)
            return samples;

        const float invN = 1.0f / static_cast<float>(frequenciesHz.size());
        const float twoPi = 6.283185307f;

        for (uint32_t i = 0; i < sampleCount; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(sampleRate);
            float sum = 0.0f;
            for (float f : frequenciesHz)
                sum += std::sin(twoPi * f * t);
            samples[i] = sum * invN;  // normalise by component count
        }
        return samples;
    }

    // ── Waveform rendering ───────────────────────────────────────────

    /// Render a mono waveform into an ARGB pixel buffer of size width * height.
    /// Returns the buffer (row-major, top-to-bottom).
    std::vector<SpecColor> RenderWaveform(const std::vector<float>& samples, uint32_t width, uint32_t height) const
    {
        std::vector<SpecColor> pixels(static_cast<size_t>(width) * height, m_backgroundColor);
        if (samples.empty() || width == 0 || height == 0)
            return pixels;

        const float halfH = static_cast<float>(height) / 2.0f;
        const float step = static_cast<float>(samples.size()) / static_cast<float>(width);

        for (uint32_t x = 0; x < width; ++x) {
            size_t idx = static_cast<size_t>(static_cast<float>(x) * step);
            idx = (std::min)(idx, samples.size() - 1);

            float val = samples[idx];
            val = (std::max)(-1.0f, (std::min)(1.0f, val));

            int y = static_cast<int>(halfH - val * halfH);
            y = (std::max)(0, (std::min)(static_cast<int>(height) - 1, y));

            // Draw a vertical line from centre to sample point
            int yMid = static_cast<int>(halfH);
            int yLo = (std::min)(y, yMid);
            int yHi = (std::max)(y, yMid);
            for (int row = yLo; row <= yHi; ++row)
                pixels[static_cast<size_t>(row) * width + x] = m_waveformColor;
        }
        return pixels;
    }

    // ── Spectrogram helpers ──────────────────────────────────────────

    /// Compute a simple DFT magnitude for `binCount` frequency bins over a
    /// window of samples.  This is O(N*K) and intended for thumbnail-sized
    /// output only.
    std::vector<FrequencyBin> ComputeFrequencyBins(const std::vector<float>& samples, uint32_t sampleRate,
                                                   uint32_t binCount) const
    {
        std::vector<FrequencyBin> bins(binCount);
        if (samples.empty() || sampleRate == 0 || binCount == 0)
            return bins;

        const float twoPi = 6.283185307f;
        const float N = static_cast<float>(samples.size());

        float maxMag = 0.0f;
        for (uint32_t k = 0; k < binCount; ++k) {
            float freq =
                static_cast<float>(k + 1) * static_cast<float>(sampleRate) / (2.0f * static_cast<float>(binCount));
            float re = 0.0f, im = 0.0f;
            for (size_t n = 0; n < samples.size(); ++n) {
                float angle = twoPi * static_cast<float>(k) * static_cast<float>(n) / N;
                re += samples[n] * std::cos(angle);
                im -= samples[n] * std::sin(angle);
            }
            float mag = std::sqrt(re * re + im * im) / N;
            bins[k].frequency = freq;
            bins[k].magnitude = mag;
            maxMag = (std::max)(maxMag, mag);
        }
        // Normalise
        if (maxMag > 0.0f) {
            for (auto& b : bins)
                b.magnitude /= maxMag;
        }
        return bins;
    }

    /// Map a normalised magnitude [0, 1] to a heat-map ARGB colour.
    SpecColor GetSpectrogramColor(float magnitude) const
    {
        magnitude = (std::max)(0.0f, (std::min)(1.0f, magnitude));
        // Blue → Cyan → Green → Yellow → Red
        uint8_t r = 0, g = 0, b = 0;
        if (magnitude < 0.25f) {
            float t = magnitude / 0.25f;
            b = 255;
            g = static_cast<uint8_t>(t * 255.0f);
        } else if (magnitude < 0.5f) {
            float t = (magnitude - 0.25f) / 0.25f;
            g = 255;
            b = static_cast<uint8_t>((1.0f - t) * 255.0f);
        } else if (magnitude < 0.75f) {
            float t = (magnitude - 0.5f) / 0.25f;
            g = 255;
            r = static_cast<uint8_t>(t * 255.0f);
        } else {
            float t = (magnitude - 0.75f) / 0.25f;
            r = 255;
            g = static_cast<uint8_t>((1.0f - t) * 255.0f);
        }
        return (0xFFu << 24) | (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8)
               | static_cast<uint32_t>(b);
    }

    /// Return all heat-map colours for 256 quantised magnitude levels.
    std::vector<SpecColor> GetSpectrogramColors() const
    {
        std::vector<SpecColor> lut(256);
        for (int i = 0; i < 256; ++i)
            lut[i] = GetSpectrogramColor(static_cast<float>(i) / 255.0f);
        return lut;
    }

    // ── Analysis helpers ─────────────────────────────────────────────

    /// Peak absolute amplitude in the sample buffer.
    float GetPeakAmplitude(const std::vector<float>& samples) const
    {
        float peak = 0.0f;
        for (float s : samples)
            peak = (std::max)(peak, std::abs(s));
        return peak;
    }

    /// RMS level (0..1 for normalised input).
    float GetRmsLevel(const std::vector<float>& samples) const
    {
        if (samples.empty())
            return 0.0f;
        double sumSq = 0.0;
        for (float s : samples)
            sumSq += static_cast<double>(s) * s;
        return static_cast<float>(std::sqrt(sumSq / static_cast<double>(samples.size())));
    }

    // ── Configuration ────────────────────────────────────────────────

    void SetBackgroundColor(SpecColor c)
    {
        m_backgroundColor = c;
    }
    void SetWaveformColor(SpecColor c)
    {
        m_waveformColor = c;
    }

  private:
    SpecColor m_backgroundColor = 0xFF1A1A2E;  // dark navy
    SpecColor m_waveformColor = 0xFF00D4FF;    // cyan
};

}  // namespace Engine
}  // namespace ExplorerLens
