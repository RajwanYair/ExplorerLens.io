#pragma once
//==============================================================================
// EncoderExportEngine
// Thumbnail export to multiple output formats (PNG, JPEG, WebP, BMP, TIFF, etc.)
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

/// Supported export formats
enum class ExportFormat : uint8_t {
    PNG = 0,
    JPEG,
    WebP,
    BMP,
    TIFF,
    ICO,
    GIF,
    JXL,
    AVIF,
    Count
};

/// Quality preset
enum class QualityPreset : uint8_t {
    Draft    = 0,   // Fast, low quality
    Normal   = 1,   // Balanced
    High     = 2,   // High quality
    Lossless = 3    // Lossless where supported
};

/// Color space for export
enum class ExportColorSpace : uint8_t {
    sRGB = 0,
    AdobeRGB,
    DisplayP3,
    LinearRGB
};

/// Export configuration
struct ExportConfig {
    ExportFormat    format = ExportFormat::PNG;
    QualityPreset   quality = QualityPreset::Normal;
    ExportColorSpace colorSpace = ExportColorSpace::sRGB;
    uint32_t        width = 256;
    uint32_t        height = 256;
    uint8_t         jpegQuality = 85;    // 1-100
    uint8_t         webpQuality = 80;    // 1-100
    bool            preserveAlpha = true;
    bool            embedICCProfile = false;
    bool            progressive = false;  // Progressive JPEG
    uint8_t         pngCompression = 6;  // 0-9
};

/// Export result
struct ExportResult {
    bool        success = false;
    std::wstring outputPath;
    uint64_t    fileSizeBytes = 0;
    double      encodingTimeMs = 0.0;
    ExportFormat format = ExportFormat::PNG;
    std::wstring error;
};

//------------------------------------------------------------------------------
class EncoderExportEngine {
public:
    EncoderExportEngine();
    ~EncoderExportEngine() = default;

    // Export operations
    ExportResult ExportThumbnail(const uint8_t* rgbaData, uint32_t width,
                                  uint32_t height, const ExportConfig& config,
                                  const std::wstring& outputPath);
    ExportResult ExportToMemory(const uint8_t* rgbaData, uint32_t width,
                                 uint32_t height, const ExportConfig& config,
                                 std::vector<uint8_t>& outputBuffer);

    // Batch export
    std::vector<ExportResult> BatchExport(const uint8_t* rgbaData, uint32_t width,
                                           uint32_t height,
                                           const std::vector<ExportConfig>& configs,
                                           const std::wstring& outputDir);

    // Format info
    static const wchar_t* GetFormatName(ExportFormat format);
    static const wchar_t* GetFormatExtension(ExportFormat format);
    static const wchar_t* GetFormatMimeType(ExportFormat format);
    static bool SupportsAlpha(ExportFormat format);
    static bool SupportsLossless(ExportFormat format);
    static uint32_t GetFormatCount();

    // Quality helpers
    static uint8_t GetDefaultQuality(ExportFormat format, QualityPreset preset);
    static const wchar_t* GetPresetName(QualityPreset preset);
    static const wchar_t* GetColorSpaceName(ExportColorSpace cs);

private:
    bool WritePNG(const uint8_t* data, uint32_t w, uint32_t h,
                  const ExportConfig& config, std::vector<uint8_t>& output);
    bool WriteBMP(const uint8_t* data, uint32_t w, uint32_t h,
                  const ExportConfig& config, std::vector<uint8_t>& output);
};

}} // namespace ExplorerLens::Engine

