#pragma once
//==============================================================================
// MalformedInputHandler
// Robust handling of corrupt, truncated, and adversarial file inputs
//
// Architecture:
// 1. Header validation before full decode
// 2. Size/dimension sanity checks (anti-decompression bomb)
// 3. Structured error reporting with recovery suggestions
// 4. Per-format validation rules
// 5. Timeout/memory limit enforcement
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

/// Input corruption type
enum class CorruptionType : uint8_t {
 None,
 TruncatedFile, ///< File ends prematurely
 InvalidMagic, ///< Magic bytes don't match format
 CorruptHeader, ///< Header fields invalid
 InvalidDimensions, ///< Width/height out of range
 DecompressionBomb, ///< Compressed ratio exceeds limit
 MalformedMetadata, ///< EXIF/XMP/ICC corruption
 CircularReference, ///< Self-referencing structure
 UnsupportedVersion, ///< Valid format but unsupported version
 EncryptedContent, ///< Password-protected content
 ZeroByteFile, ///< Empty file
 ExcessiveNesting, ///< ZIP-in-ZIP-in-ZIP etc
 InvalidChecksum ///< CRC/hash mismatch
};

/// Validation severity
enum class ValidationSeverity : uint8_t {
 Info, ///< Non-blocking observation
 Warning, ///< Decode may produce artifacts
 Error, ///< Decode will fail
 Critical ///< Could crash/hang without protection
};

/// Validation result for a single check
struct ValidationIssue {
 CorruptionType type = CorruptionType::None;
 ValidationSeverity severity = ValidationSeverity::Info;
 std::wstring message;
 std::wstring field; ///< Which header field failed
 uint64_t fileOffset = 0; ///< Byte offset of issue
};

/// File validation result
struct FileValidationResult {
 bool isValid = false;
 bool isSafeToDeccode = false;
 std::wstring filePath;
 uint64_t fileSize = 0;
 std::vector<ValidationIssue> issues;
 CorruptionType worstIssue = CorruptionType::None;
 ValidationSeverity worstSeverity = ValidationSeverity::Info;
};

/// Decompression bomb limits
struct BombLimits {
 uint64_t maxDecompressedSize = 256ULL * 1024 * 1024; ///< 256MB max
 double maxCompressionRatio = 100.0; ///< 100:1 max ratio
 uint32_t maxImageWidth = 65536; ///< 64K pixels
 uint32_t maxImageHeight = 65536;
 uint64_t maxPixelCount = 256ULL * 1024 * 1024; ///< 256M pixels
 uint32_t maxArchiveNesting = 3; ///< Max nested archives
 uint32_t maxArchiveEntries = 100000; ///< Max files in archive
 uint32_t maxMetadataSize = 10 * 1024 * 1024; ///< 10MB metadata
};

/// Malformed input handling configuration
struct MalformedInputConfig {
 BombLimits bombLimits;
 bool enableHeaderValidation = true;
 bool enableSizeChecks = true;
 bool enableTimeouts = true;
 uint32_t decodeTimeoutMs = 30000; ///< 30s timeout
 uint64_t maxFileSize = 4ULL * 1024 * 1024 * 1024; ///< 4GB
 bool logViolations = true;
 bool returnPlaceholderOnError = true; ///< Return error icon instead of null
};

//==============================================================================
// MalformedInputHandler
//==============================================================================
class MalformedInputHandler {
public:
 MalformedInputHandler();
 explicit MalformedInputHandler(const MalformedInputConfig& config);

 /// Validate file before decode attempt
 FileValidationResult ValidateFile(const std::wstring& filePath) const;

 /// Check if dimensions are safe
 bool AreDimensionsSafe(uint32_t width, uint32_t height) const;

 /// Check compression ratio
 bool IsCompressionRatioSafe(uint64_t compressed, uint64_t decompressed) const;

 /// Check nesting depth
 bool IsNestingDepthSafe(uint32_t depth) const;

 /// Quick header magic check for known formats
 static bool CheckMagicBytes(const uint8_t* data, uint32_t size,
 const std::wstring& expectedFormat);

 /// Get safe decode dimensions (clamped)
 static void ClampDimensions(uint32_t& width, uint32_t& height,
 uint32_t maxWidth, uint32_t maxHeight);

 /// Get config
 const MalformedInputConfig& GetConfig() const { return m_config; }

 /// Static name helpers
 static const wchar_t* GetCorruptionName(CorruptionType type);
 static const wchar_t* GetSeverityName(ValidationSeverity severity);

 /// Known magic bytes
 static bool IsPNG(const uint8_t* data, uint32_t size);
 static bool IsJPEG(const uint8_t* data, uint32_t size);
 static bool IsGIF(const uint8_t* data, uint32_t size);
 static bool IsBMP(const uint8_t* data, uint32_t size);
 static bool IsZIP(const uint8_t* data, uint32_t size);
 static bool IsPDF(const uint8_t* data, uint32_t size);
 static bool IsWebP(const uint8_t* data, uint32_t size);

private:
 MalformedInputConfig m_config;
};

}} // namespace ExplorerLens::Engine

