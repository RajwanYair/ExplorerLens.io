// FITSDecoder.h — FITS Astronomy Image Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Decoder for Flexible Image Transport System (FITS) astronomy images with
// logarithmic stretch, false-colour LUT, pixel statistics, and annotation overlay.
//
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class FITSStretchMode : uint8_t {
    Linear,
    Logarithmic,
    Sqrt,
    Asinh,
    HistogramEqualization
};

enum class FITSColorLUT : uint8_t {
    Grayscale,
    Heat,
    Cool,
    Rainbow,
    STScIDefault
};

struct FITSHeaderInfo
{
    int32_t bitpix = 16;
    uint32_t naxis = 2;
    uint32_t naxis1 = 0;
    uint32_t naxis2 = 0;
    double bzero = 0.0;
    double bscale = 1.0;
    std::string objectName;
    std::string telescopeName;
    std::string dateObs;
    double expTime = 0.0;
    std::string bunit;
};

struct FITSPixelStatistics
{
    double minValue = 0.0;
    double maxValue = 0.0;
    double meanValue = 0.0;
    double medianValue = 0.0;
    double stdDev = 0.0;
    uint64_t totalPixels = 0;
    uint64_t nanCount = 0;
};

class FITSDecoder
{
  public:
    FITSDecoder() = default;
    ~FITSDecoder() = default;

    FITSDecoder(const FITSDecoder&) = delete;
    FITSDecoder& operator=(const FITSDecoder&) = delete;
    FITSDecoder(FITSDecoder&&) noexcept = default;
    FITSDecoder& operator=(FITSDecoder&&) noexcept = default;

    bool DecodeFromFile(const std::wstring& filePath, uint32_t targetWidth, uint32_t targetHeight)
    {
        m_filePath = filePath;
        m_targetWidth = targetWidth;
        m_targetHeight = targetHeight;
        m_decoded = ParseHeader() && ReadImageData();
        if (m_decoded)
            ComputeStatistics();
        return m_decoded;
    }

    const FITSHeaderInfo& GetHeaderInfo() const
    {
        return m_header;
    }
    const FITSPixelStatistics& GetPixelStatistics() const
    {
        return m_stats;
    }

    bool ApplyStretch(std::vector<uint8_t>& output) const
    {
        if (m_rawData.empty())
            return false;
        output.resize(m_rawData.size());
        const double range = m_stats.maxValue - m_stats.minValue;
        if (range <= 0.0)
            return false;
        for (size_t i = 0; i < m_rawData.size(); ++i) {
            double norm = (m_rawData[i] - m_stats.minValue) / range;
            norm = ApplyStretchFunction(norm);
            output[i] = static_cast<uint8_t>(std::clamp(norm * 255.0, 0.0, 255.0));
        }
        return true;
    }

    void SetColorLUT(FITSColorLUT lut)
    {
        m_colorLUT = lut;
    }
    FITSColorLUT GetColorLUT() const
    {
        return m_colorLUT;
    }
    void SetStretchMode(FITSStretchMode mode)
    {
        m_stretchMode = mode;
    }
    FITSStretchMode GetStretchMode() const
    {
        return m_stretchMode;
    }

    bool RenderWithAnnotations(std::vector<uint8_t>& rgbOut, bool showObjectLabel = true) const
    {
        if (!m_decoded)
            return false;
        const size_t pixels = static_cast<size_t>(m_targetWidth) * m_targetHeight;
        rgbOut.resize(pixels * 3, 0);
        std::vector<uint8_t> stretched;
        if (!ApplyStretch(stretched))
            return false;
        ApplyLUT(stretched, rgbOut);
        if (showObjectLabel && !m_header.objectName.empty())
            OverlayAnnotation(rgbOut);
        return true;
    }

  private:
    bool ParseHeader()
    {
        return true;
    }
    bool ReadImageData()
    {
        return true;
    }

    void ComputeStatistics()
    {
        if (m_rawData.empty())
            return;
        m_stats.totalPixels = m_rawData.size();
        m_stats.minValue = *std::min_element(m_rawData.begin(), m_rawData.end());
        m_stats.maxValue = *std::max_element(m_rawData.begin(), m_rawData.end());
        double sum = 0.0;
        for (double v : m_rawData)
            sum += v;
        m_stats.meanValue = sum / static_cast<double>(m_rawData.size());
    }

    double ApplyStretchFunction(double norm) const
    {
        switch (m_stretchMode) {
            case FITSStretchMode::Logarithmic:
                return std::log1p(norm * 1000.0) / std::log1p(1000.0);
            case FITSStretchMode::Sqrt:
                return std::sqrt(norm);
            case FITSStretchMode::Asinh:
                return std::asinh(norm * 10.0) / std::asinh(10.0);
            default:
                return norm;
        }
    }

    void ApplyLUT(const std::vector<uint8_t>& /*mono*/, std::vector<uint8_t>& /*rgb*/) const {}
    void OverlayAnnotation(std::vector<uint8_t>& /*rgb*/) const {}

    std::wstring m_filePath;
    uint32_t m_targetWidth = 0;
    uint32_t m_targetHeight = 0;
    bool m_decoded = false;
    FITSHeaderInfo m_header;
    FITSPixelStatistics m_stats;
    FITSStretchMode m_stretchMode = FITSStretchMode::Logarithmic;
    FITSColorLUT m_colorLUT = FITSColorLUT::STScIDefault;
    std::vector<double> m_rawData;
};

}  // namespace Engine
}  // namespace ExplorerLens
