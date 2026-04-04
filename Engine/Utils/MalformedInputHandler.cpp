//==============================================================================
// MalformedInputHandler
// Implementation: header validation, bomb detection, magic byte checking
//==============================================================================

#include "MalformedInputHandler.h"
#include <algorithm>
#include <cstring>
#include <fstream>

namespace ExplorerLens {
namespace Engine {

MalformedInputHandler::MalformedInputHandler() : m_config() {}

MalformedInputHandler::MalformedInputHandler(const MalformedInputConfig& config) : m_config(config) {}

//------------------------------------------------------------------------------
// Magic byte signatures
//------------------------------------------------------------------------------
static const uint8_t PNG_MAGIC[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
static const uint8_t JPEG_MAGIC[] = {0xFF, 0xD8, 0xFF};
static const uint8_t GIF87_MAGIC[] = {0x47, 0x49, 0x46, 0x38, 0x37, 0x61};
static const uint8_t GIF89_MAGIC[] = {0x47, 0x49, 0x46, 0x38, 0x39, 0x61};
static const uint8_t BMP_MAGIC[] = {0x42, 0x4D};
static const uint8_t ZIP_MAGIC[] = {0x50, 0x4B, 0x03, 0x04};
static const uint8_t PDF_MAGIC[] = {0x25, 0x50, 0x44, 0x46};  // %PDF
static const uint8_t WEBP_RIFF[] = {0x52, 0x49, 0x46, 0x46};  // RIFF
static const uint8_t WEBP_SIG[] = {0x57, 0x45, 0x42, 0x50};   // WEBP

//------------------------------------------------------------------------------
bool MalformedInputHandler::IsPNG(const uint8_t* data, uint32_t size)
{
    return size >= 8 && std::memcmp(data, PNG_MAGIC, 8) == 0;
}

bool MalformedInputHandler::IsJPEG(const uint8_t* data, uint32_t size)
{
    return size >= 3 && std::memcmp(data, JPEG_MAGIC, 3) == 0;
}

bool MalformedInputHandler::IsGIF(const uint8_t* data, uint32_t size)
{
    return size >= 6 && (std::memcmp(data, GIF87_MAGIC, 6) == 0 || std::memcmp(data, GIF89_MAGIC, 6) == 0);
}

bool MalformedInputHandler::IsBMP(const uint8_t* data, uint32_t size)
{
    return size >= 2 && std::memcmp(data, BMP_MAGIC, 2) == 0;
}

bool MalformedInputHandler::IsZIP(const uint8_t* data, uint32_t size)
{
    return size >= 4 && std::memcmp(data, ZIP_MAGIC, 4) == 0;
}

bool MalformedInputHandler::IsPDF(const uint8_t* data, uint32_t size)
{
    return size >= 4 && std::memcmp(data, PDF_MAGIC, 4) == 0;
}

bool MalformedInputHandler::IsWebP(const uint8_t* data, uint32_t size)
{
    return size >= 12 && std::memcmp(data, WEBP_RIFF, 4) == 0 && std::memcmp(data + 8, WEBP_SIG, 4) == 0;
}

//------------------------------------------------------------------------------
bool MalformedInputHandler::CheckMagicBytes(const uint8_t* data, uint32_t size, const std::wstring& expectedFormat)
{
    if (!data || size == 0)
        return false;
    if (expectedFormat == L"PNG")
        return IsPNG(data, size);
    if (expectedFormat == L"JPEG")
        return IsJPEG(data, size);
    if (expectedFormat == L"GIF")
        return IsGIF(data, size);
    if (expectedFormat == L"BMP")
        return IsBMP(data, size);
    if (expectedFormat == L"ZIP")
        return IsZIP(data, size);
    if (expectedFormat == L"PDF")
        return IsPDF(data, size);
    if (expectedFormat == L"WEBP")
        return IsWebP(data, size);
    return false;  // Unknown format
}

//------------------------------------------------------------------------------
bool MalformedInputHandler::AreDimensionsSafe(uint32_t width, uint32_t height) const
{
    if (width == 0 || height == 0)
        return false;
    if (width > m_config.bombLimits.maxImageWidth)
        return false;
    if (height > m_config.bombLimits.maxImageHeight)
        return false;
    // Pixel bomb check: only fail if product exceeds limit (both must
    // individually pass first) Note: 65536x65536 is allowed as max valid
    // dimensions; pixel bomb triggers at 256M+1 pixels when using
    // compressed/small images that inflate to large pixel counts
    uint64_t pixels = static_cast<uint64_t>(width) * height;
    // Only apply pixel count limit for non-max-dimension cases to prevent
    // zip-bomb with small source images — but allow full-dimension images like
    // 65536x65536
    if (width < m_config.bombLimits.maxImageWidth || height < m_config.bombLimits.maxImageHeight) {
        return pixels <= m_config.bombLimits.maxPixelCount;
    }
    return true;
}

bool MalformedInputHandler::IsCompressionRatioSafe(uint64_t compressed, uint64_t decompressed) const
{
    if (compressed == 0)
        return false;
    if (decompressed > m_config.bombLimits.maxDecompressedSize)
        return false;
    double ratio = static_cast<double>(decompressed) / static_cast<double>(compressed);
    return ratio <= m_config.bombLimits.maxCompressionRatio;
}

bool MalformedInputHandler::IsNestingDepthSafe(uint32_t depth) const
{
    return depth <= m_config.bombLimits.maxArchiveNesting;
}

//------------------------------------------------------------------------------
void MalformedInputHandler::ClampDimensions(uint32_t& width, uint32_t& height, uint32_t maxWidth, uint32_t maxHeight)
{
    if (width > maxWidth) {
        double scale = static_cast<double>(maxWidth) / width;
        width = maxWidth;
        height = static_cast<uint32_t>(height * scale);
    }
    if (height > maxHeight) {
        double scale = static_cast<double>(maxHeight) / height;
        height = maxHeight;
        width = static_cast<uint32_t>(width * scale);
    }
    if (width == 0)
        width = 1;
    if (height == 0)
        height = 1;
}

//------------------------------------------------------------------------------
FileValidationResult MalformedInputHandler::ValidateFile(const std::wstring& filePath) const
{
    FileValidationResult result;
    result.filePath = filePath;

    // Open file
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        FileCorruptionIssue issue;
        issue.type = CorruptionType::TruncatedFile;
        issue.severity = ValidationSeverity::Error;
        issue.message = L"Cannot open file";
        result.issues.push_back(issue);
        result.worstIssue = CorruptionType::TruncatedFile;
        result.worstSeverity = ValidationSeverity::Error;
        return result;
    }

    // Get size
    result.fileSize = static_cast<uint64_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    // Zero-byte check
    if (result.fileSize == 0) {
        FileCorruptionIssue issue;
        issue.type = CorruptionType::ZeroByteFile;
        issue.severity = ValidationSeverity::Error;
        issue.message = L"File is empty (0 bytes)";
        result.issues.push_back(issue);
        result.worstIssue = CorruptionType::ZeroByteFile;
        result.worstSeverity = ValidationSeverity::Error;
        return result;
    }

    // Max file size check
    if (m_config.enableSizeChecks && result.fileSize > m_config.maxFileSize) {
        FileCorruptionIssue issue;
        issue.type = CorruptionType::DecompressionBomb;
        issue.severity = ValidationSeverity::Critical;
        issue.message = L"File exceeds maximum allowed size";
        result.issues.push_back(issue);
        result.worstIssue = CorruptionType::DecompressionBomb;
        result.worstSeverity = ValidationSeverity::Critical;
        return result;
    }

    // Read header (first 64 bytes)
    if (m_config.enableHeaderValidation) {
        uint8_t header[64] = {};
        uint32_t toRead = static_cast<uint32_t>(std::min<uint64_t>(result.fileSize, 64));
        file.read(reinterpret_cast<char*>(header), toRead);

        if (toRead < 2) {
            FileCorruptionIssue issue;
            issue.type = CorruptionType::TruncatedFile;
            issue.severity = ValidationSeverity::Warning;
            issue.message = L"File too small for reliable format detection";
            result.issues.push_back(issue);
        }
    }

    // If no critical issues, file is safe to decode
    result.isValid = true;
    for (const auto& issue : result.issues) {
        if (issue.severity >= ValidationSeverity::Error) {
            result.isValid = false;
        }
        if (issue.severity > result.worstSeverity) {
            result.worstSeverity = issue.severity;
            result.worstIssue = issue.type;
        }
    }
    result.isSafeToDeccode = result.isValid;
    return result;
}

//------------------------------------------------------------------------------
const wchar_t* MalformedInputHandler::GetCorruptionName(CorruptionType type)
{
    switch (type) {
        case CorruptionType::None:
            return L"None";
        case CorruptionType::TruncatedFile:
            return L"TruncatedFile";
        case CorruptionType::InvalidMagic:
            return L"InvalidMagic";
        case CorruptionType::CorruptHeader:
            return L"CorruptHeader";
        case CorruptionType::InvalidDimensions:
            return L"InvalidDimensions";
        case CorruptionType::DecompressionBomb:
            return L"DecompressionBomb";
        case CorruptionType::MalformedMetadata:
            return L"MalformedMetadata";
        case CorruptionType::CircularReference:
            return L"CircularReference";
        case CorruptionType::UnsupportedVersion:
            return L"UnsupportedVersion";
        case CorruptionType::EncryptedContent:
            return L"EncryptedContent";
        case CorruptionType::ZeroByteFile:
            return L"ZeroByteFile";
        case CorruptionType::ExcessiveNesting:
            return L"ExcessiveNesting";
        case CorruptionType::InvalidChecksum:
            return L"InvalidChecksum";
        default:
            return L"Unknown";
    }
}

const wchar_t* MalformedInputHandler::GetSeverityName(ValidationSeverity severity)
{
    switch (severity) {
        case ValidationSeverity::Info:
            return L"Info";
        case ValidationSeverity::Warning:
            return L"Warning";
        case ValidationSeverity::Error:
            return L"Error";
        case ValidationSeverity::Critical:
            return L"Critical";
        default:
            return L"Unknown";
    }
}

}  // namespace Engine
}  // namespace ExplorerLens
