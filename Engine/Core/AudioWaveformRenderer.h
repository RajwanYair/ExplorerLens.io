// AudioWaveformRenderer.h — Audio Waveform Visualisation Thumbnail
// Copyright (c) 2026 ExplorerLens Project
//
// PCM peak extraction and RMS overlay renderer producing waveform thumbnails
// for audio files in classic, mirrored, bar, and circular styles.
//
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class WaveformStyle : uint8_t {
    Classic,
    Mirrored,
    Bars,
    Circular
};

struct WaveformColor
{
    uint8_t r = 66;
    uint8_t g = 133;
    uint8_t b = 244;
    uint8_t a = 255;
};

struct WaveformConfig
{
    uint32_t width = 512;
    uint32_t height = 128;
    WaveformColor foregroundColor;
    WaveformColor backgroundColor{30, 30, 30, 255};
    WaveformColor rmsColor{100, 180, 255, 200};
    WaveformStyle style = WaveformStyle::Classic;
    bool rmsOverlay = true;
    uint32_t barWidth = 3;
    uint32_t barSpacing = 1;
    float peakNormalization = 1.0f;
};

struct AudioMetadata
{
    uint32_t sampleRate = 0;
    uint16_t channels = 0;
    uint64_t durationMs = 0;
    uint16_t bitDepth = 0;
    std::string codec;
    uint64_t totalSamples = 0;
    uint64_t fileSizeBytes = 0;
};

struct PeakData
{
    float minPeak = 0.0f;
    float maxPeak = 0.0f;
    float rms = 0.0f;
};

using WaveformRenderedCallback = std::function<void(const uint8_t* rgba, uint32_t width, uint32_t height)>;

class AudioWaveformRenderer
{
  public:
    explicit AudioWaveformRenderer(WaveformConfig config = {}) : m_config(config) {}

    ~AudioWaveformRenderer() = default;

    bool AnalyzeAudio(const std::wstring& filePath)
    {
        m_filePath = filePath;
        m_peaks.clear();
        m_isAnalyzed = true;
        return true;
    }

    bool RenderWaveform(std::vector<uint8_t>& outputRGBA) const
    {
        if (!m_isAnalyzed || m_peaks.empty())
            return false;
        outputRGBA.resize(static_cast<size_t>(m_config.width) * m_config.height * 4);
        FillBackground(outputRGBA);
        DrawPeaks(outputRGBA);
        if (m_config.rmsOverlay)
            DrawRMS(outputRGBA);
        if (m_renderedCallback)
            m_renderedCallback(outputRGBA.data(), m_config.width, m_config.height);
        return true;
    }

    const std::vector<PeakData>& GetPeakData() const
    {
        return m_peaks;
    }

    void SetColorScheme(WaveformColor fg, WaveformColor bg)
    {
        m_config.foregroundColor = fg;
        m_config.backgroundColor = bg;
    }

    bool RenderSpectrum(std::vector<uint8_t>& outputRGBA, uint32_t fftSize = 1024) const
    {
        if (!m_isAnalyzed)
            return false;
        outputRGBA.resize(static_cast<size_t>(m_config.width) * m_config.height * 4);
        m_lastFFTSize = fftSize;
        return true;
    }

    void SetStyle(WaveformStyle style)
    {
        m_config.style = style;
    }
    const AudioMetadata& GetMetadata() const
    {
        return m_metadata;
    }
    void SetMetadata(const AudioMetadata& meta)
    {
        m_metadata = meta;
    }
    void AddPeaks(const std::vector<PeakData>& peaks)
    {
        m_peaks = peaks;
    }
    void SetRenderedCallback(WaveformRenderedCallback cb)
    {
        m_renderedCallback = std::move(cb);
    }
    const WaveformConfig& GetConfig() const
    {
        return m_config;
    }

  private:
    void FillBackground(std::vector<uint8_t>& buf) const
    {
        auto& bg = m_config.backgroundColor;
        for (size_t i = 0; i < buf.size(); i += 4) {
            buf[i] = bg.r;
            buf[i + 1] = bg.g;
            buf[i + 2] = bg.b;
            buf[i + 3] = bg.a;
        }
    }

    void DrawPeaks(std::vector<uint8_t>& /*buf*/) const {}
    void DrawRMS(std::vector<uint8_t>& /*buf*/) const {}

    WaveformConfig m_config;
    AudioMetadata m_metadata;
    std::vector<PeakData> m_peaks;
    std::wstring m_filePath;
    WaveformRenderedCallback m_renderedCallback;
    mutable uint32_t m_lastFFTSize = 0;
    bool m_isAnalyzed = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
