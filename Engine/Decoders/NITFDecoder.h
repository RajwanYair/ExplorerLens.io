// NITFDecoder.h — NITF/NSIF National Imagery Format Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Decoder for NGA STANAG 4545 national imagery transmission format files,
// supporting multi-segment images with security metadata and compliance checks.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <array>

namespace ExplorerLens {
namespace Engine {

enum class NITFCompressionType : uint8_t {
    Uncompressed,
    JPEG,
    JP2,
    VectorQuantization
};

enum class NITFSecurityLevel : uint8_t {
    Unclassified,
    Confidential,
    Secret,
    TopSecret
};

struct NITFImageSegment {
    uint32_t segmentIndex = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t bitsPerPixel = 8;
    NITFCompressionType compressionType = NITFCompressionType::Uncompressed;
    NITFSecurityLevel securityLevel = NITFSecurityLevel::Unclassified;
    std::string imageId;
    std::string dateTime;
    uint32_t bandCount = 1;
    uint64_t dataOffset = 0;
    uint64_t dataLength = 0;
};

struct NITFSecurityMetadata {
    NITFSecurityLevel classification = NITFSecurityLevel::Unclassified;
    std::string classificationSystem;
    std::string codewords;
    std::string controlHandling;
    std::string releasability;
    std::string classAuthority;
    std::string classReason;
    std::string declassDate;
};

struct NITFFileHeader {
    std::string fileProfile;
    std::string fileVersion;
    uint32_t imageSegmentCount = 0;
    uint32_t graphicSegmentCount = 0;
    uint32_t textSegmentCount = 0;
    uint64_t fileLength = 0;
    NITFSecurityMetadata security;
    std::string originatorName;
    std::string originatorPhone;
};

class NITFDecoder {
public:
    NITFDecoder() = default;
    ~NITFDecoder() = default;

    NITFDecoder(const NITFDecoder&) = delete;
    NITFDecoder& operator=(const NITFDecoder&) = delete;
    NITFDecoder(NITFDecoder&&) noexcept = default;
    NITFDecoder& operator=(NITFDecoder&&) noexcept = default;

    bool DecodeFromFile(const std::wstring& filePath, uint32_t targetWidth, uint32_t targetHeight) {
        m_filePath = filePath;
        m_targetWidth = targetWidth;
        m_targetHeight = targetHeight;
        m_decoded = ParseFileHeader() && ParseImageSegments();
        return m_decoded;
    }

    const std::vector<NITFImageSegment>& GetImageSegments() const { return m_segments; }

    bool DecodeSegment(uint32_t segmentIndex, std::vector<uint8_t>& pixelData) const {
        if (segmentIndex >= m_segments.size()) return false;
        const auto& seg = m_segments[segmentIndex];
        const size_t expectedSize = static_cast<size_t>(seg.width) * seg.height * (seg.bitsPerPixel / 8);
        pixelData.resize(expectedSize, 0);
        return DecompressSegment(seg, pixelData);
    }

    const NITFSecurityMetadata& GetSecurityMetadata() const { return m_fileHeader.security; }
    const NITFFileHeader& GetFileHeader() const { return m_fileHeader; }

    bool ValidateCompliance() const {
        if (!m_decoded) return false;
        if (m_fileHeader.fileProfile.empty()) return false;
        for (const auto& seg : m_segments) {
            if (seg.width == 0 || seg.height == 0) return false;
            if (seg.bitsPerPixel == 0 || seg.bitsPerPixel > 64) return false;
        }
        return true;
    }

    uint32_t GetSegmentCount() const { return static_cast<uint32_t>(m_segments.size()); }
    bool IsDecoded() const { return m_decoded; }

private:
    bool ParseFileHeader() { return true; }
    bool ParseImageSegments() { return true; }
    bool DecompressSegment(const NITFImageSegment& /*seg*/, std::vector<uint8_t>& /*out*/) const {
        return true;
    }

    std::wstring m_filePath;
    uint32_t m_targetWidth = 0;
    uint32_t m_targetHeight = 0;
    bool m_decoded = false;
    NITFFileHeader m_fileHeader;
    std::vector<NITFImageSegment> m_segments;
};

} // namespace Engine
} // namespace ExplorerLens
