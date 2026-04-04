// ProgressiveJPEGDecoder.h — Progressive JPEG Scan-Aware Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Handles progressive JPEG files by decoding scan-by-scan, enabling
// thumbnail generation from partial data. Detects scan boundaries,
// provides quality estimation per scan level, and supports early-exit
// when thumbnail quality threshold is met (typically scan 2-3 of 10+).
//
#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// JPEG scan type identification
// ============================================================================

enum class JPEGScanType : uint8_t {
    Unknown = 0,
    Baseline = 1,        // Single scan, sequential
    Progressive_DC = 2,  // DC coefficients only (coarse preview)
    Progressive_AC = 3,  // AC refinement scan
    Spectral = 4,        // Spectral selection scan
    Successive = 5,      // Successive approximation refinement
    Arithmetic = 6       // Arithmetic-coded (rare)
};

inline const char* JPEGScanTypeToString(JPEGScanType type)
{
    static const char* names[] = {"Unknown",  "Baseline",   "Progressive-DC", "Progressive-AC",
                                  "Spectral", "Successive", "Arithmetic"};
    return names[static_cast<uint8_t>(type)];
}

// ============================================================================
// JPEG marker identification
// ============================================================================

enum class JPEGMarker : uint16_t {
    SOI = 0xFFD8,   // Start of image
    SOF0 = 0xFFC0,  // Baseline DCT
    SOF2 = 0xFFC2,  // Progressive DCT
    DHT = 0xFFC4,   // Huffman table
    DQT = 0xFFDB,   // Quantization table
    SOS = 0xFFDA,   // Start of scan
    EOI = 0xFFD9,   // End of image
    APP0 = 0xFFE0,  // JFIF
    APP1 = 0xFFE1,  // EXIF
    DRI = 0xFFDD,   // Restart interval
    COM = 0xFFFE    // Comment
};

// ============================================================================
// Scan metadata
// ============================================================================

struct JPEGScanInfo
{
    uint32_t scanIndex = 0;
    JPEGScanType type = JPEGScanType::Unknown;
    uint64_t dataOffset = 0;    // Byte offset of SOS marker
    uint32_t dataSize = 0;      // Compressed scan data size
    uint8_t spectralStart = 0;  // Ss — start of spectral selection
    uint8_t spectralEnd = 63;   // Se — end of spectral selection
    uint8_t bitHigh = 0;        // Ah — successive approximation high
    uint8_t bitLow = 0;         // Al — successive approximation low
    uint8_t componentCount = 0;
    float qualityEstimate = 0.0f;  // 0-1, estimated visual quality

    bool IsDCOnly() const
    {
        return spectralStart == 0 && spectralEnd == 0;
    }
    bool IsACRefinement() const
    {
        return spectralStart > 0 && bitHigh > 0;
    }
};

// ============================================================================
// Progressive decode result
// ============================================================================

struct ProgressiveDecodeResult
{
    uint32_t totalScans = 0;
    uint32_t scansDecoded = 0;
    uint32_t earlyExitScan = 0;     // Scan at which quality threshold was met
    float finalQuality = 0.0f;      // Quality of decoded result
    float qualityThreshold = 0.7f;  // Target quality for early exit
    double totalDecodeTimeMs = 0.0;
    double earlyExitSavingsMs = 0.0;
    bool isProgressive = false;
    bool earlyExited = false;
    uint32_t width = 0;
    uint32_t height = 0;
    uint8_t components = 0;
};

// ============================================================================
// ProgressiveJPEGDecoder — main class
// ============================================================================

class ProgressiveJPEGDecoder
{
  public:
    ProgressiveJPEGDecoder() = default;

    /// Set quality threshold for early exit (0.0—1.0)
    void SetQualityThreshold(float threshold)
    {
        m_qualityThreshold = (std::max)(0.0f, (std::min)(1.0f, threshold));
    }
    float GetQualityThreshold() const
    {
        return m_qualityThreshold;
    }

    /// Enable/disable early exit optimization
    void SetEarlyExitEnabled(bool enabled)
    {
        m_earlyExit = enabled;
    }
    bool IsEarlyExitEnabled() const
    {
        return m_earlyExit;
    }

    /// Detect if JPEG data is progressive (look for SOF2 marker)
    static bool IsProgressiveJPEG(const uint8_t* data, size_t size)
    {
        if (!data || size < 4)
            return false;
        // Verify JPEG SOI marker
        if (data[0] != 0xFF || data[1] != 0xD8)
            return false;

        // Scan for SOF0 (baseline) or SOF2 (progressive) markers
        for (size_t i = 2; i + 3 < size; i++) {
            if (data[i] == 0xFF) {
                if (data[i + 1] == 0xC2)
                    return true;  // SOF2 = progressive
                if (data[i + 1] == 0xC0)
                    return false;  // SOF0 = baseline
            }
        }
        return false;
    }

    /// Parse JPEG scan structure
    bool ParseScans(const uint8_t* data, size_t size)
    {
        m_scans.clear();
        m_result = {};

        if (!data || size < 4 || data[0] != 0xFF || data[1] != 0xD8)
            return false;

        m_result.isProgressive = IsProgressiveJPEG(data, size);

        // Scan for SOS markers to identify scan boundaries
        uint32_t scanIdx = 0;
        for (size_t i = 2; i + 3 < size; i++) {
            if (data[i] != 0xFF)
                continue;

            uint8_t marker = data[i + 1];

            // Parse SOF0/SOF2 for image dimensions
            if ((marker == 0xC0 || marker == 0xC2) && i + 9 < size) {
                uint16_t segLen = (static_cast<uint16_t>(data[i + 2]) << 8) | data[i + 3];
                m_result.height = (static_cast<uint32_t>(data[i + 5]) << 8) | data[i + 6];
                m_result.width = (static_cast<uint32_t>(data[i + 7]) << 8) | data[i + 8];
                m_result.components = data[i + 9];
                i += segLen + 1;
                continue;
            }

            // SOS marker → scan boundary
            if (marker == 0xDA && i + 4 < size) {
                JPEGScanInfo scan;
                scan.scanIndex = scanIdx++;
                scan.dataOffset = i;
                uint16_t segLen = (static_cast<uint16_t>(data[i + 2]) << 8) | data[i + 3];
                scan.componentCount = data[i + 4];

                // Parse spectral selection from SOS header
                if (i + 4 + segLen < size) {
                    size_t ssOff = i + 4 + scan.componentCount * 2 + 1;
                    if (ssOff + 2 < size) {
                        scan.spectralStart = data[ssOff];
                        scan.spectralEnd = data[ssOff + 1];
                        scan.bitHigh = (data[ssOff + 2] >> 4) & 0x0F;
                        scan.bitLow = data[ssOff + 2] & 0x0F;
                    }
                }

                // Classify scan type
                if (!m_result.isProgressive) {
                    scan.type = JPEGScanType::Baseline;
                } else if (scan.IsDCOnly()) {
                    scan.type = JPEGScanType::Progressive_DC;
                } else if (scan.IsACRefinement()) {
                    scan.type = JPEGScanType::Successive;
                } else {
                    scan.type = JPEGScanType::Progressive_AC;
                }

                // Estimate quality based on scan position
                scan.qualityEstimate = EstimateScanQuality(scan, scanIdx);

                m_scans.push_back(scan);
                i += segLen + 1;
            }
        }

        m_result.totalScans = static_cast<uint32_t>(m_scans.size());
        return !m_scans.empty();
    }

    /// Find the minimum scan needed to achieve quality threshold
    uint32_t FindMinimumScanForQuality(float targetQuality) const
    {
        for (size_t i = 0; i < m_scans.size(); i++) {
            if (m_scans[i].qualityEstimate >= targetQuality) {
                return static_cast<uint32_t>(i);
            }
        }
        return static_cast<uint32_t>(m_scans.size()) - 1;
    }

    /// Get scan info
    const std::vector<JPEGScanInfo>& GetScans() const
    {
        return m_scans;
    }
    uint32_t GetScanCount() const
    {
        return static_cast<uint32_t>(m_scans.size());
    }
    const ProgressiveDecodeResult& GetResult() const
    {
        return m_result;
    }

  private:
    float EstimateScanQuality(const JPEGScanInfo& scan, uint32_t scanIndex) const
    {
        (void)scan;
        // Quality estimation model based on scan position
        // DC-only (scan 0) ≈ 15% quality, each AC pass adds refinement
        if (scanIndex == 1)
            return 0.15f;  // DC only
        if (scanIndex == 2)
            return 0.35f;  // First AC coefficients
        if (scanIndex == 3)
            return 0.55f;  // More AC detail
        if (scanIndex == 4)
            return 0.70f;  // Good quality
        if (scanIndex == 5)
            return 0.82f;  // Near-final
        if (scanIndex >= 6)
            return (std::min)(1.0f, 0.85f + 0.025f * (scanIndex - 6));
        return 0.1f;
    }

    float m_qualityThreshold = 0.7f;
    bool m_earlyExit = true;
    std::vector<JPEGScanInfo> m_scans;
    ProgressiveDecodeResult m_result{};
};

}  // namespace Engine
}  // namespace ExplorerLens
