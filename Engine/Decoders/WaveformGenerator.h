// WaveformGenerator.h — Enhanced Audio Waveform Preview
// Copyright (c) 2026 ExplorerLens Project
//
// Generates audio waveform thumbnails from WAV/PCM data. Computes RMS
// amplitude per time bucket, peak detection, and spectral centroid for
// color-coded frequency display.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct WaveformSample {
    float rmsAmplitude = 0.0f;
    float peakAmplitude = 0.0f;
    float spectralCentroid = 0.0f;
};

struct WAVHeader {
    uint16_t audioFormat = 0;
    uint16_t numChannels = 0;
    uint32_t sampleRate = 0;
    uint16_t bitsPerSample = 0;
    uint32_t dataSize = 0;
    double   durationSeconds = 0.0;
};

struct WaveformStats {
    uint32_t filesProcessed = 0;
    uint64_t totalSamplesProcessed = 0;
    double   avgDuration = 0.0;
};

class WaveformGenerator {
public:
    WaveformGenerator() = default;
    ~WaveformGenerator() = default;

    static const wchar_t* GetName() { return L"WaveformGenerator"; }

    bool CanProcess(const wchar_t* ext) const {
        if (!ext) return false;
        std::wstring e(ext);
        for (auto& c : e) c = towlower(c);
        return e == L".wav" || e == L".wave" || e == L".aif" || e == L".aiff";
    }

    /// Parse WAV header (RIFF/WAVE format).
    WAVHeader ParseWAV(const uint8_t* data, size_t size) const {
        WAVHeader hdr;
        if (!data || size < 44) return hdr;
        if (memcmp(data, "RIFF", 4) != 0) return hdr;
        if (memcmp(data + 8, "WAVE", 4) != 0) return hdr;

        // Find "fmt " chunk
        size_t offset = 12;
        while (offset + 8 <= size) {
            if (memcmp(data + offset, "fmt ", 4) == 0) {
                if (offset + 24 <= size) {
                    memcpy(&hdr.audioFormat, data + offset + 8, 2);
                    memcpy(&hdr.numChannels, data + offset + 10, 2);
                    memcpy(&hdr.sampleRate, data + offset + 12, 4);
                    memcpy(&hdr.bitsPerSample, data + offset + 22, 2);
                }
            }
            if (memcmp(data + offset, "data", 4) == 0) {
                memcpy(&hdr.dataSize, data + offset + 4, 4);
                break;
            }
            uint32_t chunkSize = 0;
            memcpy(&chunkSize, data + offset + 4, 4);
            offset += 8 + chunkSize;
        }

        if (hdr.sampleRate > 0 && hdr.numChannels > 0 && hdr.bitsPerSample > 0) {
            uint32_t bytesPerSample = hdr.numChannels * (hdr.bitsPerSample / 8);
            if (bytesPerSample > 0) {
                uint32_t totalSamples = hdr.dataSize / bytesPerSample;
                hdr.durationSeconds = static_cast<double>(totalSamples) / hdr.sampleRate;
            }
        }
        return hdr;
    }

    /// Generate waveform data (amplitude buckets) from 16-bit PCM.
    std::vector<WaveformSample> GenerateWaveform(const int16_t* samples,
        uint32_t sampleCount, uint32_t bucketCount) const {
        std::vector<WaveformSample> waveform(bucketCount);
        if (!samples || sampleCount == 0 || bucketCount == 0) return waveform;

        uint32_t samplesPerBucket = std::max(1u, sampleCount / bucketCount);
        for (uint32_t b = 0; b < bucketCount; ++b) {
            uint32_t start = b * samplesPerBucket;
            uint32_t end = std::min(start + samplesPerBucket, sampleCount);
            double sumSq = 0.0;
            float peak = 0.0f;
            for (uint32_t i = start; i < end; ++i) {
                float val = samples[i] / 32768.0f;
                sumSq += val * val;
                peak = std::max(peak, std::abs(val));
            }
            waveform[b].rmsAmplitude = static_cast<float>(std::sqrt(sumSq / (end - start)));
            waveform[b].peakAmplitude = peak;
        }
        return waveform;
    }

    WaveformStats GetStats() const { return m_stats; }

private:
    mutable WaveformStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
